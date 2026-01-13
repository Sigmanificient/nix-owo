#include "nix/util/file-system.hh"
#include "nix/util/file-path.hh"
#include "nix/util/file-path-impl.hh"
#include "nix/util/serialise.hh"
#include "nix/util/util.hh"

#include <cerrno>
#include <filesystem>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <boost/iostreams/device/mapped_file.hpp>

namespace nix {

DirectoryIterator::DirectoryIterator(const std::filesystem::path & p)
{
    try {
        // **Attempt to create the underlying directory_iterator**
        it_ = std::filesystem::directory_iterator(p);
    } catch (const std::filesystem::filesystem_error & e) {
        // **Catch filesystem_error and throw SysError**
        // Adapt the error message as needed for SysError
        throw SysError("cannot read directory %s", p);
    }
}

DirectoryIterator & DirectoryIterator::operator++()
{
    // **Attempt to increment the underlying iterator**
    std::error_code ec;
    it_.increment(ec);
    if (ec) {
        // Try to get path info if possible, might fail if iterator is bad
        try {
            if (it_ != std::filesystem::directory_iterator{}) {
                throw SysError("cannot read directory past %s: %s", it_->path(), ec.message());
            }
        } catch (...) {
            throw SysError("cannot read directory");
        }
    }
    return *this;
}

bool isAbsolute(PathView path)
{
    return std::filesystem::path{path}.is_absolute();
}

Path absPath(PathView path, std::optional<PathView> dir, bool resolveSymlinks)
{
    std::string scratch;

    if (!isAbsolute(path)) {
        // In this case we need to call `canonPath` on a newly-created
        // string. We set `scratch` to that string first, and then set
        // `path` to `scratch`. This ensures the newly-created string
        // lives long enough for the call to `canonPath`, and allows us
        // to just accept a `std::string_view`.
        if (!dir) {
#ifdef __GNU__
            /* GNU (aka. GNU/Hurd) doesn't have any limitation on path
               lengths and doesn't define `PATH_MAX'.  */
            char * buf = getcwd(NULL, 0);
            if (buf == NULL)
#else
            char buf[PATH_MAX];
            if (!getcwd(buf, sizeof(buf)))
#endif
                throw SysError("cannot get cwd");
            scratch = concatStrings(buf, "/", path);
#ifdef __GNU__
            free(buf);
#endif
        } else
            scratch = concatStrings(*dir, "/", path);
        path = scratch;
    }
    return canonPath(path, resolveSymlinks);
}

std::filesystem::path absPath(const std::filesystem::path & path, bool resolveSymlinks)
{
    return absPath(path.string(), std::nullopt, resolveSymlinks);
}

Path canonPath(PathView path, bool resolveSymlinks)
{
    assert(path != "");

    if (!isAbsolute(path))
        throw Error("not an absolute path: '%1%'", path);

    // For Windows
    auto rootName = std::filesystem::path{path}.root_name();

    /* This just exists because we cannot set the target of `remaining`
       (the callback parameter) directly to a newly-constructed string,
       since it is `std::string_view`. */
    std::string temp;

    /* Count the number of times we follow a symlink and stop at some
       arbitrary (but high) limit to prevent infinite loops. */
    unsigned int followCount = 0, maxFollow = 1024;

    auto ret = canonPathInner<OsPathTrait<char>>(
        path, [&followCount, &temp, maxFollow, resolveSymlinks](std::string & result, std::string_view & remaining) {
            if (resolveSymlinks && std::filesystem::is_symlink(result)) {
                if (++followCount >= maxFollow)
                    throw Error("infinite symlink recursion in path '%1%'", remaining);
                remaining = (temp = concatStrings(readLink(result), remaining));
                if (isAbsolute(remaining)) {
                    /* restart for symlinks pointing to absolute path */
                    result.clear();
                } else {
                    result = dirOf(result);
                    if (result == "/") {
                        /* we don’t want trailing slashes here, which `dirOf`
                           only produces if `result = /` */
                        result.clear();
                    }
                }
            }
        });

    if (!rootName.empty())
        ret = rootName.string() + std::move(ret);
    return ret;
}

Path dirOf(const PathView path)
{
    Path::size_type pos = OsPathTrait<char>::rfindPathSep(path);
    if (pos == path.npos)
        return ".";
    return std::filesystem::path{path}.parent_path().string();
}

std::string_view baseNameOf(std::string_view path)
{
    if (path.empty())
        return "";

    auto last = path.size() - 1;
    while (last > 0 && OsPathTrait<char>::isPathSep(path[last]))
        last -= 1;

    auto pos = OsPathTrait<char>::rfindPathSep(path, last);
    if (pos == path.npos)
        pos = 0;
    else
        pos += 1;

    return path.substr(pos, last - pos + 1);
}

bool isInDir(const std::filesystem::path & path, const std::filesystem::path & dir)
{
    /* Note that while the standard doesn't guarantee this, the
      `lexically_*` functions should do no IO and not throw. */
    auto rel = path.lexically_relative(dir);
    /* Method from
       https://stackoverflow.com/questions/62503197/check-if-path-contains-another-in-c++ */
    return !rel.empty() && rel.native()[0] != OS_STR('.');
}

bool isDirOrInDir(const std::filesystem::path & path, const std::filesystem::path & dir)
{
    return path == dir || isInDir(path, dir);
}

struct stat stat(const Path & path)
{
    struct stat st;
    if (stat(path.c_str(), &st))
        throw SysError("getting status of '%1%'", path);
    return st;
}

#ifdef _WIN32
#  define STAT stat
#else
#  define STAT lstat
#endif

struct stat lstat(const Path & path)
{
    struct stat st;
    if (STAT(path.c_str(), &st))
        throw SysError("getting status of '%1%'", path);
    return st;
}

std::optional<struct stat> maybeLstat(const Path & path)
{
    std::optional<struct stat> st{std::in_place};
    if (STAT(path.c_str(), &*st)) {
        if (errno == ENOENT || errno == ENOTDIR)
            st.reset();
        else
            throw SysError("getting status of '%s'", path);
    }
    return st;
}

bool pathExists(const std::filesystem::path & path)
{
    return maybeLstat(path.string()).has_value();
}

bool pathAccessible(const std::filesystem::path & path)
{
    try {
        return pathExists(path.string());
    } catch (SysError & e) {
        // swallow EPERM
        if (e.errNo == EPERM)
            return false;
        throw;
    }
}

Path readLink(const Path & path)
{
    return std::filesystem::read_symlink(path).string();
}

std::string readFile(const Path & path)
{
    AutoCloseFD fd = toDescriptor(open(
        path.c_str(),
        O_RDONLY
// TODO
#ifndef _WIN32
            | O_CLOEXEC
#endif
        ));
    if (!fd)
        throw SysError("opening file '%1%'", path);
    return readFile(fd.get());
}

std::string readFile(const std::filesystem::path & path)
{
    return readFile(os_string_to_string(PathViewNG{path}));
}

void readFile(const Path & path, Sink & sink, bool memory_map)
{
    // Memory-map the file for faster processing where possible.
    if (memory_map) {
        try {
            boost::iostreams::mapped_file_source mmap(path);
            if (mmap.is_open()) {
                sink({mmap.data(), mmap.size()});
                return;
            }
        } catch (const boost::exception & e) {
        }
    }

    // Stream the file instead if memory-mapping fails or is disabled.
    AutoCloseFD fd = toDescriptor(open(
        path.c_str(),
        O_RDONLY
// TODO
#ifndef _WIN32
            | O_CLOEXEC
#endif
        ));
    if (!fd)
        throw SysError("opening file '%s'", path);
    drainFD(fd.get(), sink);
}

#ifdef __FreeBSD__
#  define MOUNTEDPATHS_PARAM , std::set<Path> & mountedPaths
#  define MOUNTEDPATHS_ARG , mountedPaths
#else
#  define MOUNTEDPATHS_PARAM
#  define MOUNTEDPATHS_ARG
#endif

std::filesystem::path makeParentCanonical(const std::filesystem::path & rawPath)
{
    std::filesystem::path path(absPath(rawPath));
    ;
    try {
        auto parent = path.parent_path();
        if (parent == path) {
            // `path` is a root directory => trivially canonical
            return parent;
        }
        return std::filesystem::canonical(parent) / path.filename();
    } catch (std::filesystem::filesystem_error & e) {
        throw SysError("canonicalising parent path of '%1%'", path);
    }
}

} // namespace nix
