#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

#ifndef NUCLEX_PIXELS_STORAGE_REALFILE_H
#define NUCLEX_PIXELS_STORAGE_REALFILE_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/Storage/VirtualFile.h"

// We're being a bit silly here: it would be no problem to simply use
// fopen() on all platforms, but on Windows and Linux we can bypass
// the C/C++ runtime library wrappers and go directly for the OS function.
//
// The advantages are: a) more speed, b) we can tell the OS more accurately
// what we're going to do and c) unicode filenames on Windows systems.

#if defined(NUCLEX_PIXELS_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h> // For the HANDLE type...
#undef min
#undef max
#undef CreateFile
#undef ReadFile
#undef WriteFile
// ...and countless other define abominations that would need to be #undef'd
#elif defined(NUCLEX_PIXELS_LINUX)
// Nothing needed...
#else // No Windows, no Linux, let's try Posix
#include <cstdio> // For the FILE type...
#endif

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads and saves data to a normal file in the OS' file system</summary>
  class RealFile : public VirtualFile {

    /// <summary>Initializes a new instance accessing the file at the specified path</summary>
    /// <param name="path">Path of the file that will be accessed or created</param>
    /// <param name="promiseSequentialAccess">
    ///   Whether you promise to write to the file sequentially only
    /// </param>
    /// <param name="readOnly">Whether write accesses to the file will be denied</param>
    public: RealFile(
      const std::string &path, bool promiseSequentialAccess, bool readOnly
    );

    /// <summary>Frees all memory used by the instance</summary>
    public: virtual ~RealFile();

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: std::uint64_t GetSize() const override { return this->length; }

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: void ReadAt(
      std::uint64_t, std::size_t byteCount, std::uint8_t *buffer
    ) const override;

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
    ) override;

#if defined(NUCLEX_PIXELS_WIN32)
    /// <summary>File handle returned by CreateFile() or OpenFile()</summary>
    private: HANDLE fileHandle;
#elif defined(NUCLEX_PIXELS_LINUX)
    /// <summary>File descriptor returned by open()</summary>
    private: int fileDescriptor;
#else // Go with standard Posix and hope for the best
    /// <summary>File pointer returned by fopen()</summary>
    private: FILE *filePointer;
#endif
    /// <summary>Length of the file in bytes</summary>
    private: std::uint64_t length;
    /// <summary>Current position within the file</summary>
    private: mutable std::uint64_t position;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_REALFILE_H
