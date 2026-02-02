{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    gcc
    glfw3
    glm
    pkg-config
    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    libepoxy
    libGL
    xorg.libXext
    xorg.libXxf86vm
    xorg.xeyes
    mesa
  ];

  shellHook = ''
    export PKG_CONFIG_PATH=${pkgs.glfw3}/lib/pkgconfig:${pkgs.glm}/lib/pkgconfig:${pkgs.libepoxy}/lib/pkgconfig:$PKG_CONFIG_PATH
    export LD_LIBRARY_PATH=${pkgs.mesa.drivers}/lib:${pkgs.libGL}/lib:${pkgs.xorg.libX11}/lib:${pkgs.glfw3}/lib:$LD_LIBRARY_PATH
    export LIBGL_DRIVERS_PATH=${pkgs.mesa.drivers}/lib/dri
    export LIBGL_ALWAYS_INDIRECT=0
    echo "SPH 2D Simulator development environment loaded"
    echo "Available packages: cmake, gcc, glfw3, glm"
  '';
}
