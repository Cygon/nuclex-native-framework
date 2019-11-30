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
#error This file must be included through RealFile.cpp
#endif

#if !defined(NUCLEX_PIXELS_WIN32)
#include <cerrno> // errno
#include <cstring> // strerror()
#include <sys/stat.h> // stat()
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Returns the error message for the specified Posix error code</summary>
  /// <param name="errorNumber">Error number to provide an error message for</param>
  /// <returns>The error message for the specified Posix error code</returns>
  std::string getPosixError(int errorNumber) {
    return std::string(::strerror(errorNumber));
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Throws a FileAccessError exception for a Posix error code</summary>
  /// <param name="errorNumber">Error number for which a file access error is thrown</param>
  void throwPosixFileAccessError(int errorNumber) {
    throw Nuclex::Pixels::Errors::FileAccessError(
      std::error_code(errorNumber, std::system_category()), getPosixError(errorNumber)
    );
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32) && !defined(NUCLEX_PIXELS_LINUX)

  /// <summary>Determines the size of a file using the Posix API</summary>
  /// <param name="path">Path to the file whose size will be determined</param>
  /// <returns>The size of the specified file</returns>
  std::uint64_t getPosixFileSize(const std::string &path) {
    struct stat fileStatus;

    // The fseek()+ftell() trick is unsafe and terrible for compressed and
    // downloading files. Doing the check by path is much nicer on the system.
    int failed = ::stat(path.c_str(), &fileStatus);
    if(failed) {
      int errorNumber = errno;
      throwPosixFileAccessError(errorNumber);
    }

    return static_cast<std::uint64_t>(fileStatus.st_size);
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32) && !defined(NUCLEX_PIXELS_LINUX)

  /// <summary>Moves the file cursor in a Posix file</summary>
  /// <param name="filePointer">FILE pointer for the file whose cursor will be moved</param>
  /// <param name="position">New position for the file cursor</param>
  void movePosixFileCursor(FILE *filePointer, std::uint64_t position) {
    assert(
      (position < std::numeric_limits<long>::max()) &&
      "Requested position must be within the limitations of the Posix API"
    );

    int failed = ::fseek(filePointer, static_cast<long>(position), SEEK_SET);
    if(failed) {
      int errorNumber = errno;
      throwPosixFileAccessError(errorNumber);
    }
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32) && !defined(NUCLEX_PIXELS_LINUX)

  /// <summary>Reads data from a file using the Posix API</summary>
  /// <param name="filePointer">Pointer for the file from which data will be read</param>
  /// <param name="fileCursor">
  ///   Position within the file (not checked to move the file cursor but is updated)
  /// </param>
  /// <param name="byteCount">Number of bytes that will be read</param>
  /// <param name="buffer">Buffer into which the data will be written</param>
  void readPosixFile(
    FILE *filePointer,
    std::uint64_t &fileCursor, std::size_t byteCount, std::uint8_t *buffer
  ) {
    std::size_t readByteCount = ::fread(buffer, 1, byteCount, filePointer);
    fileCursor += readByteCount;

    if(readByteCount != byteCount) {
      int errorNumber = errno;
      throwPosixFileAccessError(errorNumber);
    }
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32) && !defined(NUCLEX_PIXELS_LINUX)

  /// <summary>Writes data to a file using the Posix API</summary>
  /// <param name="filePointer">Pointer for the file to which data will be written</param>
  /// <param name="fileCursor">
  ///   Position within the file (not checked to move the file cursor but is updated)
  /// </param>
  /// <param name="byteCount">Number of bytes that will be read</param>
  /// <param name="buffer">Buffer into which the data will be written</param>
  void writePosixFile(
    FILE *filePointer,
    std::uint64_t &fileCursor, std::size_t byteCount, const std::uint8_t *buffer
  ) {
    std::size_t writtenByteCount = ::fwrite(buffer, 1, byteCount, filePointer);
    fileCursor += writtenByteCount;

    if(writtenByteCount != byteCount) {
      int errorNumber = errno;
      throwPosixFileAccessError(errorNumber);
    }
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace
