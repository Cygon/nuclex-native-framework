How to Compile
==============

To build the libraries, you can use either

 * [https://scons.org/](SCons) (a Python-based build system)
 * [https://cmake.org/](CMake) (a build system based on poor design)

Projects for Visual Studio 2017 are also provided. You will have to
compile external libraries with SCons or CMake first before you can
use these.

Because CMake has become somewhat of a a standard for C++ projects,
I'll provide instructions for building with CMake here.


Visual Studio 2017
------------------

You can open the `NuclexNativeFramework (msvc-14.1).sln` file.

Make sure to build the libraries with CMake once (my libraries
include a bunch of third-party dependencies that need to be downloaded
and built first - either SCons or CMake take care of that).


Visual Studio 2019
------------------

Currently doesn't provide a convenient `.sln` or `.vcxproj` for you.

You can build with CMake, but to use the IDE projects, you'd have to
edit the library paths to use `windows-msvc14.2-amd64-*` instead of
`windows-msvc14.1-amd64-*` to look for the library binaries.

Also, Visual Studio 2019 has a compiler bug. Compiling with the VS2019
compiler will disable the affected code and print a warning. Due to this,
I recommend not using Visual Studio 2019 just yet.


CMake (Linux)
-------------

You need CMake 3.18 or later for this.

Linux users should simply run `build.sh` found at the root directory
level. It will call the `build.sh` in each library directory,
which in turn builds each library by calling `cmake`.

If you want to build just a single library, you can also run
the `build.sh` of that library directly.


CMake (Windows)
---------------

You need CMake 3.18 or later for this.

Windows users can simply run `build.cmd` found at the root directory
level. It will call the `build.cmd` in each library directory,
which in turn builds each library by calling `cmake`.

If you want to build just a single library, you can also run
the `build.cmd` of that library directly.
