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

#ifndef NUCLEX_SUPPORT_PLATFORM_LINUXFILEAPI_H
#define NUCLEX_SUPPORT_PLATFORM_LINUXFILEAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include <string> // std::string
#include <cstdint> // std::uint8_t and std::size_t

#include <sys/stat.h> // ::fstat() and permission flags
#include <dirent.h> // struct ::dirent

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Linux file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is just a small helper class that reduces the amount of boilerplate code
  ///     required when calling the file system API functions, such as checking
  ///     result codes over and over again.
  ///   </para>
  ///   <para>
  ///     It is not intended to hide operating system details or make this API platform
  ///     neutral (the File and Container classes do that), so depending on the amount
  ///     of noise required by the file system APIs, only some methods will be wrapped here.
  ///   </para>
  /// </remarks>
  class LinuxFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>The descriptor (numeric handle) of the opened file</returns>
    public: static int OpenFileForReading(const std::string &path);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>The descriptor (numeric handle) of the opened file</returns>
    public: static int OpenFileForWriting(const std::string &path);

    /// <summary>Changes the position of the file cursor</summary>
    /// <param name="fileDescriptor">File handle whose file cursor will be moved</param>
    /// <param name="offset">Relative position, in bytes, to move the file cursor to</param>
    /// <param name="anchor">Position to which the offset is relative</param>
    /// <returns>The absolute position in bytes from the beginning of the file</returns>
    public: static std::size_t Seek(int fileDescriptor, ::off_t offset, int anchor);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="fileDescriptor">Handle of the file from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(
      int fileDescriptor, std::uint8_t *buffer, std::size_t count
    );

    /// <summary>Writes data into the specified file</summary>
    /// <param name="fileDescriptor">Handle of the file into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(
      int fileDescriptor, const std::uint8_t *buffer, std::size_t count
    );

    /// <summary>Truncates or pads the file to the specified length</summary>
    /// <param name="fileDescriptor">Handle of the file whose length will be set</param>
    /// <param name="byteCount">New length fo the file in bytes</param>
    public: static void SetLength(int fileDescriptor, std::size_t byteCount);

    /// <summary>Flushes all buffered output to the hard drive<summary>
    /// <param name="fileDescriptor">
    ///   File descriptor whose buffered output will be flushed
    /// </param>
    public: static void Flush(int fileDescriptor);

    /// <summary>Closes the specified file</summary>
    /// <param name="fileDescriptor">Handle of the file that will be closed</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the file cannot be closed
    /// </param>
    public: static void Close(int fileDescriptor, bool throwOnError = true);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // NUCLEX_SUPPORT_PLATFORM_LINUXFILEAPI_H
