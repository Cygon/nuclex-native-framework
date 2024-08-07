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

#ifndef NUCLEX_PIXELS_PLATFORM_WINDOWSFILEAPI_H
#define NUCLEX_PIXELS_PLATFORM_WINDOWSFILEAPI_H

#include "Nuclex/Pixels/Config.h"

#if defined(NUCLEX_PIXELS_WINDOWS)

#include "WindowsApi.h"

namespace Nuclex { namespace Pixels { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps file access functions from the Windows file system API</summary>
  /// <remarks>
  ///   <para>
  ///     This is just a small helper class that reduces the amount of boilerplate code
  ///     required when calling the file system API functions from Windows, such as
  ///     checking result codes and transforming paths from UTF-8 to UTF-16 stored in
  ///     wchar_ts of non-standard 2 byte size.
  ///   </para>
  ///   <para>
  ///     It is not intended to hide operating system details or make this API platform
  ///     neutral (the File and Container classes do that), so depending on the amount
  ///     of noise required by the file system APIs, only some methods will be wrapped here.
  ///   </para>
  /// </remarks>
  class WindowsFileApi {

    /// <summary>Opens the specified file for shared reading</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <param name="sequentialAccess">Whether the file will be read sequentially</param>
    /// <returns>The handle of the opened file</returns>
    public: static HANDLE OpenFileForReading(const std::string &path, bool sequentialAccess);

    /// <summary>Creates or opens the specified file for exclusive writing</summary>
    /// <param name="path">Path of the file that will be opened</param>
    /// <param name="sequentialAccess">Whether the file will be written sequentially</param>
    /// <returns>The handle of the opened file</returns>
    public: static HANDLE OpenFileForWriting(const std::string &path, bool sequentialAccess);

    /// <summary>Returns the total size of the file in bytes</summary>
    /// <param name="fileHandle">Handle of the file whose size will be reported</param>
    /// <returns>The size of the file with the specified handle in bytes</returns>
    public: static std::uint64_t GetFileSize(::HANDLE fileHandle);

    /// <summary>Moves the file cursor to a different position</summary>
    /// <param name="fileHandle">Handle of the file whose file cursor to move</param>
    /// <param name="offset">Offset to move the file cursor relative to the anchor</param>
    /// <param name="anchor">Anchor relative to which the file cursor will be placed</param>
    /// <returns>The new absolute position of the file cursor from the file start</returns>
    public: static std::size_t Seek(HANDLE fileHandle, std::ptrdiff_t offset, DWORD anchor);

    /// <summary>Reads data from the specified file</summary>
    /// <param name="fileHandle">Handle of the file from which data will be read</param>
    /// <param name="buffer">Buffer into which the data will be put</param>
    /// <param name="count">Number of bytes that will be read from the file</param>
    /// <returns>The number of bytes that were actually read</returns>
    public: static std::size_t Read(HANDLE fileHandle, void *buffer, std::size_t count);

    /// <summary>Writes data into the specified file</summary>
    /// <param name="fileHandle">Handle of the file into which data will be written</param>
    /// <param name="buffer">Buffer containing the data that will be written</param>
    /// <param name="count">Number of bytes that will be written into the file</param>
    /// <returns>The number of bytes that were actually written</returns>
    public: static std::size_t Write(HANDLE fileHandle, const void *buffer, std::size_t count);

    /// <summary>Closes the specified file</summary>
    /// <param name="fileHandle">Handle of the file that will be closed</param>
    /// <param name="throwOnError">
    ///   Whether to throw an exception if the file cannot be closed
    /// </param>
    public: static void CloseFile(HANDLE fileHandle, bool throwOnError = true);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that should be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorCode">
    ///   Value that GetLastError() returned at the time of failure
    /// </param>
    public: [[noreturn]] static void ThrowExceptionForFileAccessError(
      const std::string &errorMessage, DWORD errorCode
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Platform

#endif // defined(NUCLEX_PIXELS_WINDOWS)

#endif // NUCLEX_PIXELS_PLATFORM_WINDOWSFILEAPI_H
