#include <iostream>

#include <nix/util/base-n.hh>
#include <nix/util/hash.hh>
#include <nix/util/posix-source-accessor.hh>

using namespace nix;

// Patched method of Source Accessor

static void SAdumpContents(SourceAccessor& sa, const CanonPath & path, Sink & sink)
{
    sink << "contents";

    auto s = sa.readFile(path);

    // SourceAccessor::readFile(3 params)
    uint64_t size = s.size();

    sink << size;
    sink(s);

    writePadding(size, sink);
}

static void SAdump(SourceAccessor& sa, const CanonPath & path, Sink & sink)
{
    auto st = sa.lstat(path);

    sink << "(";

    if (st.type == SourceAccessor::tRegular) {
        sink << "type" << "regular";
        if (st.isExecutable)
            sink << "executable" << "";
        SAdumpContents(sa, path, sink);
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
            SAdump(sa, path / i.second, sink);
            sink << ")";
        }
    }

    else if (st.type == SourceAccessor::tSymlink)
        sink << "type" << "symlink" << "target" << sa.readLink(path);

    else
        throw Error("file '%s' has an unsupported type", path);

    sink << ")";
}

static
void SAdumpPath(SourceAccessor& sa, const CanonPath &path, Sink &sink)
{
    sink << "nix-archive-1";
    SAdump(sa, path, sink);
}

// End

static std::string hash_to_string(Hash hash)
{
    std::string s = "sha256-";

    const auto bytes = std::as_bytes(std::span<const uint8_t>{&hash.hash[0], hash.hashSize});
    s += base64::encode(bytes);
 
    return s;
}


int main(int argc, char **argv)
{
    Path path = (argc > 1) ? argv[1] : ".";
    std::filesystem::path abspath = absPath(path);
    PosixSourceAccessor root = abspath.root_path();

    HashSink sink(HashAlgorithm::SHA256);

    try {
        sink.writeUnbuffered(""); // ensure buffer exists (optional)

        SAdumpPath(root, CanonPath{abspath.relative_path().string()}, sink);

        HashResult result = sink.finish();
        std::cout << hash_to_string(result.hash) << "\n";
        return 0;
    }
    catch (const Error & e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
