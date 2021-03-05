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
#error This file must be included through RealFile.cpp
#endif

// We assume the posix functions were inline-included before this file :-)
//#include <cerrno> // errno
//#include <cstring> // strerror()
#if defined(NUCLEX_PIXELS_LINUX)
#include <fcntl.h> // open(), creat()
#include <unistd.h> // close()
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_LINUX)

  /// <summary>Determines the size of a file using the Linux API</summary>
  /// <param name="fileDescriptor">
  ///   File descriptor of the file whose size will be determined
  /// </param>
  /// <returns>The size of the specified file</returns>
  std::uint64_t getLinuxFileSize(int fileDescriptor) {
    struct stat fileStatus;

    int failed = ::fstat(fileDescriptor, &fileStatus);
    if(failed) {
      int errorNumber = errno;
      throwPosixFileAccessError(errorNumber);
    }

    return static_cast<std::uint64_t>(fileStatus.st_size);
  }

#endif // defined(NUCLEX_PIXELS_LINUX)

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace
