{
  boost-custom,
  stdenv,
  brotli,
  libarchive,
  libblake3,
  libcpuid,
  libsodium,
  nlohmann_json,
  openssl,
  pkg-config,
}: stdenv.mkDerivation {
  name = "nix-hash";

  src = ./.;

  enableParallelBuilding = true;

  nativeBuildInputs = [ pkg-config ];

  buildInputs = [
    boost-custom
    brotli.dev
    libarchive
    libblake3
    libcpuid
    libsodium
    nlohmann_json
    openssl
  ];

  installPhase = ''
    runHook preInstall

    mkdir -p $out/bin
    cp nix-hash $out/bin

    runHook postInstall
  '';
}
