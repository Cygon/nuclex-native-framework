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

#ifndef NUCLEX_PIXELS_ERRORS_FILEFORMATERROR_H
#define NUCLEX_PIXELS_ERRORS_FILEFORMATERROR_H

#include "Nuclex/Pixels/Config.h"

#include <stdexcept>

namespace Nuclex { namespace Pixels { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that a file format was not supported or didn't conform to expectations
  /// </summary>
  /// <remarks>
  ///   <para>
  ///     This error is thrown when an image file format encoder or decoder runs into
  ///     a problem, such as a corrupted file, unsupported file format features or
  ///     if a file type is simply not recognized. If you implement your own
  ///     <see cref="Nuclex.Pixels.Storage.BitmapCodec" />, throw this exception if
  ///     you encounter invalid information in the file.
  ///   </para>
  ///   <para>
  ///     If you encounter this exception while using the library, you should consider
  ///     it as a permanent error. The file you are attempting to load is broken,
  ///     or if you're trying to save, the selected image file format cannot support
  ///     the image you're trying to save (i.e. floating point pixel format in jpg).
  ///   </para>
  /// </remarks>
  class FileFormatError : public std::runtime_error {

    /// <summary>Initializes a new file format error</summary>
    /// <param name="message">Message that describes the error</param>
    public: explicit FileFormatError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a new file format error</summary>
    /// <param name="message">Message that describes the error</param>
    public: explicit FileFormatError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Errors

#endif // NUCLEX_PIXELS_ERRORS_FILEFORMATERROR_H
