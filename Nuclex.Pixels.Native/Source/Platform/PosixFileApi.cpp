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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "PosixFileApi.h"

#if !defined(NUCLEX_PIXELS_WINDOWS)

#include "PosixApi.h"
#include "Nuclex/Pixels/Errors/FileAccessError.h" // for FileAccessError

#include <cstdio> // for fopen() and fclose()
#include <cerrno> // To access ::errno directly
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForReading(const std::string &path) {
    static const char *fileMode = "rb";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(unlikely(file == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  FILE *PosixFileApi::OpenFileForWriting(const std::string &path) {
    static const char *fileMode = "w+b";

    FILE *file = ::fopen(path.c_str(), fileMode);
    if(unlikely(file == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      ThrowExceptionForFileAccessError(errorMessage, errorNumber);
    }

    return file;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileApi::Read(FILE *file, std::uint8_t *buffer, std::size_t count) {
    size_t readByteCount = ::fread(buffer, 1, count, file);
    if(unlikely(readByteCount == 0)) {
      int errorNumber = errno;

      int result = ::feof(file);
      if(result != 0) {
        return 0; // Read was successful, but end of file has been reached
      }

      ThrowExceptionForFileAccessError(
        u8"Could not read data from file", errorNumber
      );
    }

    return static_cast<std::size_t>(readByteCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PosixFileApi::Write(FILE *file, const std::uint8_t *buffer, std::size_t count) {
    size_t writtenByteCount = ::fwrite(buffer, 1, count, file);
    if(unlikely(writtenByteCount == 0)) {
      int errorNumber = errno;

      int result = ::ferror(file);
      if(result == 0) {
        return 0; // Write was successful but no bytes could be written ?_?
      }

      ThrowExceptionForFileAccessError(
        u8"Could not write data to file", errorNumber
      );
    }

    return writtenByteCount;
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::Close(FILE *file, bool throwOnError /* = true */) {
    int result = ::fclose(file);
    if(throwOnError && unlikely(result != 0)) {
      int errorNumber = errno;
      ThrowExceptionForFileAccessError(
        u8"Could not close file", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::Seek(FILE *file, std::ptrdiff_t offset, int anchor) {
    int result = ::fseek(file, static_cast<long>(offset), anchor);
    if(result == -1) {
      int errorNumber = errno;
      ThrowExceptionForFileAccessError(
        u8"Could not move file cursor via fseek()", errorNumber
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t PosixFileApi::Tell(FILE *file) {
    long fileCursorPosition = ::ftell(file);
    if(fileCursorPosition == -1) {
      int errorNumber = errno;
      ThrowExceptionForFileAccessError(
        u8"Could not query current file cursor position", errorNumber
      );
    }

    return static_cast<std::uint64_t>(fileCursorPosition);
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixFileApi::ThrowExceptionForFileAccessError(
    const std::string &errorMessage, int errorNumber
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(PosixApi::GetErrorMessage(errorNumber));

    throw Errors::FileAccessError(
      std::error_code(errorNumber, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Platform

#endif // !defined(NUCLEX_PIXELS_WINDOWS)
