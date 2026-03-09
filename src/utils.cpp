#include "utils.hpp"

using namespace nix;

std::string makeHeader(uint64_t num)
{
    return "<!-- 0w0: " + std::to_string(num) + " -->\n";
}

// Patched method of Source Accessor

bool SAdumpContents(SourceAccessor& sa, const CanonPath & path, Sink & sink, uint64_t num)
{
    bool ret = false;

    sink << "contents";

    std::string s;

    static bool printedFound = false;
    if (num != 0 && path.baseName() == "README.md") {
        ret = true;
        if (!printedFound) {
            std::cerr << "found README.md at " << path << std::endl;
            std::cerr.flush();
            printedFound = true;
        }
        s = makeHeader(num);
    }

    s += sa.readFile(path);

    // SourceAccessor::readFile(3 params)
    uint64_t size = s.size();

    sink << size;
    sink(s);

    writePadding(size, sink);

    return ret;
}

bool SAdump(SourceAccessor& sa, const CanonPath & path, Sink & sink, uint64_t num)
{
    if (path.baseName() == ".git")
        return false;

    bool ret = false;

    auto st = sa.lstat(path);

    sink << "(";

    if (st.type == SourceAccessor::tRegular) {
        sink << "type" << "regular";
        if (st.isExecutable)
            sink << "executable" << "";
        ret |= SAdumpContents(sa, path, sink, num);
    }

    else if (st.type == SourceAccessor::tDirectory) {
        sink << "type" << "directory";

        /* If we're on a case-insensitive system like macOS, undo
           the case hack applied by restorePath(). */
        StringMap unhacked;
        for (auto & i : sa.readDirectory(path))
                unhacked.emplace(i.first, i.first);

        for (auto & i : unhacked) {
            sink << "entry" << "(" << "name" << i.first << "node";
            // Pass 0 as num if we've already found README.md, so nested READMEs don't get processed
            ret |= SAdump(sa, path / i.second, sink, ret ? 0 : num);
            sink << ")";
        }
    }

    else if (st.type == SourceAccessor::tSymlink)
        sink << "type" << "symlink" << "target" << sa.readLink(path);

    else
        throw Error("file '%s' has an unsupported type", path);

    sink << ")";

    return ret;
}

bool SAdumpPath(SourceAccessor& sa, const CanonPath &path, Sink &sink, uint64_t num)
{
    sink << "nix-archive-1";
    return SAdump(sa, path, sink, num);
}

// End

std::string hash_to_string(Hash hash)
{
    std::string s = "sha256-";

    const auto bytes = std::as_bytes(std::span<const uint8_t>{&hash.hash[0], hash.hashSize});
    s += base64::encode(bytes);
 
    return s;
}
