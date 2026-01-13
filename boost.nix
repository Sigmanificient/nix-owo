{ lib, boost }:
(boost.override {
  extraB2Args = [
    "--with-container"
    "--with-context"
    "--with-coroutine"
    "--with-iostreams"
    "--with-url"
  ];
  enableIcu = false;
}).overrideAttrs (old: {
  # Need to remove `--with-*` to use `--with-libraries=...`
  buildPhase = lib.replaceStrings [ "--without-python" ] [ "" ] old.buildPhase;
  installPhase = lib.replaceStrings [ "--without-python" ] [ "" ] old.installPhase;
})
