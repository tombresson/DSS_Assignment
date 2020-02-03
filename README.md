# DSS_Assignment

## Building

* Install VisualStudio 2019
* Clone [**vcpkg** from GitHub](https://github.com/Microsoft/vcpkg)
* Follow the setup steps in the [**vcpkg** Quick Start](https://github.com/microsoft/vcpkg/blob/master/README.md#quick-start)
* Install libcurl with **vcpkg** `vcpkg install curl:x64-windows`
* Copy all the files in the `redist` to the `x64\Debug` folder
* Build the project
* Run/Debug the project

All the libraries other libraries (**SDL2**) to build/debug should be included in the project folder `./src/lib/SDL2`.

DO **NOT** use `vcpkg` to install `sdl2` or `sdl2-image`. There's some issue with the way they are
built that causes SDL to just crash. The included SDL libraries were download from the SDL website from
[SDL2 (SDL2-devel-2.0.10-VC.zip (Visual C++ 32/64-bit))](https://www.libsdl.org/download-2.0.php) and [SDL2-Image (SDL2_image-devel-2.0.5-VC.zip (Visual C++ 32/64-bit))](https://www.libsdl.org/projects/SDL_image/).

The [**jsmn**](https://github.com/zserge/jsmn) library is included, but it is built into the project.

## Running

* Make sure [Visual C++ Redistributable x64](https://aka.ms/vs/16/release/vc_redist.x64.exe) is installed
* Unzip the `DSS_Assignment.zip` file in the `release` directory to a folder
* Run `DSS_Assignment.exe`

### Control

Navigation is done with `left` and `right` keys. `Q` will quit, as will clicking the close button on the window.


## Notes

JSON endpoint is hard coded into the application (in `display.c`), so recompiling is the only way to change it.
