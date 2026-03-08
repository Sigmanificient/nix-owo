{
  boost-custom,
  stdenv,
  nix,
  pkg-config,
  meson,
  ninja,
}: stdenv.mkDerivation {
  name = "nix-hash";

  src = ./src;

  enableParallelBuilding = true;

  nativeBuildInputs = [ meson ninja pkg-config ];

  buildInputs = [
    boost-custom
    nix.libs.nix-util
  ];

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    cp nix-hash $out/bin

    runHook postInstall
  '';
}
