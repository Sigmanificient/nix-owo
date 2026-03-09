#include <cstdint>
#include <iostream>

#include "utils.hpp"

using namespace nix;

int main(int argc, char **argv)
{
    Path path = (argc > 1) ? argv[1] : ".";
    uint64_t total = (argc > 2) ? std::stoull(argv[2]) : 1;
    uint64_t rank = (argc > 3) ? std::stoull(argv[3]) : 1;

    uint64_t interval = UINT64_MAX / total;
    uint64_t start = 1 + interval * (rank - 1);
    uint64_t end = interval * rank;

    std::filesystem::path abspath = absPath(path);
    PosixSourceAccessor root = abspath.root_path();

    for (uint64_t i = start; i < end; ++i) {
        HashSink sink(HashAlgorithm::SHA256);
        bool found_readme = SAdumpPath(root, CanonPath{abspath.relative_path().string()}, sink, i);

        if (!found_readme) {
            std::cerr << "Could not find README.md" << '\n';
            return 1;
        }

        HashResult result = sink.finish();
        auto hash = result.hash.to_string(nix::HashFormat::SRI, true);

        if (hash[47] == '0' && hash[48] == 'w' && hash[49] == '0') {
            std::cerr << '\n';
            std::cout << hash << '\n';
            std::cout << makeHeader(i);
            return EXIT_SUCCESS;
        }
        if (i % 100 == 0)
            std::cerr << ".";
    }

    std::cerr << "\nUnable to find match\n";
    return EXIT_FAILURE;
}
