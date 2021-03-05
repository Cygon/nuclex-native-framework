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

#ifndef NUCLEX_STORAGE_HELPERS_POSIXAPI_H
#define NUCLEX_STORAGE_HELPERS_POSIXAPI_H

#include "Nuclex/Storage/Config.h"

#if !defined(NUCLEX_STORAGE_WIN32)

#include <string>

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Offers generic methods for dealing with the Posix API</summary>
  class PosixApi {

    /// <summary>Returns the error message for the specified error number</summary>
    /// <param name="errorNumber">
    ///   Error number for which the error message will be looked up
    /// </param>
    /// <returns>The error message for the specified error number</param>
    public: static std::string GetErrorMessage(int errorNumber);

    /// <summary>Checks if the error number indicates a path problem</summary>
    /// <param name="errorNumber">Error number that will be checked</param>
    /// <returns>True if the specified error is a path error</returns>
    /// <remarks>
    ///   Any error that indicates a bad path (file not found, symlink loop,
    ///   malformed path, etc.) is covered under this.
    /// </remarks>
    public: static bool IsPathError(int errorNumber);

    /// <summary>Retrieves the value held by an environment variable</summary>
    /// <param name="name">Name of the environment variable that will be looked up</param>
    /// <param name="value">String that will receive the value of the variable</param>
    /// <returns>
    ///   True if the environment variable was retrieved, false if the variable
    ///   did not exist and the <see cref="value" /> parameter is unchanged.
    /// </returns>
    public: static bool GetEnvironmentVariable(const std::string &name, std::string &value);

    /// <summary>Throws the appropriate exception for an error reported by the OS</summary>
    /// <param name="errorMessage">
    ///   Error message that shoudl be included in the exception, will be prefixed to
    ///   the OS error message
    /// </param>
    /// <param name="errorNumber">Value that 'errno' had at the time of failure</param>
    public: [[noreturn]] static void ThrowExceptionForSystemError(
      const std::string &errorMessage, int errorNumber
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers

#endif // !defined(NUCLEX_STORAGE_WIN32)

#endif // NUCLEX_STORAGE_HELPERS_POSIXAPI_H
