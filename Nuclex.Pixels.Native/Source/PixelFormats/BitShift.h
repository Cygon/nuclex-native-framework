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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_BITSHIFT_H
#define NUCLEX_PIXELS_PIXELFORMATS_BITSHIFT_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/UInt128.h"

#include <cstdint> // for std::uint8_t, std::uint16_t, std::uint32_t and std::uint64_t

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bit-shifts the value by the specified number of bits</summary>
  /// <typeparam name="ShiftOffset">Number of bits the value will be shifted</typeparam>
  /// <param name="integer">Integer value that will be bit-shifted</param>
  /// <returns>The bit-shifted 8 bit integer</returns>
  /// <remarks>
  ///   This bit shift allows shifting all bits out of the integer as well as
  ///   negative shifts (negatve shifts to the left, positive to the right)
  /// </remarks>
  template<int ShiftOffset>
  NUCLEX_PIXELS_ALWAYS_INLINE constexpr uint8_t BitShift(uint8_t integer) {
    if constexpr((ShiftOffset <= -8) || (ShiftOffset >= 8)) {
      (void)integer;
      return 0;
    } else if constexpr(ShiftOffset < 0) {
      return integer << (-ShiftOffset);
    } else if constexpr(ShiftOffset > 0) {
      return integer >> ShiftOffset;
    } else {
      return integer;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bit-shifts the value by the specified number of bits</summary>
  /// <typeparam name="ShiftOffset">Number of bits the value will be shifted</typeparam>
  /// <param name="integer">Integer value that will be bit-shifted</param>
  /// <returns>The bit-shifted 16 bit integer</returns>
  /// <remarks>
  ///   This bit shift allows shifting all bits out of the integer as well as
  ///   negative shifts (negatve shifts to the left, positive to the right)
  /// </remarks>
  template<int ShiftOffset>
  NUCLEX_PIXELS_ALWAYS_INLINE constexpr uint16_t BitShift(uint16_t integer) {
#if defined(_MSC_VER) // MSVC invents warnings here for things that aren't really there
#pragma warning(push)
#pragma warning(disable:4554) // check operator precedence for possible error
#endif
    if constexpr((ShiftOffset <= -16) || (ShiftOffset >= 16)) {
      (void)integer;
      return 0;
    } else if constexpr(ShiftOffset < 0) {
      return integer << (-ShiftOffset);
    } else if constexpr(ShiftOffset > 0) {
      return integer >> ShiftOffset;
    } else {
      return integer;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bit-shifts the value by the specified number of bits</summary>
  /// <typeparam name="ShiftOffset">Number of bits the value will be shifted</typeparam>
  /// <param name="integer">Integer value that will be bit-shifted</param>
  /// <returns>The bit-shifted 32 bit integer</returns>
  /// <remarks>
  ///   This bit shift allows shifting all bits out of the integer as well as
  ///   negative shifts (negatve shifts to the left, positive to the right)
  /// </remarks>
  template<int ShiftOffset>
  NUCLEX_PIXELS_ALWAYS_INLINE constexpr uint32_t BitShift(uint32_t integer) {
#if defined(_MSC_VER) // MSVC invents warnings here for things that aren't really there
#pragma warning(push)
#pragma warning(disable:4554) // check operator precedence for possible error
#endif
    if constexpr((ShiftOffset <= -32) || (ShiftOffset >= 32)) {
      (void)integer;
      return 0;
    } else if constexpr(ShiftOffset < 0) {
      return integer << (-ShiftOffset);
    } else if constexpr(ShiftOffset > 0) {
      return integer >> ShiftOffset;
    } else {
      return integer;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bit-shifts the value by the specified number of bits</summary>
  /// <typeparam name="ShiftOffset">Number of bits the value will be shifted</typeparam>
  /// <param name="integer">Integer value that will be bit-shifted</param>
  /// <returns>The bit-shifted 64 bit integer</returns>
  /// <remarks>
  ///   This bit shift allows shifting all bits out of the integer as well as
  ///   negative shifts (negatve shifts to the left, positive to the right)
  /// </remarks>
  template<int ShiftOffset>
  NUCLEX_PIXELS_ALWAYS_INLINE constexpr uint64_t BitShift(uint64_t integer) {
#if defined(_MSC_VER) // MSVC invents warnings here for things that aren't really there
#pragma warning(push)
#pragma warning(disable:4554) // check operator precedence for possible error
#endif
    if constexpr((ShiftOffset <= -64) || (ShiftOffset >= 64)) {
      (void)integer;
      return 0;
    } else if constexpr(ShiftOffset < 0) {
      return integer << (-ShiftOffset);
    } else if constexpr(ShiftOffset > 0) {
      return integer >> ShiftOffset;
    } else {
      return integer;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Bit-shifts the value by the specified number of bits</summary>
  /// <typeparam name="ShiftOffset">Number of bits the value will be shifted</typeparam>
  /// <param name="integer">Integer value that will be bit-shifted</param>
  /// <returns>The bit-shifted 16 bit integer</returns>
  /// <remarks>
  ///   This bit shift allows shifting all bits out of the integer as well as
  ///   negative shifts (negatve shifts to the left, positive to the right)
  /// </remarks>
  template<int ShiftOffset>
  NUCLEX_PIXELS_ALWAYS_INLINE constexpr uint128_t BitShift(uint128_t integer) {
    if constexpr((ShiftOffset <= -128) || (ShiftOffset >= 128)) {
      (void)integer;
      return uint128_t(0U);
    } else if constexpr(ShiftOffset < 0) {
      return integer << (-ShiftOffset);
    } else if constexpr(ShiftOffset > 0) {
      return integer >> ShiftOffset;
    } else {
      return integer;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_BITSHIFT_H
