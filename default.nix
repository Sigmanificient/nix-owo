{
  lib,
  stdenv,
  nix,
  pkg-config,
  boost,
}:
stdenv.mkDerivation {
  name = "nix-hash";

  src = lib.sourceFilesBySuffices ./. [
    "Makefile"
    ".cpp"
    ".hpp"
    ".sh"
  ];

  enableParallelBuilding = true;

  nativeBuildInputs = [pkg-config];

  buildInputs = [
    nix.libs.nix-util
    boost
  ];

  env.PREFIX = placeholder "out";

  doCheck = true;

  nativeCheckInputs = [ nix ];

  meta = {
    description = "Customize your nix (SRI) hashes";
    maintainers = with lib.maintainers; [sigmanificient];
    platforms = lib.platforms.unix;
  };
}
