#pragma once
/**
 * @file
 *
 * Utilities for working with the file system and file paths.
 */

#include "nix/util/types.hh"
#include "nix/util/file-descriptor.hh"
#include "nix/util/file-path.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#ifdef _WIN32
#  include <windef.h>
#endif

#include <functional>
#include <optional>

/**
 * Polyfill for MinGW
 *
 * Windows does in fact support symlinks, but the C runtime interfaces predate this.
 *
 * @todo get rid of this, and stop using `stat` when we want `lstat` too.
 */
#ifndef S_ISLNK
#  define S_ISLNK(m) false
#endif

namespace nix {

struct Sink;
struct Source;

/**
 * Return whether the path denotes an absolute path.
 */
bool isAbsolute(PathView path);

/**
 * @return An absolutized path, resolving paths relative to the
 * specified directory, or the current directory otherwise.  The path
 * is also canonicalised.
 *
 * In the process of being deprecated for `std::filesystem::absolute`.
 */
Path absPath(PathView path, std::optional<PathView> dir = {}, bool resolveSymlinks = false);

inline Path absPath(const Path & path, std::optional<PathView> dir = {}, bool resolveSymlinks = false)
{
    return absPath(PathView{path}, dir, resolveSymlinks);
}

std::filesystem::path absPath(const std::filesystem::path & path, bool resolveSymlinks = false);

/**
 * Canonicalise a path by removing all `.` or `..` components and
 * double or trailing slashes.  Optionally resolves all symlink
 * components such that each component of the resulting path is *not*
 * a symbolic link.
 *
 * In the process of being deprecated for
 * `std::filesystem::path::lexically_normal` (for the `resolveSymlinks =
 * false` case), and `std::filesystem::weakly_canonical` (for the
 * `resolveSymlinks = true` case).
 */
Path canonPath(PathView path, bool resolveSymlinks = false);

/**
 * @return The directory part of the given canonical path, i.e.,
 * everything before the final `/`.  If the path is the root or an
 * immediate child thereof (e.g., `/foo`), this means `/`
 * is returned.
 *
 * In the process of being deprecated for
 * `std::filesystem::path::parent_path`.
 */
Path dirOf(const PathView path);

/**
 * @return the base name of the given canonical path, i.e., everything
 * following the final `/` (trailing slashes are removed).
 *
 * In the process of being deprecated for
 * `std::filesystem::path::filename`.
 */
std::string_view baseNameOf(std::string_view path);

/**
 * Check whether 'path' is a descendant of 'dir'. Both paths must be
 * canonicalized.
 */
bool isInDir(const std::filesystem::path & path, const std::filesystem::path & dir);

/**
 * Check whether 'path' is equal to 'dir' or a descendant of
 * 'dir'. Both paths must be canonicalized.
 */
bool isDirOrInDir(const std::filesystem::path & path, const std::filesystem::path & dir);

/**
 * Get status of `path`.
 */
struct stat stat(const Path & path);
struct stat lstat(const Path & path);
/**
 * `lstat` the given path if it exists.
 * @return std::nullopt if the path doesn't exist, or an optional containing the result of `lstat` otherwise
 */
std::optional<struct stat> maybeLstat(const Path & path);

/**
 * @return true iff the given path exists.
 */
bool pathExists(const std::filesystem::path & path);

/**
 * Canonicalize a path except for the last component.
 *
 * This is useful for getting the canonical location of a symlink.
 *
 * Consider the case where `foo/l` is a symlink. `canonical("foo/l")` will
 * resolve the symlink `l` to its target.
 * `makeParentCanonical("foo/l")` will not resolve the symlink `l` to its target,
 * but does ensure that the returned parent part of the path, `foo` is resolved
 * to `canonical("foo")`, and can therefore be retrieved without traversing any
 * symlinks.
 *
 * If a relative path is passed, it will be made absolute, so that the parent
 * can always be canonicalized.
 */
std::filesystem::path makeParentCanonical(const std::filesystem::path & path);

/**
 * A version of pathExists that returns false on a permission error.
 * Useful for inferring default paths across directories that might not
 * be readable.
 * @return true iff the given path can be accessed and exists
 */
bool pathAccessible(const std::filesystem::path & path);

/**
 * Read the contents (target) of a symbolic link.  The result is not
 * in any way canonicalised.
 *
 * In the process of being deprecated for
 * `std::filesystem::read_symlink`.
 */
Path readLink(const Path & path);

/**
 * Open a `Descriptor` with read-only access to the given directory.
 */
Descriptor openDirectory(const std::filesystem::path & path);

/**
 * Read the contents of a file into a string.
 */
std::string readFile(const Path & path);
std::string readFile(const std::filesystem::path & path);
void readFile(const Path & path, Sink & sink, bool memory_map = true);

enum struct FsSync { Yes, No };

struct DIRDeleter
{
    void operator()(DIR * dir) const
    {
        closedir(dir);
    }
};

typedef std::unique_ptr<DIR, DIRDeleter> AutoCloseDir;

/**
 * Return `TMPDIR`, or the default temporary directory if unset or empty.
 */
Path defaultTempDir();

/**
 * Used in various places.
 */
typedef std::function<bool(const Path & path)> PathFilter;

extern PathFilter defaultPathFilter;

/**
 * @brief A directory iterator that can be used to iterate over the
 * contents of a directory. It is similar to std::filesystem::directory_iterator
 * but throws NixError on failure instead of std::filesystem::filesystem_error.
 */
class DirectoryIterator
{
public:
    // --- Iterator Traits ---
    using iterator_category = std::input_iterator_tag;
    using value_type = std::filesystem::directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::filesystem::directory_entry *;
    using reference = const std::filesystem::directory_entry &;

    // Default constructor (represents end iterator)
    DirectoryIterator() noexcept = default;

    // Constructor taking a path
    explicit DirectoryIterator(const std::filesystem::path & p);

    reference operator*() const
    {
        // Accessing the value itself doesn't typically throw filesystem_error
        // after successful construction/increment, but underlying operations might.
        // If directory_entry methods called via -> could throw, add try-catch there.
        return *it_;
    }

    pointer operator->() const
    {
        return &(*it_);
    }

    DirectoryIterator & operator++();

    // Postfix increment operator
    DirectoryIterator operator++(int)
    {
        DirectoryIterator temp = *this;
        ++(*this); // Uses the prefix increment's try-catch logic
        return temp;
    }

    // Equality comparison
    friend bool operator==(const DirectoryIterator & a, const DirectoryIterator & b) noexcept
    {
        return a.it_ == b.it_;
    }

    // Inequality comparison
    friend bool operator!=(const DirectoryIterator & a, const DirectoryIterator & b) noexcept
    {
        return !(a == b);
    }

    // Allow direct use in range-based for loops if iterating over an instance
    DirectoryIterator begin() const
    {
        return *this;
    }

    DirectoryIterator end() const
    {
        return DirectoryIterator{};
    }


private:
    std::filesystem::directory_iterator it_;
};

#ifdef __FreeBSD__
class AutoUnmount
{
    Path path;
    bool del;
public:
    AutoUnmount(Path &);
    AutoUnmount();
    ~AutoUnmount();
    void cancel();
};
#endif

} // namespace nix
