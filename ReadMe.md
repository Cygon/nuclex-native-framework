Nuclex Native Framework
=======================

The Nuclex Native Framework is a set of C++ libraries carefully designed
to provide sane, reliable and efficient building blocks that you can drop
into your own projects.

I call it a framework because its individual parts can form a neat and tidy
layer between your code and an uncertain environment. Nevertheless, this is
not the kind of framework you build your application into, it's a set of
components that you can use individually and which make no assumptions
about your architecture.

What makes this one different from other libraries?

  * It's clean, really clean. I haven't cut corners anywhere and all parts
    are designed to make the best possible compromise between performance,
    memory use und simplicity.

  * All public headers have the absolute minimal dependencies. What
    dependencies exist are only to ISO/ANSI C++ headers. Zero global namespace
    pollution and no worries about exploding build times.

  * Full unit test coverage. Everything you can do has a test case verifying
    correct behavior. All components are modular pieces that can be used and
    tested in isolation.

  * All the code is documented. No "filler comments" either, but proper
    explanations of each method's purpose with multiple paragraphs of
    usage instructions and example code where appropriate.
  
  * A single error handling philosophy, executed from beginning to end. This
    library uses exceptions with specific exception classes for specific types
    of errors. All returns from OS and library routines are checked, always.


Libraries
=========

The framework is split into several independent libraries that each contain
classes covering one particular topic.

Another ReadMe.md explaining the details can be found in each library's
subfolder. Here's a short overview of the available functionality:

Nuclex.Support.Native
---------------------

Small, General-purpose supporting code that is useful for almost any project.
This is also the only library that other Nuclex libraries depend on.

  ⚫ Fast delegates   ⚫ Signal/Slot system  ⚫ Modern ScopeGuard
  ⚫ IoC container + dependency injector  ⚫ UTF-8, UTF-16, UTF-32 conversion
  ⚫ UTF-8 case insensitive comparison  ⚫ UTF-8 wildcard matching
  ⚫ lexical casts (number <-> string conversion)  ⚫ Observable lists and maps
  ⚫ Lock-free multi-threaded queues  ⚫ Portable thread pool for micro tasks
  ⚫ Child process wrapper with stdin/stdout/stderr intercept  ⚫ Error logger


Nuclex.Pixels.Native
--------------------

Covers most of the graphics plumbing work. The Bitmap class is straightforward,
yet can run with borrowed memory (i.e. buffers from a 3D API). From there,
you can use other classes to convert pixel formats, resize images and save/load
most common image file formats.

  ⚫ Easy, flexible Bitmap class  ⚫ Bitmaps can be views into other Bitmaps
  ⚫ Copy-on-write  ⚫ All pixel formats supported  ⚫ Pixel format conversion
  ⚫ Conversion between RGB, HSV and HSL  ⚫ Extensible serializer
  ⚫ Toggleable support for  ⚫ .tga  ⚫.tif  ⚫ .png  ⚫ .jpg  ⚫ .exr
  ⚫ High-quality bitmap resizing  ⚫ All file access via user-defined methods


Nuclex.Storage.Native
---------------------

Deals with everything storage-related. It features a complete file system
abstraction layer with decent error handling, extensible compression and
decompression with popular as well as exotic algorithms, XML serialization
and compressed archive reading.

  ⚫ Portable mapping of common paths  ⚫ Install directory  ⚫ Data directory
  ⚫ Settings directory  ⚫ Documents directory  ⚫ Saved game directory
  ⚫ Streaming compression in popular and exotic formats  ⚫ Deflate
  ⚫ LZMA (LZip)  ⚫ LZMA (7-Zip)  ⚫ Brotli  ⚫ UnRAR  ⚫ CSC  ⚫ ZPaq


Others
------

I'll provide controlled releases of my other libraries over time. Currently
in my Subversion repository are `Nuclex.Game.Native`  (game-specific stuff
such as reliable cross-platform precision timing), `Nuclex.Geometry.Native`
(sweet 2D and 3D geometry classes without any rough edges) and a few others.
