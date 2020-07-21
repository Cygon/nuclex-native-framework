#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Pixels/Rectangle.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(RectangleTest, ConstructorAppliesSpecifiedBounds) {
    Rectangle rectangle(112, 223, 445, 556);

    EXPECT_EQ(112U, rectangle.MinX);
    EXPECT_EQ(223U, rectangle.MinY);
    EXPECT_EQ(445U, rectangle.MaxX);
    EXPECT_EQ(556U, rectangle.MaxY);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RectangleTest, HasNamedConstructorForMinAndMaxCorner) {
    Rectangle rectangle = Rectangle::FromMinAndMax(9, 8, 7, 6);

    EXPECT_EQ(9U, rectangle.MinX);
    EXPECT_EQ(8U, rectangle.MinY);
    EXPECT_EQ(7U, rectangle.MaxX);
    EXPECT_EQ(6U, rectangle.MaxY);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RectangleTest, HasNamedConstructorForPositionAndSize) {
    Rectangle rectangle = Rectangle::FromPositionAndSize(100, 200, 23, 34);

    EXPECT_EQ(100U, rectangle.MinX);
    EXPECT_EQ(200U, rectangle.MinY);
    EXPECT_EQ(123U, rectangle.MaxX);
    EXPECT_EQ(234U, rectangle.MaxY);
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
