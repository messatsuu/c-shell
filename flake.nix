{
  description = "C SHELL";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    cshread-lib.url = "github:messatsuu/c-shell-read";
    cshread-lib.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, cshread-lib }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      cshread = cshread-lib.packages.${system}.default;
    in {
      # Package build for c-shell
      packages.${system}.default = pkgs.stdenv.mkDerivation {
        pname = "c-shell";
        version = "0.1.0";
        src = ./.;

        nativeBuildInputs = [ pkgs.makeWrapper ];
        buildInputs = [ cshread pkgs.clang ];
        makeFlags = [
          "CC=clang"
        ];

        buildPhase = ''
          make
        '';

        installPhase = ''
          mkdir -p $out/bin
          cp bin/main $out/bin/csh
        '';
      };

      apps.${system}.c-shell = {
        type = "app";
        program = "${self.packages.${system}.default}/bin/csh";
      };

      devShells.${system}.default = pkgs.mkShell {
        packages = [
          pkgs.clang-tools
          pkgs.clang-analyzer
          pkgs.clang
          # For generating compile_commands.json
          pkgs.bear
          # debugging
          pkgs.valgrind
          pkgs.lldb
          # Linter
          pkgs.checkmake
          # man pages
          pkgs.glibcInfo
          pkgs.man-pages
          cshread
          # Unit testing
          pkgs.cmocka
          pkgs.bats
        ];

        shellHook = ''
          echo "C shell dev environment loaded. Rebuilding libs."

          alias valgrind-debug="make bd && valgrind --leak-check=full ./bin/main-debug"
          alias vd="valgrind-debug"
        '';
      };
    };
}
