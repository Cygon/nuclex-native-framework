How to Compile
==============

The libraries in this repository use [https://scons.org/](SCons),
a Python-based build system to compile.

For the convenience of Windows users, a Visual Studio solution
is also provided.

**No matter what you use, you'll have to compile the third-party
libraries first!**

(I'll provide a master SCons script that will take care of all
the third-party libraries with one call in the future!)


Linux Users
-----------

1. Install SCons via your package manager (i.e. `emerge scons` or `sudo apt-get install scons`)
2. Go into each directory in the ThirdParty folder and run SCons there like this:

```
scons -j8 DEBUG=1
scons -j8
```

3. Now you can do the same in the Nuclex.*.Native directories or use the Visual Studio code workspace.


Windows Users
-------------

1. Install the Windows distribution of SCons
2. Go into each directory in the ThirdParty folder and run SCons there like this:

```
scons -j8 DEBUG=1
scons -j8
```

3. Now you can do the same in the Nuclex.*.Native directories or use the Visual Studio solution
