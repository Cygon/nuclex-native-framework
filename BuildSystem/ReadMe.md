Build System
============

This are the build systems I use for all my projects. You actually have
the choice between two build systems: [SCons](https://scons.org) and
[CMake](https://cmake.org).

My preferred choice would be [SCons](https://scons.org), which uses Python
to describe which input files have to be processed by which tools to generate
which outputs. So as long as you are accurate down to the last header file,
an optimal build (only what's needed, but nothing ever forgotten) is guaranteed.

The other option, [CMake](https://cmake.org) is a hodgepodge of flawed
solutions to various C++ build tasks, but it has become somewhat of a standard
for C and C++ projects. In order to allow easy integration into existing
projects, CMake is fully supported as well.


CMake Builds
------------

CMake builds are designed to be included via `add_subproject()`, but you can also
build the Nuclex libraries independently and use them as pre-built binaries via
CMake's `find_package()`.




SCons Builds
------------

SCons works like this: rather than issue commands that get executed immediately,
your build script tells SCons which inputs generate which outputs when processed
by what tool.

After your build script has run, SCons has a graph and figures out the quickest
way to generate the output file you want - in parallel with as many threads as
you tell it to use. This may include compiling 5 files while downloading another
2 and at the same time exporting a 3D model.

My build system offers some "convention over configuration" convenience functions
on top of SCons.

If you call my build_library() method like this:

```python
env.build_library()
```

It will assume that your project as a directory `Source`, that you want intermediate
files written a directory named `obj/platform-compiler-release` and that the final
build output is to be copied into `bin/platform-compiler-release` (all overridable).


Example
-------

Here's a full build script for one of my libraries:

```python
# Standard C/C++ build environment with Nuclex extension methods
common_environment = nuclex.create_cplusplus_environment()

# Compile the main library
library_environment = common_environment.Clone()
library_artifacts = library_environment.build_library('Nuclex.Support.Native')

# Compile the unit test executable
unit_test_environment = common_environment.Clone()
unit_test_environment.add_preprocessor_constant('NUCLEX_SUPPORT_EXECUTABLE')
unit_test_artifacts = unit_test_environment.build_unit_tests(
    'Nuclex.Support.Native.Tests'
)
```

Check the `SConstruct` files in each library directory to see how my SCons scripts
are loaded.
