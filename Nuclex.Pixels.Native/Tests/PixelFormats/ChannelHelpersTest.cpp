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

#include "../../Source/PixelFormats/ChannelHelpers.h"
#include <gtest/gtest.h>

#include <cstdint> // for std::uint16_t

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //
#if 0
  TEST(ChannelAdjustTest, IntegerWideningAndNarrowingWorks) {

    // The channel adjuster should be able to widen an 8 bit channel to a 16 bit channel.
    // This is a good check to see if the intermediate pixel type is the wider of the two
    // because otherwise the shift result would move the bits out of the type's range.
    std::uint16_t widened = ChannelAdjuster<std::uint8_t, std::uint16_t>::Adjust<8, 16>(
      0xFB
    );
    EXPECT_EQ(widened, 0xFBFB);

    // The channel adjuster should be able to narrow a 16 bit channel to an 8 bit channel.
    // In this direction, the upper bits need to be used, so this, too, verifies that the
    // wider of the two conversion types is used for the intermediate result.
    std::uint8_t narrowed = ChannelAdjuster<std::uint16_t, std::uint8_t>::Adjust<16, 8>(
      0xBFFF
    );
    EXPECT_EQ(narrowed, 0xBF);

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelAdjustTest, FloatWideningAndNarrowingWorks) {

    // Float to double widening. There is no bit manipulation neccessary.
    double widenedFloat = ChannelAdjuster<float, double>::Adjust<32, 64>(0.875f);
    EXPECT_EQ(widenedFloat, 0.875);

    // Double to float narrowing. There is no bit manipulation neccessary.
    float narrowedFloat = ChannelAdjuster<double, float>::Adjust<64, 32>(0.125);
    EXPECT_EQ(narrowedFloat, 0.125f);

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelAdjustTest, FloatToIntegerConversionWorks) {

    // Trivial test, zero should stay zero. If fancy rounding math is employed,
    // this verifies that it's still possible for a color channel to be black.
    std::uint16_t zeroInteger = ChannelAdjuster<float, std::uint16_t>::Adjust<32, 16>(0.0f);
    EXPECT_EQ(zeroInteger, 0);

    // Rounding test. 0.5 expands to exactly halfway between 32767 and 32768,
    // so for this one result, both outcomes are valid
    std::uint16_t halfInteger = ChannelAdjuster<float, std::uint16_t>::Adjust<32, 16>(0.5f);
    if(halfInteger == 32768) {
      EXPECT_EQ(halfInteger, 32768);
    } else {
      EXPECT_EQ(halfInteger, 32767);
    }

    // Finally, a floating point value of 1.0 should convert to the highest
    // representable value of the integer. This ensures that a color can be full white.
    std::uint16_t maxInteger = ChannelAdjuster<float, std::uint16_t>::Adjust<32, 16>(1.0f);
    EXPECT_EQ(maxInteger, 65535);

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ChannelAdjustTest, IntegerToFloatConversionWorks) {

    // When converting from integer to float it's important to divide the integer by its
    // maximum value, not the power-of-two (i.e. divide a byte by 255, not 256)
    float zeroFloat = ChannelAdjuster<std::uint16_t, float>::Adjust<16, 32>(0);
    EXPECT_EQ(zeroFloat, 0.0f);

    // To achieve an intensity of exactly 0.5, the color channel would have to hold
    // the value 32767.5, but as an integer, that's not possible. We verify that the result
    // still is calculated exactly
    float halfFloat = ChannelAdjuster<std::uint16_t, float>::Adjust<16, 32>(32767);
    EXPECT_EQ(halfFloat, static_cast<float>(32767.0f / 65535.0f));

    // Finally, full white should convert to exactly 1.0 and not one ULP less.
    float maxFloat = ChannelAdjuster<std::uint16_t, float>::Adjust<16, 32>(65535);
    EXPECT_EQ(maxFloat, 1.0f);

  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
