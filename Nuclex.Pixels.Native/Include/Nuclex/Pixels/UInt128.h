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

#ifndef NUCLEX_PIXELS_UINT128_H
#define NUCLEX_PIXELS_UINT128_H

#include "Nuclex/Pixels/Config.h"

#include <cstdint> // for std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t
#include <cassert> // for assert()
#include <limits> // for std::numeric_limits
#include <type_traits> // for std::true_type

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>128 bit unsigned intgeer</summary>
  class NUCLEX_PIXELS_TYPE UInt128 {

    /// <summary>Initializes a new 128 bit unsigned integer</summary>
    /// <remarks>
    ///   To emulate the behavior of other C++ primitive types, the value remains
    ///   uninitialized and it is up to the user to make sure a value is assigned before
    ///   the variable is accessed.
    /// </remarks>
    public: constexpr NUCLEX_PIXELS_API UInt128() :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(0),
      mostSignificant(0) {}
#else
      mostSignificant(0),
      leastSignificant(0) {}
#endif

    /// <summary>Initializes an 128 bit integer using two 64 bit integers</summary>
    /// <param name="mostSignificant">
    ///   64 bit integer that will be used to fill the 64 most significant bits
    /// </param>
    /// <param name="leastSignificant">
    ///   64 bit integer that will be used to fill the 64 least significant bits
    /// </param>
    public: constexpr NUCLEX_PIXELS_API explicit UInt128(
      std::uint64_t mostSignificant, std::uint64_t leastSignificant
    ) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(leastSignificant),
      mostSignificant(mostSignificant) {}
#else
      mostSignificant(mostSignificant),
      leastSignificant(leastSignificant) {}
#endif

    /// <summary>Initializes a new 128 bit unsigned integer from an integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: constexpr NUCLEX_PIXELS_API UInt128(int value) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(static_cast<std::uint64_t>(value)),
      mostSignificant((value < 0) ? 0xFFFFFFFFFFFFFFFF : 0) {}
#else
      mostSignificant((value < 0) ? 0xFFFFFFFFFFFFFFFF : 0),
      leastSignificant(static_cast<std::uint64_t>(value)) {}
#endif

    /// <summary>Initializes a new 128 bit unsigned integer from an 8 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: constexpr NUCLEX_PIXELS_API UInt128(std::uint8_t value) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(value),
      mostSignificant(0) {}
#else
      mostSignificant(0),
      leastSignificant(value) {}
#endif

    /// <summary>Initializes a new 128 bit unsigned integer from a 16 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: constexpr NUCLEX_PIXELS_API UInt128(std::uint16_t value) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(value),
      mostSignificant(0) {}
#else
      mostSignificant(0),
      leastSignificant(value) {}
#endif

    /// <summary>Initializes a new 128 bit unsigned integer from a 32 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: constexpr NUCLEX_PIXELS_API UInt128(std::uint32_t value) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(value),
      mostSignificant(0) {}
#else
      mostSignificant(0),
      leastSignificant(value) {}
#endif

    /// <summary>Initializes a new 128 bit unsigned integer from a 64 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: constexpr NUCLEX_PIXELS_API UInt128(std::uint64_t value) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(value),
      mostSignificant(0) {}
#else
      mostSignificant(0),
      leastSignificant(value) {}
#endif

#if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)

  #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wpedantic" // ISO C++ does not support ‘__int128’
  #elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic" // ISO C++ does not support ‘__int128’
  #endif

    /// <summary>Initializes a new 128 bit unsigned integer from a 128 bit integer</summary>
    /// <param name="value">Value with which the 128 bit integer will be initialized</param>
    public: constexpr NUCLEX_PIXELS_API UInt128(unsigned __int128 value) :
#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      leastSignificant(static_cast<std::uint64_t>(value)),
      mostSignificant(static_cast<std::uint64_t>(value >> 64)) {}
#else
      mostSignificant(static_cast<std::uint64_t>(value >> 64)),
      leastSignificant(static_cast<std::uint64_t>(value)) {}
#endif

  #if defined(__clang__)
    #pragma clang diagnostic pop
  #elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic pop
  #endif

#endif // defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)

    /// <summary>Returns only the lower 8 bits of the 128 bit integer</summary>
    /// <returns>The lower 8 bits of the 128 bit integer</returns>
    public: NUCLEX_PIXELS_API explicit operator std::uint8_t() const {
      return static_cast<std::uint8_t>(this->leastSignificant);
    }

    /// <summary>Returns only the lower 16 bits of the 128 bit integer</summary>
    /// <returns>The lower 16 bits of the 128 bit integer</returns>
    public: NUCLEX_PIXELS_API explicit operator std::uint16_t() const {
      return static_cast<std::uint16_t>(this->leastSignificant);
    }

    /// <summary>Returns only the lower 32 bits of the 128 bit integer</summary>
    /// <returns>The lower 32 bits of the 128 bit integer</returns>
    public: NUCLEX_PIXELS_API explicit operator std::uint32_t() const {
      return static_cast<std::uint32_t>(this->leastSignificant);
    }

    /// <summary>Returns only the lower 64 bits of the 128 bit integer</summary>
    /// <returns>The lower 64 bits of the 128 bit integer</returns>
    public: NUCLEX_PIXELS_API explicit operator std::uint64_t() const {
      return this->leastSignificant;
    }

    /// <summary>Initializes the 128 bit integer from an 8 bit integer</summary>
    /// <param name="value">8 bit integer the 128 integer will be initialized from</parma>
    /// <returns>The 128 bit integer</returns>
    public: NUCLEX_PIXELS_API UInt128 &operator =(std::uint8_t value) {
      this->mostSignificant = 0;
      this->leastSignificant = value;
      return *this;
    }

    /// <summary>Initializes the 128 bit integer from a 16 bit integer</summary>
    /// <param name="value">16 bit integer the 128 integer will be initialized from</parma>
    /// <returns>The 128 bit integer</returns>
    public: NUCLEX_PIXELS_API UInt128 &operator =(std::uint16_t value) {
      this->mostSignificant = 0;
      this->leastSignificant = value;
      return *this;
    }

    /// <summary>Initializes the 128 bit integer from a 32 bit integer</summary>
    /// <param name="value">32 bit integer the 128 integer will be initialized from</parma>
    /// <returns>The 128 bit integer</returns>
    public: NUCLEX_PIXELS_API UInt128 &operator =(std::uint32_t value) {
      this->mostSignificant = 0;
      this->leastSignificant = value;
      return *this;
    }

    /// <summary>Initializes the 128 bit integer from a 64 bit integer</summary>
    /// <param name="value">64 bit integer the 128 integer will be initialized from</parma>
    /// <returns>The 128 bit integer</returns>
    public: NUCLEX_PIXELS_API UInt128 &operator =(std::uint64_t value) {
      this->mostSignificant = 0;
      this->leastSignificant = value;
      return *this;
    }

    /// <summary>Checks whether another 128 bit integer is equal to this one</summary>
    /// <param name="other">Other 128 bit integer that will be compared</param>
    /// <returns>True if the other 128 bit integer has the same value as this</returns>
    public: NUCLEX_PIXELS_API bool operator ==(const UInt128 &other) const {
      return (
        (this->leastSignificant == other.leastSignificant) &&
        (this->mostSignificant == other.mostSignificant)
      );
    }

    /// <summary>Checks whether another 128 bit integer is different from this one</summary>
    /// <param name="other">Other 128 bit integer that will be compared</param>
    /// <returns>True if the other 128 bit integer has a different value to this</returns>
    public: NUCLEX_PIXELS_API bool operator !=(const UInt128 &other) const {
      return (
        (this->leastSignificant != other.leastSignificant) ||
        (this->mostSignificant != other.mostSignificant)
      );
    }

    /// <summary>Returns the integer bit-shifted to the left</summary>
    /// <param name="bitOffset">Number of bits the integer will be shifted</param>
    /// <returns>The bit-shifted copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 operator <<(int bitOffset) const {
      assert((bitOffset >= 0) && u8"UInt128 shift operator doesn't support negative shifts");

      if(bitOffset == 0) { // Using <64 case would result in undefined behavior
        return *this;
      } else if(bitOffset < 64) {
        UInt128 shifted;
        shifted.mostSignificant = (this->mostSignificant << bitOffset);
        shifted.mostSignificant |= (this->leastSignificant >> (64 - bitOffset));
        shifted.leastSignificant = this->leastSignificant << bitOffset;
        return shifted;
      } else {
        UInt128 shifted;
        shifted.mostSignificant = (this->leastSignificant << (bitOffset - 64));
        shifted.leastSignificant = 0;
        return shifted;
      }
    }

    /// <summary>Returns the integer bit-shifted to the right</summary>
    /// <param name="bitOffset">Number of bits the integer will be shifted</param>
    /// <returns>The bit-shifted copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 operator >>(int bitOffset) const {
      assert((bitOffset >= 0) && u8"UInt128 shift operator doesn't support negative shifts");

      if(bitOffset == 0) { // Using <64 case would result in undefined behavior
        return *this;
      } else if(bitOffset < 64) {
        UInt128 shifted;
        shifted.leastSignificant = (this->leastSignificant >> bitOffset);
        shifted.leastSignificant |= (this->mostSignificant << (64 - bitOffset));
        shifted.mostSignificant = this->mostSignificant >> bitOffset;
        return shifted;
      } else {
        UInt128 shifted;
        shifted.leastSignificant = (this->mostSignificant >> (bitOffset - 64));
        shifted.mostSignificant = 0;
        return shifted;
      }
    }

    /// <summary>Bit shifts the integer to the left</summary>
    /// <param name="bitOffset">Number of bits the integer will be shifted</param>
    /// <returns>The bit-shifted integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 &operator <<=(int bitOffset) {
      assert((bitOffset >= 0) && u8"UInt128 shift operator doesn't support negative shifts");

      if(bitOffset == 0) { // Can't use generic method because shift by 64 is undefined
        return *this;
      } else if(bitOffset < 64) {
        this->mostSignificant <<= bitOffset;
        this->mostSignificant |= (this->leastSignificant >> (64 - bitOffset));
        this->leastSignificant <<= bitOffset;
        return *this;
      } else {
        this->mostSignificant = (this->leastSignificant << (bitOffset - 64));
        this->leastSignificant = 0;
        return *this;
      }
    }

    /// <summary>Bit shifts the integer to the right</summary>
    /// <param name="bitOffset">Number of bits the integer will be shifted</param>
    /// <returns>The bit-shifted integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 &operator >>=(int bitOffset) {
      assert((bitOffset >= 0) && u8"UInt128 shift operator doesn't support negative shifts");

      if(bitOffset == 0) {
        return *this; // Can't use generic method because shift by 64 is undefined
      } else if(bitOffset < 64) {
        this->leastSignificant >>= bitOffset;
        this->leastSignificant |= (this->mostSignificant << (64 - bitOffset));
        this->mostSignificant >>= bitOffset;
        return *this;
      } else {
        this->leastSignificant = (this->mostSignificant >> (bitOffset - 64));
        this->mostSignificant = 0;
        return *this;
      }
    }

    /// <summary>Returns the integer ORed with another integer</summary>
    /// <param name="other">Other integer that will be ORed with this one</param>
    /// <returns>The ORed copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 operator |(const UInt128 &other) const {
      UInt128 combined;
      combined.mostSignificant = this->mostSignificant | other.mostSignificant;
      combined.leastSignificant = this->leastSignificant | other.leastSignificant;
      return combined;
    }

    /// <summary>Returns the integer ANDed with another integer</summary>
    /// <param name="other">Other integer that will be ANDed with this one</param>
    /// <returns>The ANDed copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 operator &(const UInt128 &other) const {
      UInt128 combined;
      combined.mostSignificant = this->mostSignificant & other.mostSignificant;
      combined.leastSignificant = this->leastSignificant & other.leastSignificant;
      return combined;
    }

    /// <summary>Returns the integer XORed with another integer</summary>
    /// <param name="other">Other integer that will be XORed with this one</param>
    /// <returns>The XORed copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 operator ^(const UInt128 &other) const {
      UInt128 combined;
      combined.mostSignificant = this->mostSignificant ^ other.mostSignificant;
      combined.leastSignificant = this->leastSignificant ^ other.leastSignificant;
      return combined;
    }

    /// <summary>Returns the integer ORed with another integer</summary>
    /// <param name="other">Other integer that will be ORed with this one</param>
    /// <returns>The ORed copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 &operator |=(const UInt128 &other) {
      this->mostSignificant |= other.mostSignificant;
      this->leastSignificant |= other.leastSignificant;
      return *this;
    }

    /// <summary>Returns the integer ANDed with another integer</summary>
    /// <param name="other">Other integer that will be ANDed with this one</param>
    /// <returns>The ANDed copy of the integer</summary>
    public: NUCLEX_PIXELS_API constexpr UInt128 &operator &=(const UInt128 &other) {
      this->mostSignificant &= other.mostSignificant;
      this->leastSignificant &= other.leastSignificant;
      return *this;
    }

#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    /// <summary>Stores the least signifcant 64 bits of the 128 bit integer</summary>
    private: std::uint64_t leastSignificant;
    /// <summary>Stores the most signifcant 64 bits of the 128 bit integer</summary>
    private: std::uint64_t mostSignificant;
#else
    /// <summary>Stores the most signifcant 64 bits of the 128 bit integer</summary>
    private: std::uint64_t mostSignificant;
    /// <summary>Stores the least signifcant 64 bits of the 128 bit integer</summary>
    private: std::uint64_t leastSignificant;
#endif
  };

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)

  #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wpedantic" // ISO C++ does not support ‘__int128’
  #elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic" // ISO C++ does not support ‘__int128’
  #endif

  /// <summary>Alias for the best 128 bit integer implementation to use</summary>
  typedef unsigned __int128 uint128_t;

  #if defined(__clang__)
    #pragma clang diagnostic pop
  #elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic pop
  #endif

#else

  /// <summary>Alias for the best 128 bit integer implementation to use</summary>
  typedef UInt128 uint128_t;

#endif

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

namespace std {

  // ------------------------------------------------------------------------------------------- //

  // CHECK: C++ std says I'm not allowed to specialize std::is_arithmetic ;..(
#if NUCLEX_SUPPORT_UINT128_I_HAVE_IMPLEMENTED_ARITHMETIC_OPS
  /// <summary>Declares that the 128 bit integer supports arithmetics</summary>
  template<>
  struct is_arithmetic<Nuclex::Pixels::UInt128> :
    public std::true_type {};
#endif

  /// <summary>Declares that the 128 bit integer is an integral type</summary>
  template<>
  struct is_integral<Nuclex::Pixels::UInt128> :
    public std::true_type {};

  /// <summary>Declares that the 128 bit integer is an unsigned type</summary>
  template<>
  struct is_unsigned<Nuclex::Pixels::UInt128> :
    public std::true_type {};

  /// <summary>Numeric limits for 128 bit unsigned integers</summary>
  /// <remarks>
  ///   Users are not allowed to add anything to the std namespace. However, an exception
  ///   has been defined for supporting custom, non-complex types in std::numeric_limits.
  /// </remarks>
  template<> class numeric_limits<Nuclex::Pixels::UInt128> {

    /// <summary>Type for which the class is providing informations</summary>
    public: typedef Nuclex::Pixels::UInt128 _Ty;

    /// <summary>
    ///   Minimum finite positive value that is representable by a 128 bit integer
    /// </summary>
    public: static _Ty min() throw() {
      return _Ty(0U);
    }

    /// <summary>
    ///   Maximum finite value that is representable by a 128 bit integer
    /// </summary>
    public: static _Ty max() throw() {
#if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)
      return (_Ty(18446744073709551615U) << 64) | _Ty(18446744073709551615U);
#else
      return _Ty(18446744073709551615U, 18446744073709551615U);
#endif
    }

    /// <summary>
    ///   Lowest finite value that is representable by the 128 bit integer
    /// </summary>
    public: static _Ty lowest() throw() {
      return _Ty(0U);
    }

    /// <summary>Smallest effective increment from the value 1.0</summary>
    public: static _Ty epsilon() throw() {
      return _Ty(1U);
    }

  };

  // ------------------------------------------------------------------------------------------- //

}

#endif // NUCLEX_PIXELS_UINT128_H
