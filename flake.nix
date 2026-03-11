{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = {
    self,
    nixpkgs,
  }: let
    inherit (nixpkgs) lib;

    forAllSystems = function:
      lib.genAttrs [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ] (system: function nixpkgs.legacyPackages.${system} system);

    mkEnv = pkgs: {
      FILTERPATH_SOURCE_TEST = pkgs.filterpath.src;
      # sha256-FOewYznmWOWH2TyNySVoa+spvH4QlXnjlko+/zFiNik=

      CRITERION_SOURCE_TEST = pkgs.criterion.src;
      # sha256-X4m/uCyanS7HLtf6GyK4XuaT5i+HQt1PZC7gd813IVQ=

      QTILE_SOURCE_TEST = pkgs.python3Packages.qtile.src;
      # sha256-PPyI+IGvHBQusVmU3D26VjYjLaa9+94KUqNwbQSzeaI=
    };
  in {
    formatter = forAllSystems (pkgs: system: pkgs.alejandra);

    devShells = forAllSystems (pkgs: system: {
      default = pkgs.mkShell {
        hardeningDisable = ["fortify"];

        env = mkEnv pkgs;

        inputsFrom = [
          self.packages.${system}.nix-owo
        ];

        packages = with pkgs; [
          clang-tools
          compiledb
        ];
      };
    });

    checks = forAllSystems (pkgs: system: {
      hashCorrectnessTest =
        pkgs.runCommand "hashCorrectnessTest" {
          env = mkEnv pkgs;
        } ''
          ${./test-nix-hash.sh} ${lib.getExe' self.packages.${system}.nix-owo "nix-sri-hash"}
          touch $out
        '';
      find0w0SuffixTest = pkgs.runCommand "find0w0SuffixTest" {} ''
        set -o pipefail
        touch README.md
        if ! ${lib.getExe' self.packages.${system}.nix-owo "nix-owo"} | grep "0w0="; then
            echo "hash does not contain 0w0"
            exit 1
        fi
        touch $out
      '';
    });

    packages = forAllSystems (pkgs: system: rec {
      nix-owo = pkgs.callPackage ./default.nix {};

      default = nix-owo;
    });
  };
}
