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
      ] (system: function nixpkgs.legacyPackages.${system});
  in {
    formatter = forAllSystems (pkgs: pkgs.alejandra);

    devShells = forAllSystems (pkgs: {
      default = pkgs.mkShell {
        hardeningDisable = ["fortify"];

        env = {
          FILTERPATH_SOURCE_TEST = pkgs.filterpath.src;
          # sha256-FOewYznmWOWH2TyNySVoa+spvH4QlXnjlko+/zFiNik=

          CRITERION_SOURCE_TEST = pkgs.criterion.src;
          # sha256-X4m/uCyanS7HLtf6GyK4XuaT5i+HQt1PZC7gd813IVQ=

          QTILE_SOURCE_TEST = pkgs.python3Packages.qtile.src;
          # sha256-PPyI+IGvHBQusVmU3D26VjYjLaa9+94KUqNwbQSzeaI=
        };

        inputsFrom = [
          self.packages.${pkgs.stdenv.hostPlatform.system}.nix-hash
        ];

        packages = with pkgs; [
          clang-tools
          compiledb
        ];
      };
    });

    packages = forAllSystems (pkgs: rec {
      boost-custom = pkgs.callPackage ./boost.nix { };

      nix-hash = pkgs.callPackage ./default.nix { inherit boost-custom; };
    });
  };
}
