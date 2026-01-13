
#include "nix/util/file-path.hh"

namespace nix {

std::optional<std::filesystem::path> maybePath(PathView path)
{
    return {path};
}

std::filesystem::path pathNG(PathView path)
{
    return path;
}

} // namespace nix
