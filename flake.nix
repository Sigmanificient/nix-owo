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
          pkgs.nix.libs.nix-util
        ];

        packages = with pkgs; [
          clang-tools
          compiledb
        ];
      };
    });
  };
}
