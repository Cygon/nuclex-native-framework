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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "Nuclex/Pixels/PixelFormat.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatTest, BitsPerPixelCanBeCounted) {
    EXPECT_EQ(8, CountBitsPerPixel(PixelFormat::R8_Unsigned));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R8_G8_Signed));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R5_G6_B5_Unsigned));
    EXPECT_EQ(24, CountBitsPerPixel(PixelFormat::R8_G8_B8_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Signed));
    EXPECT_EQ(4, CountBitsPerPixel(PixelFormat::BC1_Compressed));
    EXPECT_EQ(8, CountBitsPerPixel(PixelFormat::BC2_Compressed));
    EXPECT_EQ(8, CountBitsPerPixel(PixelFormat::BC3_Compressed));
    EXPECT_EQ(8, CountBitsPerPixel(PixelFormat::Modern_A8_Unsigned));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::Modern_R16_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::Modern_R16_G16_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::Modern_R16_G16_Float));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::Modern_R32_Float));
    EXPECT_EQ(64, CountBitsPerPixel(PixelFormat::Modern_R16_G16_B16_A16_Unsigned));
    EXPECT_EQ(64, CountBitsPerPixel(PixelFormat::Modern_R16_G16_B16_A16_Float));
    EXPECT_EQ(128, CountBitsPerPixel(PixelFormat::Offline_R32_G32_B32_A32_Float));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatTest, BitsPerBlockCanBeCounted) {
    EXPECT_EQ(1, CountBytesPerBlock(PixelFormat::R8_Unsigned));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R8_G8_Signed));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R5_G6_B5_Unsigned));
    EXPECT_EQ(3, CountBytesPerBlock(PixelFormat::R8_G8_B8_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Signed));
    EXPECT_EQ(8, CountBytesPerBlock(PixelFormat::BC1_Compressed));
    EXPECT_EQ(16, CountBytesPerBlock(PixelFormat::BC2_Compressed));
    EXPECT_EQ(16, CountBytesPerBlock(PixelFormat::BC3_Compressed));
    EXPECT_EQ(1, CountBytesPerBlock(PixelFormat::Modern_A8_Unsigned));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::Modern_R16_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::Modern_R16_G16_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::Modern_R16_G16_Float));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::Modern_R32_Float));
    EXPECT_EQ(8, CountBytesPerBlock(PixelFormat::Modern_R16_G16_B16_A16_Unsigned));
    EXPECT_EQ(8, CountBytesPerBlock(PixelFormat::Modern_R16_G16_B16_A16_Float));
    EXPECT_EQ(16, CountBytesPerBlock(PixelFormat::Offline_R32_G32_B32_A32_Float));
  }

  // ------------------------------------------------------------------------------------------- //


}} // namespace Nuclex::Pixels
