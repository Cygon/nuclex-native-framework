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

#include "../../Source/PixelFormats/EndianFlip.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianFlipTest, SingleBytesAreNotModified) {
    for(std::size_t index = 0; index < 8; ++index) {
      std::uint8_t byte = (std::uint8_t(1U) << index);
      std::uint8_t flippedByte = EndianFlip(byte);
      
      EXPECT_EQ(byte, flippedByte);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianFlipTest, Int16IsFlippedCorrectly) {
    for(std::size_t index = 0; index < 16; ++index) {
      std::uint16_t word = (std::uint16_t(1U) << index);
      std::uint16_t flippedWord = EndianFlip(word);

      std::uint16_t expected;
      {
        const std::uint8_t *wordBytes = reinterpret_cast<const std::uint8_t *>(&word);
        std::uint8_t *expectedBytes = reinterpret_cast<std::uint8_t *>(&expected);

        expectedBytes[0] = wordBytes[1];
        expectedBytes[1] = wordBytes[0];
      }
      
      EXPECT_EQ(flippedWord, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianFlipTest, Int32IsFlippedCorrectly) {
    for(std::size_t index = 0; index < 32; ++index) {
      std::uint32_t integer = (std::uint32_t(1U) << index);
      std::uint32_t flippedinteger = EndianFlip(integer);

      std::uint32_t expected;
      {
        const std::uint8_t *integerBytes = reinterpret_cast<const std::uint8_t *>(&integer);
        std::uint8_t *expectedBytes = reinterpret_cast<std::uint8_t *>(&expected);

        expectedBytes[0] = integerBytes[3];
        expectedBytes[1] = integerBytes[2];
        expectedBytes[2] = integerBytes[1];
        expectedBytes[3] = integerBytes[0];
      }
      
      EXPECT_EQ(flippedinteger, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianFlipTest, Int64IsFlippedCorrectly) {
    for(std::size_t index = 0; index < 64; ++index) {
      std::uint64_t integer = (std::uint64_t(1U) << index);
      std::uint64_t flippedinteger = EndianFlip(integer);

      std::uint64_t expected;
      {
        const std::uint8_t *integerBytes = reinterpret_cast<const std::uint8_t *>(&integer);
        std::uint8_t *expectedBytes = reinterpret_cast<std::uint8_t *>(&expected);

        expectedBytes[0] = integerBytes[7];
        expectedBytes[1] = integerBytes[6];
        expectedBytes[2] = integerBytes[5];
        expectedBytes[3] = integerBytes[4];
        expectedBytes[4] = integerBytes[3];
        expectedBytes[5] = integerBytes[2];
        expectedBytes[6] = integerBytes[1];
        expectedBytes[7] = integerBytes[0];
      }
      
      EXPECT_EQ(flippedinteger, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(EndianFlipTest, Int128IsFlippedCorrectly) {
    for(std::size_t index = 0; index < 128; ++index) {
      uint128_t integer = (uint128_t(1U) << static_cast<int>(index));
      uint128_t flippedinteger = EndianFlip(integer);

      uint128_t expected;
      {
        const std::uint8_t *integerBytes = reinterpret_cast<const std::uint8_t *>(&integer);
        std::uint8_t *expectedBytes = reinterpret_cast<std::uint8_t *>(&expected);

        expectedBytes[0] = integerBytes[15];
        expectedBytes[1] = integerBytes[14];
        expectedBytes[2] = integerBytes[13];
        expectedBytes[3] = integerBytes[12];
        expectedBytes[4] = integerBytes[11];
        expectedBytes[5] = integerBytes[10];
        expectedBytes[6] = integerBytes[9];
        expectedBytes[7] = integerBytes[8];
        expectedBytes[8] = integerBytes[7];
        expectedBytes[9] = integerBytes[6];
        expectedBytes[10] = integerBytes[5];
        expectedBytes[11] = integerBytes[4];
        expectedBytes[12] = integerBytes[3];
        expectedBytes[13] = integerBytes[2];
        expectedBytes[14] = integerBytes[1];
        expectedBytes[15] = integerBytes[0];
      }
      
      EXPECT_EQ(flippedinteger, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
