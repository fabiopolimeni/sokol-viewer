# sokol-viewer

This project aims at creating a simple 3D model viewer with voxel real-time rendering tech.

## How to build

To build the project use the python build tool **fips** https://github.com/floooh/fips

Make sure that the following tools are in the path. Exact versions shouldn't
matter:
```
> python --version
Python 2.7.10
> cmake --version
cmake version 3.8.2
# make is only needed for building through emscripten
> make --version
GNU Make 3.81
# on OSX (on Windows you just need a recent VS)
> xcodebuild -version
Xcode 9.0
```

### Building the platform-specific samples

Use the platform-agnostic library *sokol* application-wrapper
in the folder ```sapp```.

## To build the platform-agnostic sokol_app.h samples:

Building the sokol_app.h application is currently supported for MacOS, Windows, 
Linux, iOS, HTML5 and Android (RaspberryPi is planned).

Use any of the following custom build configs starting with ```sapp-```
which matches your platform and build system:

```bash
> ./fips list configs | grep sapp-
  sapp-android-make-debug
  ...
  sapp-d3d11-win64-vs2017-debug
  sapp-d3d11-win64-vs2017-release
  sapp-d3d11-win64-vscode-debug
  sapp-d3d11-win64-vstudio-debug
  sapp-d3d11-win64-vstudio-release
  sapp-ios-xcode-debug
  ...
  sapp-win64-vstudio-debug
  sapp-win64-vstudio-release
> ./fips set config sapp-...
> ./fips build
> ./fips list targets
> ./fips run cube-sapp
```

Note the following caveats:
- for HTML5, first install the emscripten SDK 
```
./fips setup emscripten
```

- for iOS, set the developer team id, as described above in the iOS section
- OpenGL is currently not supported on MacOS because NSOpenGLView and
friends are broken on the MacOS Mojave beta, instead use the
```sapp-metal-*``` build configs (GLES on iOS is supported though)
