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

#include "Nuclex/Pixels/Bitmap.h"
#include "Nuclex/Pixels/ColorModels/RgbPixelIterator.h"
#include <gtest/gtest.h>

#include <random>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Memory buffer containing a 5x5 bitmap in R8-G8-B8-A8 pixel format</summary>
  std::uint8_t ExampleBitmapPixels[] = {
    0x00, 0x00, 0x00, 0xff, /*|*/ 0xff, 0x00, 0x00, 0xff, /*|*/ 0x00, 0xff, 0x00, 0xff,
    0x00, 0x00, 0xff, 0xff, /*|*/ 0xff, 0xff, 0xff, 0xff,

    0x33, 0x33, 0x33, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,
    0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,

    0x66, 0x66, 0x66, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,
    0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,

    0x99, 0x99, 0x99, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,
    0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,

    0xcc, 0xcc, 0xcc, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff,
    0x00, 0x00, 0x00, 0xff, /*|*/ 0x00, 0x00, 0x00, 0xff
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  TEST(RgbPixelIteratorTest, ChannelOrderCorrectForR8G8B8A8) {
    Bitmap testBitmap = Bitmap::InExistingMemory(
      BitmapMemory { 5, 5, 20, PixelFormat::R8_G8_B8_A8_Unsigned, ExampleBitmapPixels }
    );
    RgbPixelIterator iterator(testBitmap.Access());

    RgbColor color = *iterator;
    EXPECT_EQ(color.Red, 0.0f);
    EXPECT_EQ(color.Green, 0.0f);
    EXPECT_EQ(color.Blue, 0.0f);
    EXPECT_EQ(color.Alpha, 1.0f);
    ++iterator;

    color = *iterator;
    EXPECT_EQ(color.Red, 1.0f);
    EXPECT_EQ(color.Green, 0.0f);
    EXPECT_EQ(color.Blue, 0.0f);
    EXPECT_EQ(color.Alpha, 1.0f);
    ++iterator;

    color = *iterator;
    EXPECT_EQ(color.Red, 0.0f);
    EXPECT_EQ(color.Green, 1.0f);
    EXPECT_EQ(color.Blue, 0.0f);
    EXPECT_EQ(color.Alpha, 1.0f);
    ++iterator;

    color = *iterator;
    EXPECT_EQ(color.Red, 0.0f);
    EXPECT_EQ(color.Green, 0.0f);
    EXPECT_EQ(color.Blue, 1.0f);
    EXPECT_EQ(color.Alpha, 1.0f);
    ++iterator;

    color = *iterator;
    EXPECT_EQ(color.Red, 1.0f);
    EXPECT_EQ(color.Green, 1.0f);
    EXPECT_EQ(color.Blue, 1.0f);
    EXPECT_EQ(color.Alpha, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RgbPixelIteratorTest, ChannelValuesCorrectForR8G8B8A8) {
    Bitmap testBitmap = Bitmap::InExistingMemory(
      BitmapMemory { 5, 5, 20, PixelFormat::R8_G8_B8_A8_Unsigned, ExampleBitmapPixels }
    );
    RgbPixelIterator iterator(testBitmap.Access());

    RgbColor color = *iterator;
    EXPECT_EQ(color.Red, 0.0f);
    EXPECT_EQ(color.Green, 0.0f);
    EXPECT_EQ(color.Blue, 0.0f);
    EXPECT_EQ(color.Alpha, 1.0f);
    iterator += Lines(1);

    color = *iterator;
    EXPECT_EQ(color.Red, 0.2f);
    EXPECT_EQ(color.Green, 0.2f);
    EXPECT_EQ(color.Blue, 0.2f);
    EXPECT_EQ(color.Alpha, 1.0f);
    iterator += Lines(1);

    color = *iterator;
    EXPECT_EQ(color.Red, 0.4f);
    EXPECT_EQ(color.Green, 0.4f);
    EXPECT_EQ(color.Blue, 0.4f);
    EXPECT_EQ(color.Alpha, 1.0f);
    iterator += Lines(1);

    color = *iterator;
    EXPECT_EQ(color.Red, 0.6f);
    EXPECT_EQ(color.Green, 0.6f);
    EXPECT_EQ(color.Blue, 0.6f);
    EXPECT_EQ(color.Alpha, 1.0f);
    iterator += Lines(1);

    color = *iterator;
    EXPECT_EQ(color.Red, 0.8f);
    EXPECT_EQ(color.Green, 0.8f);
    EXPECT_EQ(color.Blue, 0.8f);
    EXPECT_EQ(color.Alpha, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels
