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

#ifndef NUCLEX_STORAGE_ERRORS_FILEACCESSERROR_H
#define NUCLEX_STORAGE_ERRORS_FILEACCESSERROR_H

#include "Nuclex/Storage/Config.h"

#include <system_error>

namespace Nuclex { namespace Storage { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a file was not found or could not be opened</summary>
  class FileAccessError : public std::system_error {

    /// <summary>Initializes a new file access error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: explicit FileAccessError(std::error_code errorCode, const std::string &message) :
      std::system_error(errorCode, message) {}

    /// <summary>Initializes a new file access error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: explicit FileAccessError(std::error_code errorCode, const char *message) :
      std::system_error(errorCode, message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Errors

#endif // NUCLEX_STORAGE_ERRORS_FILEACCESSERROR_H
