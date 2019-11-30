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

#include "Nuclex/Pixels/Storage/OptionalBitmap.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalBitmapTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      OptionalBitmap bitmap;
      (void)bitmap;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalBitmapTest, DefaultInstanceIsEmpty) {
    OptionalBitmap emptyBitmap;

    EXPECT_FALSE(emptyBitmap.HasValue());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalBitmapTest, ConstructorAcceptsBitmap) {
    Bitmap testBitmap(12, 34);
    OptionalBitmap bitmapContainer(std::move(testBitmap));

    EXPECT_TRUE(bitmapContainer.HasValue());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalBitmapTest, BitmapCanBeTakenOut) {
    Bitmap testBitmap(12, 34);
    OptionalBitmap bitmapContainer(std::move(testBitmap));

    EXPECT_TRUE(bitmapContainer.HasValue());
    Bitmap returnedBitmap = bitmapContainer.Take();

    EXPECT_FALSE(bitmapContainer.HasValue());
    EXPECT_EQ(returnedBitmap.GetWidth(), 12);
    EXPECT_EQ(returnedBitmap.GetHeight(), 34);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalBitmapTest, OptionalBitmapCanBeCopied) {
    Bitmap testBitmap(12, 34);
    OptionalBitmap bitmapContainer(std::move(testBitmap));

    OptionalBitmap copy(bitmapContainer);

    EXPECT_TRUE(bitmapContainer.HasValue());
    EXPECT_TRUE(copy.HasValue());

    Bitmap returnedBitmap = copy.Take();
    EXPECT_TRUE(bitmapContainer.HasValue());
    EXPECT_FALSE(copy.HasValue());

    EXPECT_EQ(returnedBitmap.GetWidth(), 12);
    EXPECT_EQ(returnedBitmap.GetHeight(), 34);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(OptionalBitmapTest, OptionalBitmapCanBeMoved) {
    Bitmap testBitmap(12, 34);
    OptionalBitmap bitmapContainer(std::move(testBitmap));

    OptionalBitmap copy(std::move(bitmapContainer));

    EXPECT_TRUE(copy.HasValue());

    Bitmap returnedBitmap = copy.Take();
    EXPECT_FALSE(copy.HasValue());

    EXPECT_EQ(returnedBitmap.GetWidth(), 12);
    EXPECT_EQ(returnedBitmap.GetHeight(), 34);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
