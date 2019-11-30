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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "RealFile.h"
#include "Nuclex/Pixels/Errors/FileAccessError.h"

#include <vector>
#include <limits>
#include <cassert>

#include "RealFile.Windows.inl"
#include "RealFile.Posix.inl"
#include "RealFile.Linux.inl"

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  RealFile::RealFile(
    const std::string &path, bool promiseSequentialAccess, bool readOnly
  ) : position(0) {
#if defined(NUCLEX_PIXELS_WIN32)

    if(readOnly) {
      this->fileHandle = openWindowsFileForReading(path, promiseSequentialAccess);
      this->length = getWindowsFileSize(this->fileHandle);
    } else {
      this->fileHandle = openWindowsFileForWriting(path, promiseSequentialAccess);
      this->length = 0;
    }

#elif defined(NUCLEX_PIXELS_LINUX)

    (void)promiseSequentialAccess; // Not supported here.
    if(readOnly) {
      this->fileDescriptor = ::open(path.c_str(), O_RDONLY | O_NOATIME);
      if(this->fileDescriptor == -1) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
      this->length = getLinuxFileSize(this->fileDescriptor);
    } else {
      this->fileDescriptor = ::open(
        path.c_str(),
        O_CREAT | O_WRONLY | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
      );
      if(this->fileDescriptor == -1) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
      this->length = 0;
    }

#else // No Windows, no Linux, let's try Posix

    (void)promiseSequentialAccess; // Not supported here.
    if(readOnly) {
      this->filePointer = ::fopen(path.c_str(), "rb");
      if(this->filePointer == nullptr) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
      this->length = getPosixFileSize(path.c_str());
    } else {
      this->filePointer = ::fopen(path.c_str(), "wb");
      if(this->filePointer == nullptr) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
      this->length = 0;
    }

#endif
  }

  // ------------------------------------------------------------------------------------------- //

  RealFile::~RealFile() {
#if defined(NUCLEX_PIXELS_WIN32)
    ::CloseHandle(this->fileHandle);
#elif defined(NUCLEX_PIXELS_LINUX)
    ::close(this->fileDescriptor);
#else // No Windows, no Linux, let's try Posix
    ::fclose(this->filePointer);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::ReadAt(
    std::uint64_t start, std::size_t byteCount, std::uint8_t *buffer
  ) const {
#if defined(NUCLEX_PIXELS_WIN32)

    if(start != this->position) {
      moveWindowsFileCursor(this->fileHandle, start);
      this->position = start;
    }
    readWindowsFile(this->fileHandle, this->position, byteCount, buffer);

#elif defined(NUCLEX_PIXELS_LINUX)

    ssize_t readByteCount;
    if(start == this->position) {
      readByteCount = ::read(this->fileDescriptor, buffer, byteCount);
      if(readByteCount == -1) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
      this->position += readByteCount;
    } else {
      readByteCount = ::pread(this->fileDescriptor, buffer, byteCount, start);
      if(readByteCount == -1) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
    }
    if(static_cast<std::size_t>(readByteCount) != byteCount) {
      throwPosixFileAccessError(EINVAL);
    }

#else // No Windows, no Linux, let's try Posix

    if(start != this->position) {
      movePosixFileCursor(this->filePointer, start);
      this->position = start;
    }
    readPosixFile(this->filePointer, this->position, byteCount, buffer);

#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void RealFile::WriteAt(
    std::uint64_t start, std::size_t byteCount, const std::uint8_t *buffer
  ) {
#if defined(NUCLEX_PIXELS_WIN32)

    if(start != this->position) {
      if(start > this->length) { // Don't allow writing past end with gap
        throwWindowsFileAccessError(ERROR_HANDLE_EOF);
      }
      moveWindowsFileCursor(this->fileHandle, start);
    }
    writeWindowsFile(this->fileHandle, this->position, byteCount, buffer);

#elif defined(NUCLEX_PIXELS_LINUX)

    ssize_t writtenByteCount;
    if(start == this->position) {
      writtenByteCount = ::write(this->fileDescriptor, buffer, byteCount);
      if(writtenByteCount == -1) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
      this->position += writtenByteCount;
    } else {
      if(start > this->length) { // Don't allow writing past end with gap
        throwPosixFileAccessError(EINVAL);
      }
      writtenByteCount = ::pwrite(this->fileDescriptor, buffer, byteCount, start);
      if(writtenByteCount == -1) {
        int errorNumber = errno;
        throwPosixFileAccessError(errorNumber);
      }
    }
    if(static_cast<std::size_t>(writtenByteCount) != byteCount) {
      throwPosixFileAccessError(EINVAL);
    }

#else // No Windows, no Linux, let's try Posix

    if(start != this->position) {
      if(start > this->length) { // Don't allow writing past end with gap
        throwPosixFileAccessError(EINVAL);
      }
      movePosixFileCursor(this->filePointer, start);
      this->position = start;
    }
    writePosixFile(this->filePointer, this->position, byteCount, buffer);

#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
