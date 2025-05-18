{
  description = "C SHELL";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
      {
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "c-shell";
        version = "0.1";
        src = ./.;

        buildInputs = [ pkgs.clang ];

        buildPhase = ''
          make build
        '';

        installPhase = ''
          mkdir -p $out/bin
          cp bin/main $out/bin/c-shell
        '';
      };

      defaultPackage.${system} = self.packages.${system}.default;
    };
}
