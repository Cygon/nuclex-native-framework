Nuclex.Storage.Native
=====================

This library has everything you need to access data from the file system,
including from archive files (like `.zip`, `.rar` or `.7z`), directly
streaming compressed data, serializing arbitraty data structures to XML files
and reading them back, patching, hashing and more.

As always, there are unit tests for the whole library, so everything is
verifiably working on all platforms tested (Linux, Windows, Raspberry).

* Compression algorithm manager to select appropriate algorithms
* Common and exotic algorithms wrapped behind a clean streaming interface
  * Deflate (via ZLib, known from `.zip`)
  * LZMA (via LZip)
  * LZMA (via LZMA-SDK, known from `.7z`)
  * Brotli (Google's high-end algorithm)
  * UnRar (known from `.rar`)
  * BSC (high-compressing BZip2-like)
  * CSC (high-compressing LZMA-like)
  * ZPaq (ultra-high compressing bytecode-based system)
  * Tangelo (optimized ZPaq, faster but similar ratios)
* Reading and writing binary data with endian-awareness
* File system abstraction with airtight error handling
* Find each platform's intended locations for storing different kinds of data
  * i.e. executables, game assets, saved games, configuration files

* (WIP) Hashing with MD5, SHA-1 SHA-256
* (WIP) Binary diffs and patching

* Supports Windows, Linux and ARM Linux (Raspberry PI)
* Compiles cleanly at maximum warning levels with MSVC, GCC and clang
* Everything is unit-tested

