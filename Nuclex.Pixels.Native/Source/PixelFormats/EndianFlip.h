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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_ENDIANFLIP_H
#define NUCLEX_PIXELS_PIXELFORMATS_ENDIANFLIP_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/UInt128.h" // for uint128_t

// Microsoft compilers need a special header to know their intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Does nothing</summary>
  /// <param name="integer">Integer that will be returned unmodified</param>
  /// <returns>The input value, unmodified</returns>
  NUCLEX_PIXELS_ALWAYS_INLINE uint8_t EndianFlip(uint8_t integer) {
    return integer;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reverses the bytes of a 16 bit integer</summary>
  /// <param name="integer">Integer whose bytes will be reversed</param>
  /// <returns>The endian-flipped 16 bit integer</returns>
  NUCLEX_PIXELS_ALWAYS_INLINE uint16_t EndianFlip(uint16_t integer) {
#if defined(_MSC_VER)
    return _byteswap_ushort(integer);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    return __builtin_bswap16(integer);
#else
    return (integer << 8) | (integer >> 8);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reverses the bytes of a 32 bit integer</summary>
  /// <param name="integer">Integer whose bytes will be reversed</param>
  /// <returns>The endian-flipped 32 bit integer</returns>
  NUCLEX_PIXELS_ALWAYS_INLINE uint32_t EndianFlip(std::uint32_t integer) {
#if defined(_MSC_VER)
    return _byteswap_ulong(integer);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    return __builtin_bswap32(integer);
#else
    return (
      (integer << 24) |
      ((integer & 0x0000FF00) << 8) |
      ((integer & 0x00FF0000) >> 8) |
      (integer >> 24)
    );
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reverses the bytes of a 64 bit integer</summary>
  /// <param name="integer">Integer whose bytes will be reversed</param>
  /// <returns>The endian-flipped 64 bit integer</returns>
  NUCLEX_PIXELS_ALWAYS_INLINE uint64_t EndianFlip(std::uint64_t integer) {
#if defined(_MSC_VER)
    return _byteswap_uint64(integer);
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    return __builtin_bswap64(integer);
#else
    return (
      (integer << 56) |
      ((integer & 0x000000000000FF00ULL) << 40) |
      ((integer & 0x0000000000FF0000ULL) << 24) |
      ((integer & 0x00000000FF000000ULL) << 8) |
      ((integer & 0x000000FF00000000ULL) >> 8) |
      ((integer & 0x0000FF0000000000ULL) >> 24) |
      ((integer & 0x00FF000000000000ULL) >> 40) |
      (integer >> 56)
    );
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reverses the bytes of a 128 bit integer</summary>
  /// <param name="integer">Integer whose bytes will be reversed</param>
  /// <returns>The endian-flipped 128 bit integer</returns>
  NUCLEX_PIXELS_ALWAYS_INLINE uint128_t EndianFlip(uint128_t integer) {
#if defined(_MSC_VER)
/* Doesn't optimize any better in VS2019
    const std::uint64_t *halves = reinterpret_cast<const std::uint64_t *>(&integer);
    uint128_t result;
    {
      std::uint64_t *resultHalves = reinterpret_cast<std::uint64_t *>(&result);
      resultHalves[0] = _byteswap_uint64(halves[1]);
      resultHalves[1] = _byteswap_uint64(halves[0]);
    }
    return result;
*/
    // CHECK: Does this violate aliasing rules? Do tested compilers handle it correctly?
    //   If so, the cast could be replaced by bit shifts which might optimize poorly:
    //   (uint128_t(__builtin_bswap64(static_cast<std::uint64_t>(integer))) << 64) |
    //   (uint128_t(__builtin_bswap64(static_cast<std::uint64_t>(integer >> 64)))) |
    const std::uint64_t *halves = reinterpret_cast<const std::uint64_t *>(&integer);
    return (
      (uint128_t(_byteswap_uint64(halves[0])) << 64) |
      (uint128_t(_byteswap_uint64(halves[1])))
    );
#elif defined(__clang__) || (defined(__GNUC__) || defined(__GNUG__))
    // CHECK: Does this violate aliasing rules? Do tested compilers handle it correctly?
    //   If so, the cast could be replaced by bit shifts which might optimize poorly:
    //   (uint128_t(__builtin_bswap64(static_cast<std::uint64_t>(integer))) << 64) |
    //   (uint128_t(__builtin_bswap64(static_cast<std::uint64_t>(integer >> 64)))) |
    const std::uint64_t *halves = reinterpret_cast<const std::uint64_t *>(&integer);
  #if defined(NUCLEX_PIXELS_HAVE_BUILTIN_INT128)
    return (
      (uint128_t(__builtin_bswap64(halves[0])) << 64) |
      (uint128_t(__builtin_bswap64(halves[1])))
    );
  #else
    return UInt128(
    #if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
      __builtin_bswap64(halves[0]),
      __builtin_bswap64(halves[1])
    #else
      __builtin_bswap64(halves[1]),
      __builtin_bswap64(halves[0])
    #endif
    );
  #endif

#else
    const constexpr uint128_t mask1 = uint128_t(0x00000000000000FFULL);
    const constexpr uint128_t mask2 = uint128_t(0x000000000000FF00ULL);
    const constexpr uint128_t mask3 = uint128_t(0x0000000000FF0000ULL);
    const constexpr uint128_t mask4 = uint128_t(0x00000000FF000000ULL);
    const constexpr uint128_t mask5 = uint128_t(0x000000FF00000000ULL);
    const constexpr uint128_t mask6 = uint128_t(0x0000FF0000000000ULL);
    const constexpr uint128_t mask7 = uint128_t(0x00FF000000000000ULL);
    const constexpr uint128_t mask8 = uint128_t(0xFF00000000000000ULL);
    const constexpr uint128_t mask9 = uint128_t(mask1 << 64);
    const constexpr uint128_t mask10 = uint128_t(mask2 << 64);
    const constexpr uint128_t mask11 = uint128_t(mask3 << 64);
    const constexpr uint128_t mask12 = uint128_t(mask4 << 64);
    const constexpr uint128_t mask13 = uint128_t(mask5 << 64);
    const constexpr uint128_t mask14 = uint128_t(mask6 << 64);
    const constexpr uint128_t mask15 = uint128_t(mask7 << 64);
    return (
      (integer << 120) |
      ((integer & mask2) << 104) |
      ((integer & mask3) << 88) |
      ((integer & mask4) << 72) |
      ((integer & mask5) << 56) |
      ((integer & mask6) << 40) |
      ((integer & mask7) << 24) |
      ((integer & mask8) << 8) |
      ((integer & mask9) >> 8) |
      ((integer & mask10) >> 24) |
      ((integer & mask11) >> 40) |
      ((integer & mask12) >> 56) |
      ((integer & mask13) >> 72) |
      ((integer & mask14) >> 88) |
      ((integer & mask15) >> 104) |
      (integer >> 120)
    );
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_ENDIANFLIP_H
