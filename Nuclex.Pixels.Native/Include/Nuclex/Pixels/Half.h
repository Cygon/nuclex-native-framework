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

#ifndef NUCLEX_PIXELS_HALF_H
#define NUCLEX_PIXELS_HALF_H

#include "Nuclex/Pixels/Config.h"

#include <cstdint>
#include <limits>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Half-precision (16 bit) floating point number</summary>
  /// <remarks>
  ///   The format matches the IEEE-754 binary-16 specification
  ///   http://en.wikipedia.org/wiki/Half-precision_floating-point_format
  /// </remarks>
  class Half {

    /// <summary>The value 1.0 as a half-precision float</summary>
    public: NUCLEX_PIXELS_API static const Half One;

    /// <summary>The value 0.0 as a half-precision float</summary>
    public: NUCLEX_PIXELS_API static const Half Zero;

    /// <summary>Initializes a new half-precision floating point value</summary>
    /// <remarks>
    ///   To emulate the behavior of other C++ primitive types, the value remains
    ///   uninitialized and it is up to the user to make sure a value is assigned before
    ///   the variable is accessed.
    /// </remarks>
    public: NUCLEX_PIXELS_API Half() {}

    /// <summary>Initializes a new half-precision floating point value</summary>
    /// <param name="value">Floating point value the half will be initialized with</param>
    public: NUCLEX_PIXELS_API Half(float value) :
      bits(BitsFromFloat(value)) {}

    /// <summary>Converts the half precision floating point value into a float</summary>
    /// <returns>A float equivalent to the half precision floating point value</returns>
    public: NUCLEX_PIXELS_API operator float() const {
      return FloatFromBits(this->bits);
    }

    /// <summary>Builds a half directly from bits stored in a 16 bit unsigned integer</summary>
    /// <param name="bits">Bits of the half precision integer</param>
    /// <returns>The half precision integer constructed from the provided bits</returns>
    public: NUCLEX_PIXELS_API static Half FromBits(std::uint16_t bits) {
      Half value;
      value.bits = bits;
      return value;
    }

    /// <summary>Returns the bits of the half stored in a 16 bit unsigned integer</summary>
    /// <returns>A 16 unsigned integer containing the bits making up the half</returns>
    public: NUCLEX_PIXELS_API std::uint16_t GetBits() const {
      return this->bits;
    }

    // CHECK: Find method to convert a normalized byte to half without converting to float
    /// <summary>Converts a byte into a half precision float</summary>
    /// <param name="value">Byte that will be converted</param>
    /// <returns>The half-precision float equivalent to the byte</returns>
    public: NUCLEX_PIXELS_API static Half FromNormalizedByte(std::uint8_t value) {
      float valueAsFloat = static_cast<float>(value) / 255.0f;
      return Half(valueAsFloat);
    }

    // CHECK: Find method to convert a half to normalized byte without converting to float
    /// <summary>Converts the half precision float into a normalized byte</summary>
    /// <returns>The equivalent normalized byte to the input half-precision float</returns>
    public: NUCLEX_PIXELS_API std::uint8_t ToNormalizedByte() const {
      float valueAsFloat = FloatFromBits(this->bits);
      if(valueAsFloat <= 0.0f) {
        return 0;
      } else if(valueAsFloat >= 1.0f) {
        return 255;
      } else {
        return static_cast<std::uint8_t>(valueAsFloat * 255.0f) + 1;
      }
    }

    /// <summary>
    ///   Converts a floating point value into a half-precision floating point value
    /// </summary>
    /// <param name="value">Floating point value that will be converted</param>
    /// <returns>
    ///   The equivalent half-precision floating point value to the input value
    /// </returns>
    /// <remarks>
    ///   Based on a code snippet by Phermost
    ///   http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
    /// </remarks>
    public: NUCLEX_PIXELS_API static std::uint16_t BitsFromFloat(float value);

    /// <summary>
    ///   Converts a half-precision floating point value to a floating point value
    /// </summary>
    /// <param name="bits">Half-precision floating point bits that will be converted</param>
    /// <returns>The equivalent floating point value to the input value</returns>
    /// <remarks>
    ///   Based on a code snippet by Phermost
    ///   http://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
    /// </remarks>
    public: NUCLEX_PIXELS_API static float FloatFromBits(std::uint16_t bits);

    /// <summary>Stores the bits of the half-precision floating point value</summary>
    private: std::uint16_t bits;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

namespace std {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Numeric limits for half precision floats</summary>
  /// <remarks>
  ///   Users are not allowed to add anything to the std namespace. However, an exception
  ///   has been defined for supporting custom, non-complex types in std::numeric_limits.
  /// </remarks>
  template<> class numeric_limits<Nuclex::Pixels::Half> {

    /// <summary>Type for which the class is providing informations</summary>
    public: typedef Nuclex::Pixels::Half _Ty;

    /// <summary>
    ///   Minimum finite positive value that is representable by a half precision float
    /// </summary>
    public: static _Ty min() throw() {
      return _Ty(std::uint16_t(1024));
    }

    /// <summary>
    ///   Maximum finite value that is representable by a half precision float
    /// </summary>
    public: static _Ty max() throw() {
      return _Ty(std::uint16_t(31743));
    }

    /// <summary>
    ///   Lowest finite value that is representable by a half precision float
    /// </summary>
    public: static _Ty lowest() throw() {
      return _Ty(std::uint16_t(64511));
    }

    /// <summary>Smallest effective increment from the value 1.0</summary>
    public: static _Ty epsilon() throw() {
      return _Ty(std::uint16_t(0x3c01));
    }

    /// <summary>Largest possible rounding error within representable numeric range</summary>
    public: static _Ty round_error() throw() {
      return _Ty::Zero; // TODO: Determine rounding error
    }

    /// <summary>Minimum denormalized value</summary>
    public: static _Ty denorm_min() throw() {
      return _Ty::Zero; // TODO: Determine minimum denormalized value
    }

    /// <summary>Positive infinity</summary>
    public: static _Ty infinity() throw() {
      return _Ty(std::uint16_t(0x7c00));
    }

    /// <summary>A quiet not-a-number value</summary>
    public: static _Ty quiet_NaN() throw() {
      return _Ty::Zero; // TODO: Determine quiet not-a-number value
    }

    /// <summary>A signaling not-a-number value</summary>
    public: static _Ty signaling_NaN() throw() {
      return _Ty::Zero; // TODO: Determine signaling not-a-number value
    }

    /// <summary>Number of binary digits that can directly be represented (mantissa)</summary>
    public: const int digits = 0; // TODO: Determine mantissa

    /// <summary>Number of base10 digits that can directly be represented</summary>
    public: const int digits10 = 0; // TODO: Determine base10 directly representable digits

    /// <summary>I have no idea...</summary>
    public: const int max_digits10 = 0; // TODO: Determine whatever this is

    /// <summary>I have no idea...</summary>
    public: const int max_exponent = 0; // TODO: Determine whatever this is

    /// <summary>I have no idea...</summary>
    public: const int max_exponent10 = 0; // TODO: Determine whatever this is

    /// <summary>I have no idea...</summary>
    public: const int min_exponent = 0; // TODO: Determine whatever this is

    /// <summary>I have no idea...</summary>
    public: const int min_exponent10 = 0; // TODO: Determine whatever this is

    private: numeric_limits(const numeric_limits &);
    private: numeric_limits &operator =(const numeric_limits &);

  };

  // ------------------------------------------------------------------------------------------- //

}

#endif // NUCLEX_PIXELS_HALF_H
