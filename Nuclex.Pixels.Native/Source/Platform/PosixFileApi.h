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

#ifndef NUCLEX_PIXELS_PLATFORM_POSIXFILEAPI_H
#define NUCLEX_PIXELS_PLATFORM_POSIXFILEAPI_H

#include "Nuclex/Pixels/Config.h"

#if !defined(NUCLEX_PIXELS_WINDOWS)

#include <string> // for std::string
#include <cstdint> // for std::uint8_t
#include <cstdio> // for FILE, ::fopen(), etc.

namespace Nuclex { namespace Pixels { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Posix file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is a helper class that wraps Posix calls with error checking and conversion
  ///     between C strings and C++ strings so that this boilerplate code does not have
  ///     to be repeated over and over in other places.
  ///   </para>
  /// </remarks>
  class PosixFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>A pointer representing the opened file</returns>
    public: static FILE *OpenFileForReading(const std::string &path);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <returns>A pointer representing the opened file</returns>
    public: static FILE *OpenFileForWriting(const std::string &path);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="file">File from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(
      FILE *file, std::uint8_t *buffer, std::size_t count
    );

    /// <summary>Writes data into the specified file</summary>
    /// <param name="file">File into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(
      FILE *file, const std::uint8_t *buffer, std::size_t count
    );

    /// <summary>Closes the specified file</summary>
    /// <param name="file">File pointer returned by the open method</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the file cannot be closed
    /// </param>
    public: static void Close(FILE *file, bool throwOnError = true);

    /// <summary>Changes the position of the file cursor in the specified file</summary>
    /// <param name="file">File whose file cursor will be moved</param>
    /// <param name="offset">
    ///   Offset, relative to the anchor point, at which to place the file cursor
    /// </param>
    /// <param name="anchor">Anchor point relative to which the offset is interpresed</param>
    public: static void Seek(FILE *file, std::ptrdiff_t offset, int anchor);

    /// <summary>Determines the current absolute position of the file cursor</summary>
    /// <param name="file">File whose file cursor position will be reported</param>
    /// <returns>The current position of the file cursor in bytes from the beginning</returns>
    public: static std::uint64_t Tell(FILE *file);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorNumber">Value that 'errno' had at the time of failure</param>
    public: [[noreturn]] static void ThrowExceptionForFileAccessError(
      const std::string &errorMessage, int errorNumber
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Platform

#endif // !defined(NUCLEX_PIXELS_WINDOWS)

#endif // NUCLEX_PIXELS_PLATFORM_POSIXFILEAPI_H
