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

#include "Nuclex/Pixels/Bitmap.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, NewBitmapDefaultsToARGB) {
    Bitmap newBitmap(32, 24);
    EXPECT_EQ(32, newBitmap.GetWidth());
    EXPECT_EQ(24, newBitmap.GetHeight());
    EXPECT_EQ(PixelFormat::R8_G8_B8_A8_Unsigned, newBitmap.GetPixelFormat());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, HasCopyConstructor) {
    Bitmap newBitmap(123, 234, PixelFormat::R5_G6_B5_Unsigned);
    Bitmap clone(newBitmap);
    EXPECT_EQ(123, clone.GetWidth());
    EXPECT_EQ(234, clone.GetHeight());
    EXPECT_EQ(PixelFormat::R5_G6_B5_Unsigned, clone.GetPixelFormat());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, HasMoveConstructor) {
    Bitmap newBitmap(12, 23, PixelFormat::Modern_A8_Unsigned);
    Bitmap moved = std::move(newBitmap);
    EXPECT_EQ(12, moved.GetWidth());
    EXPECT_EQ(23, moved.GetHeight());
    EXPECT_EQ(PixelFormat::Modern_A8_Unsigned, moved.GetPixelFormat());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, HasCopyAssignmentOperator) {
    Bitmap copied(1, 1);
    {
      Bitmap newBitmap(54, 42, PixelFormat::R8_G8_Signed);
      copied = newBitmap;
    }
    EXPECT_EQ(54, copied.GetWidth());
    EXPECT_EQ(42, copied.GetHeight());
    EXPECT_EQ(PixelFormat::R8_G8_Signed, copied.GetPixelFormat());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, HasMoveAssignmentOperator) {
    Bitmap moved(1, 1);
    {
      Bitmap newBitmap(65, 53, PixelFormat::R8_Unsigned);
      moved = std::move(newBitmap);
    }
    EXPECT_EQ(65, moved.GetWidth());
    EXPECT_EQ(53, moved.GetHeight());
    EXPECT_EQ(PixelFormat::R8_Unsigned, moved.GetPixelFormat());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, CanAccessRawMemory) {
    Bitmap newBitmap(98, 76, PixelFormat::Modern_R16_G16_B16_A16_Float);
    {
      BitmapMemory memory = newBitmap.Access();
      EXPECT_EQ(98, memory.Width);
      EXPECT_EQ(76, memory.Height);
      EXPECT_EQ(98 * 8, memory.Stride);
      EXPECT_EQ(PixelFormat::Modern_R16_G16_B16_A16_Float, memory.PixelFormat);
      ASSERT_NE(memory.Pixels, nullptr);

      // If too little memory is allocated, this should crash or trigger heap protection
      std::fill_n(
        reinterpret_cast<std::uint8_t *>(memory.Pixels),
        98 * 76 * 8,
        std::uint8_t(123)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(BitmapTest, CanUseExistingMemory) {
    std::uint8_t foreignBitmap[16 * 17];
    {
      BitmapMemory memory;
      memory.Width = 16;
      memory.Height = 17;
      memory.Stride = 16;
      memory.PixelFormat = PixelFormat::R8_Unsigned;
      memory.Pixels = foreignBitmap;

      Bitmap borrowedBitmap = Bitmap::FromExistingMemory(memory);
      EXPECT_EQ(16, borrowedBitmap.GetWidth());
      EXPECT_EQ(17, borrowedBitmap.GetHeight());
      EXPECT_EQ(PixelFormat::R8_Unsigned, borrowedBitmap.GetPixelFormat());
      
      {
        BitmapMemory accessedMemory = borrowedBitmap.Access();
        EXPECT_EQ(memory.Width, accessedMemory.Width);
        EXPECT_EQ(memory.Height, accessedMemory.Height);
        EXPECT_EQ(memory.Stride, accessedMemory.Stride);
        EXPECT_EQ(memory.PixelFormat, accessedMemory.PixelFormat);
        EXPECT_EQ(memory.Pixels, accessedMemory.Pixels);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
