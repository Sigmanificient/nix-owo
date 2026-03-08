#pragma once

#include <nix/util/base-n.hh>
#include <nix/util/hash.hh>
#include <nix/util/posix-source-accessor.hh>

std::string makeHeader(uint64_t num);

void SAdumpContents(nix::SourceAccessor& sa, const nix::CanonPath & path, nix::Sink & sink, uint64_t num = 0);

void SAdump(nix::SourceAccessor& sa, const nix::CanonPath & path, nix::Sink & sink, uint64_t num = 0);

void SAdumpPath(nix::SourceAccessor& sa, const nix::CanonPath &path, nix::Sink &sink, uint64_t num = 0);

std::string hash_to_string(nix::Hash hash);
