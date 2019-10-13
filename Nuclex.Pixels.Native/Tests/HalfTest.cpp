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

#include "Nuclex/Pixels/Half.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TEST(HalfTest, ZeroConstantIsCorrect) {
    Half zero = Half::Zero;

    EXPECT_EQ(0.0f, static_cast<float>(zero));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HalfTest, OneConstantIsCorrect) {
    Half one = Half::One;

    EXPECT_EQ(1.0f, static_cast<float>(one));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HalfTest, HalfCanBeConstructedFromNormalizedByte) {
    for(std::size_t value = 0; value < 256; ++value) {
      Half actual = Half::FromNormalizedByte(static_cast<std::uint8_t>(value));
      float expected = static_cast<float>(value / 255.0f);

      EXPECT_NEAR(expected, static_cast<float>(actual), 0.0005f);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(HalfTest, HalfCanBeConvertedToNormalizedByte) {
    for(std::size_t expected = 0; expected < 256; ++expected) {
      float value = static_cast<float>(expected / 255.0f);
      Half actual(value);

      EXPECT_EQ(expected, actual.ToNormalizedByte());
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
