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

        inputsFrom = [
          self.packages.${pkgs.stdenv.hostPlatform.system}.nix-hash
        ];

        packages = with pkgs; [
          clang-tools
          compiledb
        ];
      };
    });

    packages = forAllSystems (pkgs: {
      nix-hash = pkgs.callPackage ./default.nix { };
    });
  };
}
