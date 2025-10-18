{
  description = "C SHELL";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    # TODO: change to git repo
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

        buildInputs = [ pkgs.clang cshread ];

        buildPhase = ''
          make
        '';

        installPhase = ''
          mkdir -p $out/bin
          cp bin/main $out/bin/c-shell
        '';
      };

      # Default package alias
      defaultPackage.${system} = self.packages.${system}.default;

      # DevShell definition
      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = [
          pkgs.clang-tools
          pkgs.clang
          # For generating compile_commands.json
          pkgs.bear
          # debugging
          pkgs.valgrind
          pkgs.lldb
        ];

        buildInputs = [
          pkgs.libcxx
          cshread
          # Unit testing
          pkgs.cmocka
        ];

        # Set CPATH environment variable
        shellHook = ''
          # env-vars
          export CPATH=${pkgs.lib.makeSearchPathOutput "dev" "include" [ pkgs.libcxx ]}:${pkgs.lib.makeSearchPath "resource-root/include" [ pkgs.clang ]}:${cshread}/include:$CPATH
          export LIBRARY_PATH=${cshread}/lib:$LIBRARY_PATH

          # aliases
          alias valgrind-debug="make bd && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./bin/main-debug"
          alias vd="valgrind-debug"
        '';
      };
    };
}
