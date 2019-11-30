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

#ifndef NUCLEX_PIXELS_ERRORS_FILEACCESSERROR_H
#define NUCLEX_PIXELS_ERRORS_FILEACCESSERROR_H

#include "Nuclex/Pixels/Config.h"

#include <system_error>

namespace Nuclex { namespace Pixels { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a file was not found or could not be opened</summary>
  /// <remarks>
  ///   <para>
  ///     This error will be thrown if anything went wrong accessing the data of a
  ///     virtual file (<see cref="Nuclex.Pixels.Storage.VirtualFile" />). If you implement
  ///     your own data sources/sinks using the virtual file interface, all exceptions
  ///     thrown should ideally inherit from this esception.
  ///   </para>
  ///   <para>
  ///     If you get this error while working with Nuclex.Pixels, it means that your
  ///     image load or save operation has failed not due to a problem with the library,
  ///     but with the underlying stream - a file may be unreadable or your custom
  ///     virtual file implementation will have failed to fetch and transmit data.
  ///   </para>
  /// </remarks>
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

}}} // namespace Nuclex::Pixels::Errors

#endif // NUCLEX_PIXELS_ERRORS_FILEACCESSERROR_H
