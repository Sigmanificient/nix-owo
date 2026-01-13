#pragma once
///@file

#include "nix/util/file-system.hh"
#include "nix/util/types.hh"
#include "nix/util/serialise.hh"

namespace nix {

void dumpPath(const Path & path, Sink & sink, PathFilter & filter = defaultPathFilter);

inline constexpr std::string_view narVersionMagic1 = "nix-archive-1";

inline constexpr std::string_view caseHackSuffix = "~nix~case~hack~";

} // namespace nix
