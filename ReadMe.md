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

Build System
------------

I build and test my libraries on multiple platforms, including Linux,
Windows and Raspberry PIs. So I need a build system where I can simply
say "this directory is a project set up with my standard layout. The library
is called X. Go build it!"

It's simple and deals with dependencies, both via source builds and from
precompiled binaries.


Nuclex.Support.Native
---------------------

This library contains general-purpose code that I use in most of my projects.

  * Delegates (like std::function + std::bind, but equality-comparable)
  * Events (super fast and lightweight signal/slot implementation)
  * Dependency injector with automatic constructor signature detection
  * Lexical cast (robust number/string conversion without iostreams)
  * UTF transcoding (turn Windowisms and silly TEXT() macros into UTF-8)
  * Observable collections (be notified when items are added/removed)
  * Lock-free thread-safe collections (fast & thread-safe without mutexes)


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


Others
------

I'll provide controlled releases of my other libraries over time. Currently
in my Subversion repository are `Nuclex.Game.Native`  (game-specific stuff
such as reliable cross-platform precision timing), `Nuclex.Geometry.Native`
(sweet 2D and 3D geometry classes without any rough edges) and a few others.
