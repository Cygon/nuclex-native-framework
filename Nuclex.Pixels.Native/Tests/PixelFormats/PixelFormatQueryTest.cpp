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

#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatQueryTest, DetectsIfPixelFormatHasRedChannel) {
    EXPECT_TRUE(PixelFormatQuery::HasRedChannel(PixelFormat::R8_Unsigned));
    EXPECT_FALSE(PixelFormatQuery::HasRedChannel(PixelFormat::A8_Unsigned));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatQueryTest, DetectsIfPixelFormatHasGreenChannel) {
    EXPECT_FALSE(PixelFormatQuery::HasGreenChannel(PixelFormat::R8_Unsigned));
    EXPECT_FALSE(PixelFormatQuery::HasGreenChannel(PixelFormat::A8_Unsigned));
    EXPECT_TRUE(PixelFormatQuery::HasGreenChannel(PixelFormat::R8_G8_Unsigned));
    EXPECT_TRUE(PixelFormatQuery::HasGreenChannel(PixelFormat::R8_G8_B8_Unsigned));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatQueryTest, DetectsIfPixelFormatHasBlueChannel) {
    EXPECT_FALSE(PixelFormatQuery::HasBlueChannel(PixelFormat::A8_Unsigned));
    EXPECT_FALSE(PixelFormatQuery::HasBlueChannel(PixelFormat::R8_G8_Unsigned));
    EXPECT_TRUE(PixelFormatQuery::HasBlueChannel(PixelFormat::R8_G8_B8_Unsigned));
    EXPECT_TRUE(PixelFormatQuery::HasBlueChannel(PixelFormat::R8_G8_B8_A8_Unsigned));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats