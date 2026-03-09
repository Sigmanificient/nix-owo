{
  stdenv,
  nix,
  pkg-config,
}: stdenv.mkDerivation {
  name = "nix-hash";

  src = ./src;

  enableParallelBuilding = true;

  nativeBuildInputs = [ pkg-config ];

  buildInputs = [
    nix.libs.nix-util
  ];

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    cp nix-hash $out/bin

    runHook postInstall
  '';
}
