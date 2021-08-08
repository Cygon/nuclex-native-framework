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

#include "RealFile.h"

#if defined(NUCLEX_PIXELS_LINUX)
#include "../Platform/LinuxFileApi.h"
#include <unistd.h> // ::read(), ::write(), ::close(), etc.
#elif defined(NUCLEX_PIXELS_WINDOWS)
#include "../Platform/WindowsFileApi.h"
#endif

#if !defined(NUCLEX_PIXELS_WINDOWS)
#include "../Platform/PosixFileApi.h" // for ThrowExceptionForFileAccessError(), etc.
#endif

#include <cassert> // for assert()

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  RealFile::RealFile(
    const std::string &path, bool promiseSequentialAccess, bool readOnly
  ) : position(0) {
#if defined(NUCLEX_PIXELS_LINUX)
    (void)promiseSequentialAccess; // Not supported here.
    if(readOnly) {
      this->fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(path);
      this->length = Platform::LinuxFileApi::StatFileSize(this->fileDescriptor);
    } else {
      this->fileDescriptor = Platform::LinuxFileApi::OpenFileForWriting(path);
      this->length = 0;
    }
#elif defined(NUCLEX_PIXELS_WINDOWS)
    if(readOnly) {
      this->fileHandle = Platform::WindowsFileApi::OpenFileForReading(
        path, promiseSequentialAccess
      );
      this->length = Platform::WindowsFileApi::GetFileSize(this->fileHandle);
    } else {
      this->fileHandle = Platform::WindowsFileApi::OpenFileForWriting(
        path, promiseSequentialAccess
      );
      this->length = 0;
    }
#else // No Windows, no Linux, let's try Posix
    (void)promiseSequentialAccess; // Not supported here.
    if(readOnly) {
      this->file = Platform::PosixFileApi::OpenFileForReading(path);
      Platform::PosixFileApi::Seek(this->file, 0, SEEK_END);
      this->length = Platform::PosixFileApi::Tell(this->file);
      Platform::PosixFileApi::Seek(this->file, 0, SEEK_SET);
    } else {
      this->file = Platform::PosixFileApi::OpenFileForWriting(path);
      this->length = 0;
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  RealFile::~RealFile() {
#if defined(NUCLEX_PIXELS_LINUX)
    int result = ::close(this->fileDescriptor);
    NUCLEX_PIXELS_NDEBUG_UNUSED(result);
    assert((result != -1) && u8"File descriptor is closed successfully");
#elif defined(NUCLEX_PIXELS_WINDOWS)
    BOOL result = ::CloseHandle(this->fileHandle);
    NUCLEX_PIXELS_NDEBUG_UNUSED(result);
    assert((result != FALSE) && u8"File handle is closed successfully");
#else // No Windows, no Linux, let's try Posix
    int result = ::fclose(this->file);
    NUCLEX_PIXELS_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"File is closed successfully");
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
  ) const {
#if defined(NUCLEX_PIXELS_LINUX)
    if(start == this->position) { // Prefer read() to support stdin etc.
      std::size_t remainingByteCount = byteCount;
      for(;;) {
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
          this->fileDescriptor, buffer, remainingByteCount
        );
        this->position += readByteCount;

        if(likely(readByteCount == remainingByteCount)) {
          return;
        } else if(unlikely(readByteCount == 0)) {
          Platform::PosixFileApi::ThrowExceptionForFileAccessError(
            u8"Encountered unexpected end of file", EIO
          );
        }

        buffer += readByteCount;
        remainingByteCount -= readByteCount;
      }
    } else { // If seeking needed anyway, use pread() instead
      std::size_t remainingByteCount = byteCount;
      for(;;) {
        std::size_t readByteCount = Platform::LinuxFileApi::PositionalRead(
          this->fileDescriptor, buffer, byteCount, start
        );
        if(likely(readByteCount == remainingByteCount)) {
          return;
        } else if(unlikely(readByteCount == 0)) {
          Platform::PosixFileApi::ThrowExceptionForFileAccessError(
            u8"Encountered unexpected end of file", EIO
          );
        }

        buffer += readByteCount;
        start += readByteCount;
        remainingByteCount -= readByteCount;
      }
    }
#elif defined(NUCLEX_PIXELS_WINDOWS)
    if(start != this->position) {
      Platform::WindowsFileApi::Seek(this->fileHandle, start, FILE_BEGIN);
      this->position = start;
    }
    std::size_t remainingByteCount = byteCount;
    for(;;) {
      std::size_t readByteCount = Platform::WindowsFileApi::Read(
        this->fileHandle, buffer, remainingByteCount
      );
      this->position += readByteCount;

      if(likely(readByteCount == remainingByteCount)) {
        return; // All done
      } else if(unlikely(readByteCount == 0)) {
        Platform::WindowsFileApi::ThrowExceptionForFileAccessError(
          u8"Encountered unexpected end of file", ERROR_HANDLE_EOF
        );
      }

      remainingByteCount -= readByteCount;
      buffer -= readByteCount;
    }
#else // No Windows, no Linux, let's try Posix
    if(start != this->position) {
      Platform::PosixFileApi::Seek(this->file, start, SEEK_SET);
      this->position = start;
    }

    std::size_t remainingByteCount = byteCount;
    for(;;) {
      std::size_t readByteCount = Platform::PosixFileApi::Read(
        this->file, buffer, remainingByteCount
      );
      this->position += readByteCount;

      if(likely(readByteCount == remainingByteCount)) {
        return; // All done
      } else if(unlikely(readByteCount == 0)) {
        Platform::PosixFileApi::ThrowExceptionForFileAccessError(
          u8"Encountered unexpected end of file", EIO
        );
      }

      remainingByteCount -= readByteCount;
      buffer -= readByteCount;
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
  ) {
#if defined(NUCLEX_PIXELS_LINUX)
    std::size_t writtenByteCount;
    if(start == this->position) { // Prefer write() to support stdout etc.
      writtenByteCount = Platform::LinuxFileApi::Write(
        this->fileDescriptor, buffer, byteCount
      );
      this->position += writtenByteCount;
      if(this->position > this->length) {
        this->length = this->position;
      }
    } else { // If seeking needed anyway, use pwrite()
      if(start > this->length) { // Don't allow writing past end with gap
        Platform::PosixFileApi::ThrowExceptionForFileAccessError(
          u8"Attempted write position would leave a gap in the file", EINVAL
        );
      }
      writtenByteCount = Platform::LinuxFileApi::PositionalWrite(
        this->fileDescriptor, buffer, byteCount, start
      );
      if(start + writtenByteCount > this->length) {
        this->length = start + writtenByteCount;
      }
    }
    if(writtenByteCount != byteCount) {
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Write finished without storing the entire buffer", EIO
      );
    }
#elif defined(NUCLEX_PIXELS_WINDOWS)
    if(start != this->position) {
      if(start > this->length) { // Don't allow writing past end with gap
        Platform::WindowsFileApi::ThrowExceptionForFileAccessError(
          u8"Attempted write position would leave a gap in the file", ERROR_HANDLE_EOF
        );
      }
      Platform::WindowsFileApi::Seek(this->fileHandle, start, FILE_BEGIN);
    }

    std::size_t writtenByteCount = Platform::WindowsFileApi::Write(
      this->fileHandle, buffer, byteCount
    );
    this->position += writtenByteCount;
    if(this->position > this->length) {
      this->length = this->position;
    }
    if(writtenByteCount != byteCount) {
      Platform::WindowsFileApi::ThrowExceptionForFileAccessError(
        u8"Write finished without storing the entire buffer", ERROR_WRITE_FAULT
      );
    }
#else // No Windows, no Linux, let's try Posix
    if(start != this->position) {
      if(start > this->length) { // Don't allow writing past end with gap
        Platform::PosixFileApi::ThrowExceptionForFileAccessError(
          u8"Attempted write position would leave a gap in the file", EIO
        );
      }
      Platform::PosixFileApi::Seek(this->file, start, SEEK_SET);
    }

    std::size_t writtenByteCount = Platform::PosixFileApi::Write(
      this->file, buffer, byteCount
    );
    this->position += writtenByteCount;
    if(this->position > this->length) {
      this->length = this->position;
    }
    if(writtenByteCount != byteCount) {
      Platform::PosixFileApi::ThrowExceptionForFileAccessError(
        u8"Write finished without storing the entire buffer", EIO
      );
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
