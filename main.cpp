#include <iostream>

#include <nix/util/base-n.hh>
#include <nix/util/hash.hh>
#include <nix/util/posix-source-accessor.hh>

using namespace nix;

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

        root.dumpPath(CanonPath{abspath.relative_path().string()}, sink);

        HashResult result = sink.finish();
        std::cout << hash_to_string(result.hash) << "\n";
        return 0;
    }
    catch (const Error & e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
