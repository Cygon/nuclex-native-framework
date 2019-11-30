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
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R16_Unsigned_Native16));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R16_Float_Native16));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R32_Float_Native32));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R8_G8_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R16_G16_Unsigned_Native16));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R16_G16_Float_Native16));

    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R5_G6_B5_Unsigned));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R5_G6_B5_Unsigned_Native16));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::R5_G6_B5_Unsigned_Flipped16));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::B5_G6_R5_Unsigned));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::B5_G6_R5_Unsigned_Native16));
    EXPECT_EQ(16, CountBitsPerPixel(PixelFormat::B5_G6_R5_Unsigned_Flipped16));

    EXPECT_EQ(24, CountBitsPerPixel(PixelFormat::R8_G8_B8_Unsigned));
    EXPECT_EQ(24, CountBitsPerPixel(PixelFormat::R8_G8_B8_Signed));
    EXPECT_EQ(24, CountBitsPerPixel(PixelFormat::B8_G8_R8_Unsigned));
    EXPECT_EQ(24, CountBitsPerPixel(PixelFormat::B8_G8_R8_Signed));

    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::A8_B8_G8_R8_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Unsigned));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::A8_B8_G8_R8_Unsigned_Native32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::A8_B8_G8_R8_Unsigned_Flipped32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Unsigned_Native32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Unsigned_Flipped32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::A8_B8_G8_R8_Signed));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Signed));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::A8_B8_G8_R8_Signed_Native32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::A8_B8_G8_R8_Signed_Flipped32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Signed_Native32));
    EXPECT_EQ(32, CountBitsPerPixel(PixelFormat::R8_G8_B8_A8_Signed_Flipped32));

/*    
    EXPECT_EQ(4, CountBitsPerPixel(OldPixelFormat::BC1_Compressed));
    EXPECT_EQ(8, CountBitsPerPixel(OldPixelFormat::BC2_Compressed));
    EXPECT_EQ(8, CountBitsPerPixel(OldPixelFormat::BC3_Compressed));
*/
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatTest, BytesPerBlockCanBeCounted) {
    EXPECT_EQ(1, CountBytesPerBlock(PixelFormat::R8_Unsigned));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R16_Unsigned_Native16));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R16_Float_Native16));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R32_Float_Native32));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R8_G8_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R16_G16_Unsigned_Native16));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R16_G16_Float_Native16));

    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R5_G6_B5_Unsigned));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R5_G6_B5_Unsigned_Native16));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::R5_G6_B5_Unsigned_Flipped16));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::B5_G6_R5_Unsigned));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::B5_G6_R5_Unsigned_Native16));
    EXPECT_EQ(2, CountBytesPerBlock(PixelFormat::B5_G6_R5_Unsigned_Flipped16));

    EXPECT_EQ(3, CountBytesPerBlock(PixelFormat::R8_G8_B8_Unsigned));
    EXPECT_EQ(3, CountBytesPerBlock(PixelFormat::R8_G8_B8_Signed));
    EXPECT_EQ(3, CountBytesPerBlock(PixelFormat::B8_G8_R8_Unsigned));
    EXPECT_EQ(3, CountBytesPerBlock(PixelFormat::B8_G8_R8_Signed));

    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::A8_B8_G8_R8_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Unsigned));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::A8_B8_G8_R8_Unsigned_Native32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::A8_B8_G8_R8_Unsigned_Flipped32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Unsigned_Native32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Unsigned_Flipped32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::A8_B8_G8_R8_Signed));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Signed));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::A8_B8_G8_R8_Signed_Native32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::A8_B8_G8_R8_Signed_Flipped32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Signed_Native32));
    EXPECT_EQ(4, CountBytesPerBlock(PixelFormat::R8_G8_B8_A8_Signed_Flipped32));
  }

  // ------------------------------------------------------------------------------------------- //


}} // namespace Nuclex::Pixels
