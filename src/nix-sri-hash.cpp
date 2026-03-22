#include <iostream>

#include <nix/util/base-n.hh>
#include <nix/util/hash.hh>
#include <nix/util/posix-source-accessor.hh>

using namespace nix;

int main(int argc, char **argv)
{
    Path path = (argc > 1) ? argv[1] : ".";
    std::filesystem::path abspath = absPath(path);
    PosixSourceAccessor root = abspath.root_path();

    HashSink sink(HashAlgorithm::SHA256);

    try {
        root.dumpPath(CanonPath{abspath.relative_path().string()}, sink);
        HashResult result = sink.finish();

        std::cout << result.hash.to_string(HashFormat::SRI, true) << "\n";

        return EXIT_SUCCESS;
    }
    catch (const Error & e) {
        std::cerr << "error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
