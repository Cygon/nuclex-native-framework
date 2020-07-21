#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_PIXELS_ERRORS_PIXELFORMATERROR_H
#define NUCLEX_PIXELS_ERRORS_PIXELFORMATERROR_H

#include "Nuclex/Pixels/Config.h"

#include <stdexcept>

namespace Nuclex { namespace Pixels { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that the pixel format was not suitable for the intended use
  /// </summary>
  /// <remarks>
  ///   <para>
  ///     This error can be thrown by algorithms that require specific pixel formats to do
  ///     their work. For example, if an embedded filter or resizing algorithm can't deal
  ///     with signed integer pixel formats.
  ///   </para>
  ///   <para>
  ///     If a method or utility class has specific requirements for the pixel format,
  ///     this information should be documented in its public interface. A pixel format
  ///     conversion should only happen automatically if no data is lost (i.e. all channels
  ///     are widened to a higher bit count temporarily).
  ///   </para>
  ///   <para>
  ///     In some cases, such as user-interactive applications, it may be appropriate to first
  ///     try the native pixel format and, if rejected through this exception, convert it to
  ///     a compatible pixel format before repeating the call. Logging a warning or displaying
  ///     the problem to the user is probably a good idea in such cases.
  ///   </para>
  /// </remarks>
  class PixelFormatError : public std::runtime_error {

    /// <summary>Initializes a new bad pixel format error</summary>
    /// <param name="message">Message that describes the error</param>
    public: explicit PixelFormatError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a bad pixel format error</summary>
    /// <param name="message">Message that describes the error</param>
    public: explicit PixelFormatError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Errors

#endif // NUCLEX_PIXELS_ERRORS_PIXELFORMATERROR_H
