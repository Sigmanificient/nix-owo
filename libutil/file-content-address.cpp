#include "nix/util/file-content-address.hh"
#include "nix/util/source-path.hh"

namespace nix {

void dumpPath(const SourcePath & path, Sink & sink, FileSerialisationMethod method, PathFilter & filter)
{
    switch (method) {
    case FileSerialisationMethod::Flat:
        path.readFile(sink);
        break;
    case FileSerialisationMethod::NixArchive:
        path.dumpPath(sink, filter);
        break;
    }
}

} // namespace nix
