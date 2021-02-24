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

#ifndef NUCLEX_SUPPORT_BITTRICKS_H
#define NUCLEX_SUPPORT_BITTRICKS_H

#include "Nuclex/Support/Config.h"

#include <cstddef>
#include <cstdint>

// Microsoft compilers need a special header to know their intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#endif

// Whether the popcnt CPU instruction is supported by the targeted architecture
//
// This could be checked at runtime, but I don't want that complexity. You either
// compile this library for post-2012 CPUs or you target something earlier.
#if defined(_MSC_VER)
  // The Microsoft compiler only offers an AVX constant, so for a bunch of CPUs
  // between 2008 and 2012 popcnt is available but we have no way of knowing...
  #if defined(__AVX__)
    #define NUCLEX_SUPPORT_POPCNT_AVAILABLE 1
  #endif
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
  // GCC and clang split the target architecture into individual feature sets.
  // The popcnt instruction was introduced with SSE4A (AMD) and SSE 4.1 (Intel)
  #if defined(__SSE4A__) || defined(__SSE4_1__)
    #define NUCLEX_SUPPORT_POPCNT_AVAILABLE 1
  #endif
#endif

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A few helper methods for bit manipulation</summary>
  class BitTricks {

    /// <summary>Counts the number of bits set in a 32 bit integer</summary>
    /// <param name="value">Value whose bits will be counted</param>
    /// <returns>The number of bits set in the 32 bit integer</returns>
    public: NUCLEX_SUPPORT_API static unsigned char CountBits(std::uint32_t value) {
#if defined(_MSC_VER) && defined(NUCLEX_SUPPORT_POPCNT_AVAILABLE)
      return static_cast<unsigned char>(__popcnt(value));
#elif ( \
  (defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))) && \
  defined(NUCLEX_SUPPORT_POPCNT_AVAILABLE) \
)
      return static_cast<unsigned char>(__builtin_popcount(value));
#else
      // http://stackoverflow.com/questions/109023
      value = value - ((value >> 1) & 0x55555555U);
      value = (value & 0x33333333U) + ((value >> 2) & 0x33333333U);
      return (((value + (value >> 4)) & 0xF0F0F0FU) * 0x1010101U) >> 24;
#endif
    }

    /// <summary>Counts the number of bits set in a 64 bit integer</summary>
    /// <param name="value">Value whose bits will be counted</param>
    /// <returns>The number of bits set in the 64 bit integer</returns>
    public: NUCLEX_SUPPORT_API static unsigned char CountBits(std::uint64_t value) {
#if defined(_MSC_VER) && defined(NUCLEX_SUPPORT_POPCNT_AVAILABLE) && defined(_M_X64)
      return static_cast<unsigned char>(__popcnt64(value));
#elif ( \
  (defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))) && \
  defined(NUCLEX_SUPPORT_POPCNT_AVAILABLE) \
)
      return static_cast<unsigned char>(__builtin_popcountll(value));
#else
      // http://stackoverflow.com/questions/2709430
      value = value - ((value >> 1) & 0x5555555555555555ULL);
      value = (value & 0x3333333333333333ULL) + ((value >> 2) & 0x3333333333333333ULL);
      return (
        (((value + (value >> 4)) & 0xF0F0F0F0F0F0F0FULL) * 0x101010101010101ULL) >> 56
      );
#endif
    }

    /// <summary>Counts the number of leading zero bits in a value</summary>
    /// <param name="value">Value in which the leading zero bits will be couned</param>
    /// <returns>The number of leading zero bits in the value</returns>
    /// <remarks>
    ///   The result is undefined if the input value is 0
    /// </remarks>
    public: NUCLEX_SUPPORT_API static unsigned char CountLeadingZeroBits(std::uint32_t value) {
#if defined(_MSC_VER)
      //return static_cast<unsigned char>(__lzcnt(value));
      unsigned long bitIndex;
      _BitScanReverse(&bitIndex, value);
      return static_cast<unsigned char>(31 - bitIndex);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
      return static_cast<unsigned char>(__builtin_clz(value));
#else
      // https://www.chessprogramming.org/BitScan#Bitscan_reverse
      // https://stackoverflow.com/questions/2589096/
      static const unsigned char deBruijnBitPosition[32] = {
        31, 22, 30, 21, 18, 10, 29, 2, 20, 17, 15, 13, 9, 6, 28, 1,
        23, 19, 11, 3, 16, 14, 7, 24, 12, 4, 8, 25, 5, 26, 27, 0
      };

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;

      return deBruijnBitPosition[(value * 0x07C4ACDDU) >> 27];
#endif
    }

    /// <summary>Counts the number of leading zero bits in a value</summary>
    /// <param name="value">Value in which the leading zero bits will be couned</param>
    /// <returns>The number of leading zero bits in the value</returns>
    /// <remarks>
    ///   The result is undefined if the input value is 0
    /// </remarks>
    public: NUCLEX_SUPPORT_API static unsigned char CountLeadingZeroBits(std::uint64_t value) {
#if defined(_MSC_VER) && defined(_M_X64)
      //return static_cast<unsigned char>(__lzcnt64(value));
      unsigned long bitIndex;
      _BitScanReverse64(&bitIndex, value);
      return static_cast<unsigned char>(63 - bitIndex);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
      return static_cast<unsigned char>(__builtin_clzll(value));
#else
      // https://stackoverflow.com/questions/21888140/
      static const unsigned char deBruijnBitPosition[64] = {
        63, 16, 62, 7, 15, 36, 61, 3, 6, 14, 22, 26, 35, 47, 60, 2,
        9, 5, 28, 11, 13, 21, 42, 19, 25, 31, 34, 40, 46, 52, 59, 1,
        17, 8, 37, 4, 23, 27, 48, 10, 29, 12, 43, 20, 32, 41, 53, 18,
        38, 24, 49, 30, 44, 33, 54, 39, 50, 45, 55, 51, 56, 57, 58, 0
      };

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;
      value |= value >> 32;

      return deBruijnBitPosition[(value * 0x03F79D71B4CB0A89ULL) >> 58];
#endif
    }

    /// <summary>
    ///   Returns the nearest power of two that is greater than or equal to the input value
    /// </summary>
    /// <param name="value">Value in which the next power of two will be returned</param>
    /// <returns>
    ///   The nearest power of two that is greater than or equal to the input value
    /// </returns>
    public: NUCLEX_SUPPORT_API static std::uint32_t GetUpperPowerOfTwo(std::uint32_t value) {
#if defined(_MSC_VER)
      unsigned long bitIndex;
      _BitScanReverse(&bitIndex, value);
      std::uint32_t lowerBound = 1U << bitIndex;
      return lowerBound << static_cast<int>(value > lowerBound);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
      std::uint32_t lowerBound = 2147483648U >> __builtin_clz(value);
      return lowerBound << static_cast<int>(value > lowerBound);
#else
      --value;

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;

      return (value + 1);
#endif
    }

    /// <summary>
    ///   Returns the nearest power of two that is greater than or equal to the input value
    /// </summary>
    /// <param name="value">Value in which the next power of two will be returned</param>
    /// <returns>
    ///   The nearest power of two that is greater than or equal to the input value
    /// </returns>
    public: NUCLEX_SUPPORT_API static std::uint64_t GetUpperPowerOfTwo(std::uint64_t value) {
#if defined(_MSC_VER) && defined(_M_X64)
      unsigned long bitIndex;
      _BitScanReverse64(&bitIndex, value);
      std::uint64_t lowerBound = 1ULL << bitIndex;
      return lowerBound << static_cast<int>(value > lowerBound);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
      std::uint64_t lowerBound = 9223372036854775808ULL >> __builtin_clzll(value);
      return lowerBound << static_cast<int>(value > lowerBound);
#else
      --value;

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;
      value |= value >> 32;

      return (value + 1);
#endif
    }

    /// <summary>Calculates the log base-2 of a 32 bit integer</summary>
    /// <param name="value">Value of which the log base-2 will be calculated</param>
    /// <returns>The log base-2 of the specified value</returns>
    /// <remarks>
    ///   The result is undefined if the input value is 0
    /// </remarks>
    public: NUCLEX_SUPPORT_API static unsigned char GetLogBase2(std::uint32_t value) {
#if defined(_MSC_VER)
      unsigned long bitIndex;
      _BitScanReverse(&bitIndex, value);
      return static_cast<unsigned char>(bitIndex);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
      return static_cast<unsigned char>(31 - __builtin_clz(value));
#else
      // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
      static const unsigned char deBruijnBitPosition[32] = {
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
      };

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;

      return deBruijnBitPosition[(value * 0x07C4ACDDU) >> 27];
#endif
    }

    /// <summary>Calculates the log base-2 of a 64 bit integer</summary>
    /// <param name="value">Value of which the log base-2 will be calculated</param>
    /// <returns>The log base-2 of the specified value</returns>
    /// <remarks>
    ///   The result is undefined if the input value is 0
    /// </remarks>
    public: NUCLEX_SUPPORT_API static unsigned char GetLogBase2(std::uint64_t value) {
#if defined(_MSC_VER) && defined(_M_X64)
      //return static_cast<unsigned char>(__lzcnt64(value));
      unsigned long bitIndex;
      _BitScanReverse64(&bitIndex, value);
      return static_cast<unsigned char>(bitIndex);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
      return static_cast<unsigned char>(63 - __builtin_clzll(value));
#else
      // https://stackoverflow.com/questions/21888140/
      static const unsigned char deBruijnBitPosition[64] = {
        0, 47, 1, 56, 48, 27, 2, 60, 57, 49, 41, 37, 28, 16, 3, 61,
        54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11, 4, 62,
        46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
        25, 39, 14, 33, 19, 30, 9, 24, 13, 18, 8, 12, 7, 6, 5, 63
      };

      value |= value >> 1;
      value |= value >> 2;
      value |= value >> 4;
      value |= value >> 8;
      value |= value >> 16;
      value |= value >> 32;

      return deBruijnBitPosition[(value * 0x03F79D71B4CB0A89ULL) >> 58];
#endif
    }

    /// <summary>Calculates the log base-10 of a 32 bit integer</summary>
    /// <param name="value">Value of which the log base-10 will be calculated</param>
    /// <returns>The log base-10 of the specified value</returns>
    /// <remarks>
    ///   The result is undefined if the input value is 0
    /// </remarks>
    public: NUCLEX_SUPPORT_API static unsigned char GetLogBase10(std::uint32_t value) {
      static const std::uint32_t powersOfTen[10] = {
        1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U,
      };

      // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
      std::uint32_t temp = (GetLogBase2(value) + 1) * 1233U >> 12;
      return static_cast<unsigned char>(temp - (value < powersOfTen[temp]));
    }

    /// <summary>Calculates the log base-10 of a 64 bit integer</summary>
    /// <param name="value">Value of which the log base-10 will be calculated</param>
    /// <returns>The log base-10 of the specified value</returns>
    /// <remarks>
    ///   The result is undefined if the input value is 0
    /// </remarks>
    public: NUCLEX_SUPPORT_API static unsigned char GetLogBase10(std::uint64_t value) {
      static const std::uint64_t powersOfTen[20] = {
        1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL,
        100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
        10000000000000ULL, 100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL,
        100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL
      };

      // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
      std::uint64_t temp = (GetLogBase2(value) + 1) * 1233U >> 12;
      return static_cast<unsigned char>(temp - (value < powersOfTen[temp]));
    }

    /// <summary>Very fast random number generation from a seed value</summary>
    /// <param name="seed">Seed from which a random number will be generated</param>
    /// <returns>The next random number after the seed value</returns>
    /// <remarks>
    ///   This is a blazingly fast method of generating random numbers, but the entropy
    ///   is not very high. It's useful if one need to generate kilobytes or megabytes of
    ///   semi-random data. Don't even think about using this with cryptographic algorithms!
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr std::uint32_t XorShiftRandom(std::uint32_t seed) {
      seed ^= (seed << 13);
	seed ^= (seed >> 17);
	seed ^= (seed << 5);
	return seed;
    }

    /// <summary>Very fast random number generation from a seed value</summary>
    /// <param name="seed">Seed from which a random number will be generated</param>
    /// <returns>The next random number after the seed value</returns>
    /// <remarks>
    ///   This is a blazingly fast method of generating random numbers, but the entropy
    ///   is not very high. It's useful if one need to generate kilobytes or megabytes of
    ///   semi-random data. Don't even think about using this with cryptographic algorithms!
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr std::uint64_t XorShiftRandom(std::uint64_t seed) {
      seed ^= (seed << 13);
	seed ^= (seed >> 7);
	seed ^= (seed << 17);
      return seed;
    }

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_BITTRICKS_H
