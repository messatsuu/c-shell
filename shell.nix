{ pkgs ? import <nixpkgs> {}, ... }:

pkgs.mkShell {
  nativeBuildInputs = [
    # builder tools
    pkgs.cmake

    # XXX: the order of include matters
    pkgs.clang-tools
    pkgs.clang # for clangd support

    pkgs.gtest
  ];

  buildInputs = [
    # stdlib for C++
    pkgs.libcxx
  ];

  # Optional compiler flags
  CXXFLAGS = "-std=c++17";

  # Path setup for includes
  CPATH = builtins.concatStringsSep ":" [
    (pkgs.lib.makeSearchPathOutput "dev" "include" [ pkgs.libcxx ])
    (pkgs.lib.makeSearchPath "resource-root/include" [ pkgs.clang ])
  ];
}

