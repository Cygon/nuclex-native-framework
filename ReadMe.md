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

  * It's clean, really clean. My components are designed to make the best
    possible compromise between performance, memory use und simplicity.

  * Full unit test coverage. Everything you can do has a test case verifying
    correct behavior and components are modular pieces that can be used and
    tested in isolation.

  * Low-dependency philosophy. For changing logic, DRY is one of the highest
    priorities. For unchanging logic in a library, going for least bloat
    and minimal dependencies is most important.


Components
==========

The framework is split into several independent libraries that each cover one
particular topic. The only dependency amongst them is Nuclex::Support, a shared
library of supporting classes (like unicode translation, signals/slots and
converting between numbers and strings).


Nuclex.Support.Native
---------------------

This library contains general-purpose code that I use in most of my projects.

  * Delegates (like std::function + std::bind, but equality-comparable)
  * Events (super fast and lightweight signal/slot implementation)
  * Lexical cast (robust number/string conversion without iostreams)
  * UTF-8 wildcard matching, case-insensitive comparison and conversion
  * Observable collections (be notified when items are added/removed)
  * Dependency injector with automatic constructor signature detection
  * Logger interfaces and high performance in-memory ring buffer logger
  * Child process wrapper (run external programs and capture their output)
  * (WIP) Lock-free thread-safe collections (fast & thread-safe without mutexes)
  * (WIP) Command-line parsing and formatting


Nuclex.Pixels.Native
--------------------

This library provides a very light yet convenient Bitmap class that can
support borrowed memory (i.e. constructed using a 3D API's buffer) and is
a pleasure to use.

  * Convenient and efficient Bitmap class
  * Bitmaps use copy-on-write for the memory they carry
  * Views into Bitmaps (Bitmaps that are an area inside another Bitmap)
  * Fast, portable pixel format conversion
  * Extensible Bitmap serializer with automatic file format detection
  * Built-in serialization formats for PNG, JPG and EXR images
  * Conversion between RGB, HSV, HSL and (WIP) YUV pixels
  * High quality bitmap resizing


Nuclex.Storage.Native
---------------------

Deals with everything storage-related. It features a complete file system
abstraction layer with decent error handling, extensible compression and
decompression with popular as well as exotic algorithms, XML serialization
and compressed archive reading.

  * Uses a platform's correct paths to store different kinds of data
  * Streamed compression/decompression of different formats, including
    * Deflate (via ZLib, known from `.zip`)
    * LZMA (via LZip)
    * LZMA (via LZMA-SDK, known from `.7z`)
    * Brotli
    * UnRar (known from `.rar`)
    * CSC (high-compressing LZMA-like)
    * ZPaq (ultra-high compressing bytecode-based system)
  * Read and write XML -formatted data
  * External type serializers, serialize 3rd party types directly and
    don't drag serialization code around in your classes
  * (WIP) Binary diffs
  * (WIP) Stream data out of .zip, .rar, .7z archives


Others
------

I'll provide controlled releases of my other libraries over time. Currently
in my Subversion repository are `Nuclex.Game.Native`  (game-specific stuff
such as reliable cross-platform precision timing), `Nuclex.Geometry.Native`
(sweet 2D and 3D geometry classes without any rough edges) and a few others.
