#include <iostream>

#include "nix/util/hash.hh"
#include "nix/util/archive.hh"
#include "nix/util/file-content-address.hh"

using namespace nix;

int main(int argc, char **argv)
{
    try {
        Path path = (argc > 1) ? argv[1] : ".";

        HashSink sink(HashAlgorithm::SHA256);

        sink.writeUnbuffered(""); // ensure buffer exists (optional)

        dumpPath(
            path,
            sink
        );

        HashResult result = sink.finish();

        std::cout << result.hash.to_string(HashFormat::SRI, true) << "\n";

        return 0;
    }
    catch (const Error & e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

