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
#define NUCLEX_SUPPORT_SOURCE 1

#include "PosixPathApi.h"

#if !defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Text/LexicalAppend.h"

#include <limits.h> // for PATH_MAX
#include <sys/stat.h> // for ::stat()

namespace {

  // ------------------------------------------------------------------------------------------- //
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  bool PosixPathApi::IsPathRelative(const std::string &path) {
    std::string::size_type length = path.length();
    if(length == 0) {
      return true;
    }

    if(length >= 2) {
      if((path[0] == '~') && (path[1] == L'/')) {
        return false;
      }
    }

    return (path[0] != '/');
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixPathApi::AppendPath(std::string &path, const std::string &extra) {
    std::string::size_type length = path.length();
    if(length == 0) {
      path.assign(extra);
    } else {
      if(path[length - 1] != '/') {
        path.push_back('/');
      }
      path.append(extra);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void PosixPathApi::RemoveFileFromPath(std::string &path) {
    std::string::size_type lastBackslashIndex = path.find_last_of('/');
    if(lastBackslashIndex != std::string::npos) {
      path.resize(lastBackslashIndex + 1); // Keep the slash on
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool PosixPathApi::DoesFileExist(const std::string &path) {
    struct ::stat fileStatus;

    int result = ::stat(path.c_str(), &fileStatus);
    if(result == -1) {
      int errorNumber = errno;

      // This is an okay outcome for us: the file or directory does not exist.
      if((errorNumber == ENOENT) || (errorNumber == ENOTDIR)) {
        return false;
      }

      std::string errorMessage(u8"Could not obtain file status for '");
      errorMessage.append(path);
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WIN32)
