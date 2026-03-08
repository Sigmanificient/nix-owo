{
  lib,
  stdenv,
  nix,
  pkg-config,
}: stdenv.mkDerivation {
  name = "nix-hash";

  src = lib.sourceFilesBySuffices ./. [ "Makefile" ".cpp" ".hpp" ];

  enableParallelBuilding = true;

  nativeBuildInputs = [ pkg-config ];

  buildInputs = [
    nix.libs.nix-util
  ];

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    cp nix-hash nix-owo $out/bin

    runHook postInstall
  '';
}
