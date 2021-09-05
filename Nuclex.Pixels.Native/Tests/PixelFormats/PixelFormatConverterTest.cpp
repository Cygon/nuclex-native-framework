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

#if 0
#include "../../Source/PixelFormats/PixelFormatConverter.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatConverterTest, UnsignedToUnsignedConversionWorks) {

    // Set up a pixel with 32 bit integer channels
    uint32_t testPixel = 0;
    {
      std::uint8_t *testPixelBytes = reinterpret_cast<std::uint8_t *>(&testPixel);
      *(testPixelBytes + 0) = 12;
      *(testPixelBytes + 1) = 34;
      *(testPixelBytes + 2) = 56;
      *(testPixelBytes + 3) = 78;
    }

    // Now let the pixel format converter change it to 8 bit unsigned integer channels
    std::uint64_t resultPixel = Convert<
      PixelFormat::A8_R8_G8_B8_Unsigned,
      PixelFormat::A16_R16_G16_B16_Unsigned
    >(testPixel);

    // Read the individual channels from the resulting pixel
    std::uint16_t r, g, b, a;
    {
      const std::uint8_t *resultPixelBytes = reinterpret_cast<const std::uint8_t *>(&resultPixel);

      a = *reinterpret_cast<const std::uint16_t *>(resultPixelBytes + 0);
      b = *reinterpret_cast<const std::uint16_t *>(resultPixelBytes + 2);
      g = *reinterpret_cast<const std::uint16_t *>(resultPixelBytes + 4);
      r = *reinterpret_cast<const std::uint16_t *>(resultPixelBytes + 8);
    }

    EXPECT_EQ(r, static_cast<std::uint16_t>(12 * 65535 / 255));
    EXPECT_EQ(g, static_cast<std::uint16_t>(34 * 65535 / 255));
    EXPECT_EQ(b, static_cast<std::uint16_t>(56 * 65535 / 255));
    EXPECT_EQ(a, static_cast<std::uint16_t>(78 * 65535 / 255));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatConverterTest, HalfToFloatConversionWorks) {

    // Set up a pixel with 16 bit half-precision floating point channels
    std::uint64_t testPixel = 0;
    {
      std::uint8_t *testPixelBytes = reinterpret_cast<std::uint8_t *>(&testPixel);

      *reinterpret_cast<Half *>(testPixelBytes + 0) = Half(1.2f);
      *reinterpret_cast<Half *>(testPixelBytes + 2) = Half(3.4f);
      *reinterpret_cast<Half *>(testPixelBytes + 4) = Half(5.6f);
      *reinterpret_cast<Half *>(testPixelBytes + 6) = Half(7.8f);
    }

    // Now let the pixel format converter change it to 32 bit floating point channels
    uint128_t resultPixel = Convert<
      PixelFormat::A16_B16_G16_R16_Float_Native16,
      PixelFormat::A32_B32_G32_R32_Float_Native32
    >(testPixel);

    // Read the individual channels from the resulting pixel
    float r, g, b, a;
    {
      const std::uint8_t *resultPixelBytes = reinterpret_cast<const std::uint8_t *>(&resultPixel);

      a = *reinterpret_cast<const float *>(resultPixelBytes + 0);
      b = *reinterpret_cast<const float *>(resultPixelBytes + 4);
      g = *reinterpret_cast<const float *>(resultPixelBytes + 8);
      r = *reinterpret_cast<const float *>(resultPixelBytes + 12);
    }

    EXPECT_NEAR(a, 1.2f, 0.01f);
    EXPECT_NEAR(b, 3.4f, 0.01f);
    EXPECT_NEAR(g, 5.6f, 0.01f);
    EXPECT_NEAR(r, 7.8f, 0.01f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatConverterTest, UnsignedToFloatConversionWorks) {

    // Set up a pixel with 8 bit unsigned integer channels
    std::uint32_t testPixel = 0;
    {
      std::uint8_t *testPixelBytes = reinterpret_cast<std::uint8_t *>(&testPixel);
      testPixelBytes[0] = 12;
      testPixelBytes[1] = 34;
      testPixelBytes[2] = 56;
      testPixelBytes[3] = 78;
    }

    // Now let the pixel format converter change it to 32 bit floating point channels
    uint128_t resultPixel = Convert<
      PixelFormat::A8_B8_G8_R8_Unsigned_Native32,
      PixelFormat::A32_B32_G32_R32_Float_Native32
    >(testPixel);

    // Read the individual channels from the resulting pixel
    float r, g, b, a;
    {
      const std::uint8_t *resultPixelBytes = reinterpret_cast<const std::uint8_t *>(&resultPixel);

      a = *reinterpret_cast<const float *>(resultPixelBytes + 0);
      b = *reinterpret_cast<const float *>(resultPixelBytes + 4);
      g = *reinterpret_cast<const float *>(resultPixelBytes + 8);
      r = *reinterpret_cast<const float *>(resultPixelBytes + 12);
    }

    EXPECT_NEAR(r, 12.0f / 255.0f, 0.01f);
    EXPECT_NEAR(g, 34.0f / 255.0f, 0.01f);
    EXPECT_NEAR(b, 56.0f / 255.0f, 0.01f);
    EXPECT_NEAR(a, 78.0f / 255.0f, 0.01f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatConverterTest, FloatToUnsignedConversionWorks) {

    // Set up a pixel with 32 bit floating point channels
    uint128_t testPixel = 0;
    {
      std::uint8_t *testPixelBytes = reinterpret_cast<std::uint8_t *>(&testPixel);

      *reinterpret_cast<float *>(testPixelBytes + 0) = 12.34f / 255.0f;
      *reinterpret_cast<float *>(testPixelBytes + 4) = 34.56f / 255.0f;
      *reinterpret_cast<float *>(testPixelBytes + 8) = 56.78f / 255.0f;
      *reinterpret_cast<float *>(testPixelBytes + 12) = 78.90f / 255.0f;
    }

    // Now let the pixel format converter change it to 8 bit unsigned integer channels
    std::uint32_t resultPixel = Convert<
      PixelFormat::A32_B32_G32_R32_Float_Native32,
      PixelFormat::A8_B8_G8_R8_Unsigned_Native32
    >(testPixel);

    // Read the individual channels from the resulting pixel
    std::uint8_t r, g, b, a;
    {
      const std::uint8_t *resultPixelBytes = reinterpret_cast<const std::uint8_t *>(&resultPixel);

      a = *(resultPixelBytes + 0);
      b = *(resultPixelBytes + 1);
      g = *(resultPixelBytes + 2);
      r = *(resultPixelBytes + 3);
    }

    EXPECT_EQ(r, static_cast<std::uint8_t>(12.34f));
    EXPECT_EQ(g, static_cast<std::uint8_t>(34.56f));
    EXPECT_EQ(b, static_cast<std::uint8_t>(56.78f));
    EXPECT_EQ(a, static_cast<std::uint8_t>(78.90f));
  }

  // ------------------------------------------------------------------------------------------- //
#if 0
  TEST(PixelFormatConverterTest, CanConvert_R8_Unsigned_to_R8_G8_Unsigned) {
    typedef PixelFormatConverter<
      PixelFormat::R8_Unsigned, PixelFormat::R8_G8_Unsigned
    > ConverterType;

    for(std::size_t value = 0; value < 256; ++value) {
      std::uint16_t result = ConverterType::Convert(static_cast<std::uint8_t>(value));
      const std::uint8_t *resultBytes = reinterpret_cast<std::uint8_t *>(&result);
      EXPECT_EQ(resultBytes[0], static_cast<std::uint8_t>(value));
      EXPECT_EQ(resultBytes[1], 0);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatConverterTest, CanConvert_R8_G8_Unsigned_to_A8_B8_G8_R8_Unsigned_Native32) {
    typedef PixelFormatConverter<
      PixelFormat::R8_G8_Unsigned, PixelFormat::A8_B8_G8_R8_Unsigned_Native32
    > ConverterType;

    for(std::size_t value = 0; value < 256; ++value) {
      std::uint16_t redInput = static_cast<std::uint16_t>(value);
      std::uint32_t result = ConverterType::Convert(redInput);
      EXPECT_EQ(result, value | 0xFF000000);

      std::uint16_t greenInput = static_cast<std::uint16_t>(value) << 8;
      result = ConverterType::Convert(greenInput);
      EXPECT_EQ(result, (value << 8) | 0xFF000000);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PixelFormatConverterTest, CanConvert_R8_G8_Unsigned_to_A8_B8_G8_R8_Unsigned_Flipped32) {
    typedef PixelFormatConverter<
      PixelFormat::R8_G8_Unsigned, PixelFormat::A8_B8_G8_R8_Unsigned_Flipped32
    > ConverterType;

    for(std::size_t value = 0; value < 256; ++value) {
      std::uint16_t redInput = static_cast<std::uint16_t>(value);
      std::uint32_t result = ConverterType::Convert(redInput);
      EXPECT_EQ(result, (value << 24) | 0x000000FF);

      std::uint16_t greenInput = static_cast<std::uint16_t>(value) << 8;
      result = ConverterType::Convert(greenInput);
      EXPECT_EQ(result, (value << 16) | 0x000000FF);
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
#endif