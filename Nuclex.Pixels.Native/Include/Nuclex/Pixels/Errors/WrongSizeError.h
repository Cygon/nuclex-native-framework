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

#ifndef NUCLEX_PIXELS_ERRORS_WRONGSIZEERROR_H
#define NUCLEX_PIXELS_ERRORS_WRONGSIZEERROR_H

#include "Nuclex/Pixels/Config.h"

#include <exception> // for std::runtime_error
#include <string> // for std::runtime_error

// DESIGN: Should WrongSizeError be classified as a std::runtime_error?
//
// The C++ standard doesn't recommend the split into std::logic_error and
// std::runtime_error anymore, but for what it's worth, which of the two
// does WrongSizeError belong to?
//
// Creating a Bitmap in the wrong format would be a programming error,
// thus std::logic_error would be correct. But than again, the image may
// have changed, thus it's an std::runtime_error (environment changed at
// runtime due to uncontrollable external interference)
//

namespace Nuclex { namespace Pixels { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a bitmap's resolution didn't match expectations</summary>
  /// <remarks>
  ///   <para>
  ///     If you use the <see cref="Nuclex.Pixels.Storage.BitmapSerializer.Reload" />
  ///     method to load an image into an existing bitmap, the resolution of the bitmap
  ///     has to match the file's resolution exactly. If it doesn't this exception
  ///     is thrown.
  ///   </para>
  ///   <para>
  ///     It may also be thrown by some algorithms (i.e. image rescaling) if there are
  ///     constraints (for example, the image width and height may need to be multiples
  ///     of some value).
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE WrongSizeError : public std::exception {

    /// <summary>Initializes a new file access error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_PIXELS_API explicit WrongSizeError(const std::string &message) :
      message(message) {}

    /// <summary>Initializes a new file access error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_PIXELS_API explicit WrongSizeError(const char *message) :
      message(message) {}

    /// <summary>Provides the error message carried by the exception</summary>
    /// <returns>A pointer to a null-terminated string describing the error</returns>
    public: virtual const char *what() const noexcept override {
      return this->message.c_str();
    }

    /// <summary>Error message being transported with the exception</summary<>
    private: std::string message;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Errors

#endif // NUCLEX_PIXELS_ERRORS_WRONGSIZEERROR_H
