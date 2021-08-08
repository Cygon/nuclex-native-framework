#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2021 Nuclex Development Labs

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

#if defined(NUCLEX_PIXELS_WINDOWS)
#include "../Platform/WindowsApi.h" /// for HANDLE, etc.
#elif !defined(NUCLEX_PIXELS_LINUX) // No Windows, no Linux -> use Posix!
#include <cstdio> // for FILE, ::fopen(), etc.
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

    /// <summary>Reads data from the file at an absolute position</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    public: void ReadAt(
      std::uint64_t, std::size_t byteCount, std::uint8_t *buffer
    ) const override;

    /// <summary>Writes data into the file at an absolute position</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    public: void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
    ) override;

#if defined(NUCLEX_PIXELS_LINUX)
    /// <summary>File descriptor returned by ::open()</summary>
    private: int fileDescriptor;
#elif defined(NUCLEX_PIXELS_WINDOWS)
    /// <summary>File handle returned by ::CreateFile() or ::OpenFile()</summary>
    private: HANDLE fileHandle;
#else // Go with old-school Posix and hope for the best
    /// <summary>File pointer returned by ::fopen()</summary>
    private: FILE *file;
#endif
    /// <summary>Length of the file in bytes</summary>
    private: std::uint64_t length;
    /// <summary>Current position within the file</summary>
    private: mutable std::uint64_t position;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_REALFILE_H
