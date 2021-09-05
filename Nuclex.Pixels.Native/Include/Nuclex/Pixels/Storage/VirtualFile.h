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

#ifndef NUCLEX_PIXELS_STORAGE_VIRTUALFILE_H
#define NUCLEX_PIXELS_STORAGE_VIRTUALFILE_H

#include "Nuclex/Pixels/Config.h"

#include <string> // for std::string
#include <memory> // for std::unique_ptr

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Allows reading and writing data to an on-disk or streamed file</summary>
  /// <remarks>
  ///   <para>
  ///     If you want to read data from a source other than a file, this class is your
  ///     means to achieve that. All codecs fully and correctly implement their underlying
  ///     library's custom IO callbacks, so no temporary files are created and all IO
  ///     is translated into the 3 methods exposed in this interface.
  ///   </para>
  ///   <para>
  ///     Custom implementations of a VirtualFile are allowed to throw exceptions from
  ///     all their methods. Such exceptions will resurface from the
  ///     <see cref="BitmapCodec.TryRead" />, <see cref="BitmapCodec.Load" /> or
  ///     <see cref="BitmapCodec.Save" /> method (or their respective wrappers in the
  ///     <see cref="BitmapSerializer" /> class) and there will be no memory leaks.
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE VirtualFile {

    /// <summary>Opens a real file stored in the OS' file system for reading</summary>
    /// <param name="path">
    ///   Path of the file that will be opened for reading as UTF-8 string
    /// </param>
    /// <param name="promiseSequentialAccess">
    ///   Whether you promise to read from the file sequentially only
    /// </param>
    /// <returns>The file at the specified path, opened in read-only mode</returns>
    /// <remarks>
    ///   <para>
    ///     This opens a file using the most direct/efficient method for accessing files
    ///     provided by the current OS.
    ///   </para>
    ///   <para>
    ///     The returned file is *not* thread-safe. This means if
    ///     <see cref="VirtualFile.ReadAt" /> and <see cref="VirtualFile.WriteAt" />
    ///     calls happen from different threads, mixed-up data, spurious exceptions and
    ///     all kinds of wrong behavior will ensue. If you want to access the same file
    ///     from multiple threads, each should call this method to get its own instance.
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API static std::unique_ptr<const VirtualFile> OpenRealFileForReading(
      const std::string &path, bool promiseSequentialAccess = false
    );

    /// <summary>Opens a real file stored in the OS' file system for writing</summary>
    /// <param name="path">
    ///   Path of the file that will be opened for writing as UTF-8 string
    /// </param>
    /// <param name="promiseSequentialAccess">
    ///   Whether you promise to write to the file sequentially only
    /// </param>
    /// <returns>The file at the specified path, opened in write-only mode</returns>
    /// <remarks>
    ///   <para>
    ///     If the file already exists, it will be truncated to 0 bytes. This creates
    ///     a file using the most direct/efficient method for accessing files provided by
    ///     the current OS.
    ///   </para>
    ///   <para>
    ///     The returned file is *not* thread-safe. This means if
    ///     <see cref="VirtualFile.ReadAt" /> and <see cref="VirtualFile.WriteAt" />
    ///     calls happen from different threads, mixed-up data, spurious exceptions and
    ///     all kinds of wrong behavior will ensue. If you want to access the same file
    ///     from multiple threads, each should call this method to get its own instance.
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API static std::unique_ptr<VirtualFile> OpenRealFileForWriting(
      const std::string &path, bool promiseSequentialAccess = false
    );

    /// <summary>Frees all memory used by the instance</summary>
    public: virtual ~VirtualFile() = default;

    /// <summary>Determines the current size of the file in bytes</summary>
    /// <returns>The size of the file in bytes</returns>
    public: virtual std::uint64_t GetSize() const = 0;

    /// <summary>Reads data from the file</summary>
    /// <param name="start">Offset in the file at which to begin reading</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <parma name="buffer">Buffer into which the data will be read</param>
    /// <remarks>
    ///   Some file system APIs let you specify a larger number of bytes to read
    ///   (i.e. your buffer size) and may read less than that if the end of the
    ///   file is reached. This implementation will throw an exception if you
    ///   attempt to read beyond the end of the file.
    /// </remarks>
    public: virtual void ReadAt(
      std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
    ) const = 0;

    /// <summary>Writes data into the file</summary>
    /// <param name="start">Offset at which writing will begin in the file</param>
    /// <param name="byteCount">Number of bytes that should be written</param>
    /// <param name="buffer">Buffer holding the data that should be written</param>
    /// <remarks>
    ///   As a special feature of the WriteAt() method, the start index can be
    ///   equal to the current file length (but not more). This will append data
    ///   at the end of the file, thus increasing the file's size.
    /// </remarks>
    public: virtual void WriteAt(
      std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
    ) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_VIRTUALFILE_H
