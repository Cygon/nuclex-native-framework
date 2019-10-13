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

#ifndef NUCLEX_PIXELS_INT128_H
#define NUCLEX_PIXELS_INT128_H

#include "Nuclex/Pixels/Config.h"

#include <cstdint>
#include <limits>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>128 bit unsigned intgeer</summary>
  class UInt128 {

    /// <summary>Initializes a new 128 bit unsigned integer</summary>
    /// <remarks>
    ///   To emulate the behavior of other C++ primitive types, the value remains
    ///   uninitialized and it is up to the user to make sure a value is assigned before
    ///   the variable is accessed.
    /// </remarks>
    public: NUCLEX_PIXELS_API UInt128() {}

    /// <summary>Initializes a new 128 bit unsigned integer from an 8 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: NUCLEX_PIXELS_API UInt128(std::uint8_t value) :
      mostSignificant(0),
      leastSignificant(value) {}

    /// <summary>Initializes a new 128 bit unsigned integer from a 16 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: NUCLEX_PIXELS_API UInt128(std::uint16_t value) :
      mostSignificant(0),
      leastSignificant(value) {}

    /// <summary>Initializes a new 128 bit unsigned integer from a 32 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: NUCLEX_PIXELS_API UInt128(std::uint32_t value) :
      mostSignificant(0),
      leastSignificant(value) {}

    /// <summary>Initializes a new 128 bit unsigned integer from a 64 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: NUCLEX_PIXELS_API UInt128(std::uint64_t value) :
      mostSignificant(0),
      leastSignificant(value) {}

    /// <summary>Stores the most signifcant 64 bits of the 128 bit integer</summary>
    private: std::uint64_t mostSignificant;
    /// <summary>Stores the least signifcant 64 bits of the 128 bit integer</summary>
    private: std::uint64_t leastSignificant;

  };

  // ------------------------------------------------------------------------------------------- //

  #if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)

  /// <summary>Alias for the best 128 bit integer implementation to use</summary>
  typedef unsigned __int128 uint128_t;

  #else

  /// <summary>Alias for the best 128 bit integer implementation to use</summary>
  typedef UInt128 uint128_t;

  #endif

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

namespace std {

  // ------------------------------------------------------------------------------------------- //
#if 0
  /// <summary>Numeric limits for half precision floats</summary>
  /// <remarks>
  ///   Users are not allowed to add anything to the std namespace. However, an exception
  ///   has been defined for supporting custom, non-complex types in std::numeric_limits.
  /// </remarks>
  template<> class numeric_limits<Nuclex::Pixels::Int128> {

    /// <summary>Type for which the class is providing informations</summary>
    public: typedef Nuclex::Pixels::Int128 _Ty;

    /// <summary>
    ///   Minimum finite positive value that is representable by a half precision float
    /// </summary>
    public: static _Ty min() throw() {
      return _Ty(0);
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
#endif

  // ------------------------------------------------------------------------------------------- //

}

#endif // NUCLEX_PIXELS_INT128_H
