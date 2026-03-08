#include <cstdint>
#include <iostream>

#include "utils.hpp"

using namespace nix;

int main(int argc, char **argv)
{
    Path path = (argc > 1) ? argv[1] : ".";
    uint64_t start = (argc > 2) ? std::stoull(argv[2]) : 1;
    uint64_t end = (argc > 3) ? std::stoull(argv[3]) : UINT64_MAX;
    std::filesystem::path abspath = absPath(path);
    PosixSourceAccessor root = abspath.root_path();

    for (uint64_t i = start; i < end; ++i) {
        HashSink sink(HashAlgorithm::SHA256);
        SAdumpPath(root, CanonPath{abspath.relative_path().string()}, sink, i);
        HashResult result = sink.finish();
        auto hash = hash_to_string(result.hash);
        if (hash[47] == '0' && hash[48] == 'w' && hash[49] == '0') {
            std::cerr << std::endl;
            std::cout << hash << std::endl;;
            std::cout << makeHeader(i);
            return 0;
        }
        if (i % 100 == 0)
            std::cerr << ".";
    }

    std::cerr << std::endl << "Unable to find match" << std::endl;
    return 1;
}
