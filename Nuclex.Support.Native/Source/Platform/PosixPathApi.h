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

#ifndef NUCLEX_SUPPORT_PLATFORM_POSIXPATHAPI_H
#define NUCLEX_SUPPORT_PLATFORM_POSIXPATHAPI_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_WIN32)

#include "PosixApi.h"

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Posix file API</summary>
  class PosixPathApi {

    /// <summary>Checks if the specified path is a relative path</summary>
    /// <param name="path">Path that will be checked</param>
    /// <returns>True if the path is a relative path</returns>
    public: static bool IsPathRelative(const std::string &path);

    /// <summary>Appends one path to another</summary>
    /// <param name="path">Path to which another path will be appended</param>
    /// <param name="extra">Other path that will be appended</param>
    public: static void AppendPath(std::string &path, const std::string &extra);

    /// <summary>Removes the file name from a path containing a file name</summary>
    /// <param name="path">Path from which the file name will be removed</param>
    public: static void RemoveFileFromPath(std::string &path);

    /// <summary>Checks if the specified path exists and if it is a file</summary>
    /// <param name="path">Path that will be checked</param>
    /// <returns>True if the path exists and is a file, false otherwise</returns>
    public: static bool DoesFileExist(const std::string &path);

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WIN32)

#endif // NUCLEX_SUPPORT_PLATFORM_POSIXPATHAPI_H
