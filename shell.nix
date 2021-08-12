let
  sources = import ./nix/sources.nix;
  niv = import sources.niv { inherit sources; };
  pkgs = import sources.nixpkgs { };
in pkgs.mkShell {
  LOCALE_ARCHIVE = "${pkgs.glibcLocales}/lib/locale/locale-archive";
  buildInputs = with pkgs; [
    # Dev tooling
    clang_12
    clang-tools
    ccache
    cmake
    cmake-format
    cppcheck
    pkgconfig
    ninja
    flawfinder
    include-what-you-use
    python3
    valgrind
    gdb
    (callPackage ./nix/pkgs/camomilla { pythonXXPackages = python39Packages; })
    (callPackage ./nix/pkgs/lizard { })

    # Libraries
    fmt
    SDL2
    SDL2.dev
    SDL2_ttf

    # Nix support
    niv.niv
  ];
}
