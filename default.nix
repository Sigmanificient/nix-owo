{
  lib,
  stdenv,
  nix,
  pkg-config,
}:
stdenv.mkDerivation {
  name = "nix-hash";

  src = lib.sourceFilesBySuffices ./. [
    "Makefile"
    ".cpp"
    ".hpp"
  ];

  enableParallelBuilding = true;

  nativeBuildInputs = [pkg-config];

  buildInputs = [nix.libs.nix-util];

  env.PREFIX = placeholder "out";

  meta = {
    description = "Customize your nix (SRI) hashes";
    maintainers = with lib.maintainers; [sigmanificient];
    platforms = lib.platforms.unix;
  };
}
