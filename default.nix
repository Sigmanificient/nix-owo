{
  boost-custom,
  stdenv,
  nix,
  pkg-config,
  meson,
  ninja,
  test-nix-hash-sh,
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
    cp ${test-nix-hash-sh} $out/bin/test-nix-hash.sh

    runHook postInstall
  '';
}
