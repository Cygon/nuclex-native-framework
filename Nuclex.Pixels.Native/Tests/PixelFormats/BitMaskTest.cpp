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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "../../Source/PixelFormats/BitMask.h"
#include "Nuclex/Pixels/UInt128.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the upper 64 bits of a 128 bit integer</summary>
  /// <param name="integer">128 bit integer of which the upper 64 bits are returned</param>
  /// <returns>The upper 64 bits of the specified 128 bit integer</returns>
  std::uint64_t upper64(Nuclex::Pixels::uint128_t integer) {
    return static_cast<std::uint64_t>(integer >> 64);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Returns the lower 64 bits of a 128 bit integer</summary>
  /// <param name="integer">128 bit integer of which the lower 64 bits are returned</param>
  /// <returns>The lower 64 bits of the specified 128 bit integer</returns>
  std::uint64_t lower64(Nuclex::Pixels::uint128_t integer) {
    return static_cast<std::uint64_t>(integer);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitMaskTest, BitMaskCanBeSingleBit) {
    EXPECT_EQ((BitMask<std::size_t, 0, 1>), 1U);
    EXPECT_EQ((BitMask<std::size_t, 1, 1>), 2U);
    EXPECT_EQ((BitMask<std::size_t, 2, 1>), 4U);
    EXPECT_EQ((BitMask<std::size_t, 3, 1>), 8U);
    EXPECT_EQ((BitMask<std::size_t, 4, 1>), 16U);
    EXPECT_EQ((BitMask<std::size_t, 5, 1>), 32U);
    EXPECT_EQ((BitMask<std::size_t, 6, 1>), 64U);
    EXPECT_EQ((BitMask<std::size_t, 7, 1>), 128U);
    EXPECT_EQ((BitMask<std::size_t, 8, 1>), 256U);
    EXPECT_EQ((BitMask<std::size_t, 9, 1>), 512U);
    EXPECT_EQ((BitMask<std::size_t, 10, 1>), 1024U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitMaskTest, BitMaskCanBeAllBits) {
    std::size_t maximumSizeT = std::numeric_limits<std::size_t>::max();
    EXPECT_EQ((BitMask<std::size_t, 0, sizeof(std::size_t) * 8>), maximumSizeT);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitMaskTest, BitMaskCanBeNoBits) {
    EXPECT_EQ((BitMask<std::size_t, 0, 0>), 0U);
    EXPECT_EQ((BitMask<std::size_t, sizeof(std::size_t) * 4, 0>), 0U);
    EXPECT_EQ((BitMask<std::size_t, sizeof(std::size_t) * 8, 0>), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitMaskTest, EachBitCanBeSet) {
    EXPECT_EQ((BitMask<std::uint8_t, 0, 1>), (1U << 0));
    EXPECT_EQ((BitMask<std::uint8_t, 1, 1>), (1U << 1));
    EXPECT_EQ((BitMask<std::uint8_t, 2, 1>), (1U << 2));
    EXPECT_EQ((BitMask<std::uint8_t, 3, 1>), (1U << 3));
    EXPECT_EQ((BitMask<std::uint8_t, 4, 1>), (1U << 4));
    EXPECT_EQ((BitMask<std::uint8_t, 5, 1>), (1U << 5));
    EXPECT_EQ((BitMask<std::uint8_t, 6, 1>), (1U << 6));
    EXPECT_EQ((BitMask<std::uint8_t, 7, 1>), (1U << 7));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitMaskTest, RealWorldBitMasksAreCorrect) {
    EXPECT_EQ((BitMask<std::uint16_t, 0, 5>), 0x001FU);
    EXPECT_EQ((BitMask<std::uint16_t, 5, 6>), 0x07E0U);
    EXPECT_EQ((BitMask<std::uint16_t, 11, 5>), 0xF800U);

    EXPECT_EQ((BitMask<std::uint32_t, 0, 8>), 0x000000FFU);
    EXPECT_EQ((BitMask<std::uint32_t, 8, 8>), 0x0000FF00U);
    EXPECT_EQ((BitMask<std::uint32_t, 16, 8>), 0x00FF0000U);
    EXPECT_EQ((BitMask<std::uint32_t, 24, 8>), 0xFF000000U);

    EXPECT_EQ((BitMask<std::uint32_t, 0, 10>), 0x000003FFU);
    EXPECT_EQ((BitMask<std::uint32_t, 10, 10>), 0x000FFC00U);
    EXPECT_EQ((BitMask<std::uint32_t, 20, 10>), 0x3FF00000U);
    EXPECT_EQ((BitMask<std::uint32_t, 30, 2>), 0xC0000000U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitMaskTest, WorksWith128BitInteger) {
    uint128_t middleBits = BitMask<uint128_t, 32, 64>;
    EXPECT_EQ(upper64(middleBits), 0x00000000FFFFFFFFU);
    EXPECT_EQ(lower64(middleBits), 0xFFFFFFFF00000000U);

    uint128_t lowerBits = BitMask<uint128_t, 0, 32>;
    EXPECT_EQ(upper64(lowerBits), 0x0000000000000000U);
    EXPECT_EQ(lower64(lowerBits), 0x00000000FFFFFFFFU);

    uint128_t upperBits = BitMask<uint128_t, 96, 32>;
    EXPECT_EQ(upper64(upperBits), 0xFFFFFFFF00000000U);
    EXPECT_EQ(lower64(upperBits), 0x0000000000000000U);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
