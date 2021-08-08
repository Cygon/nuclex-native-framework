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

#include "Nuclex/Pixels/ColorModels/ColorModelConverter.h"
#include <gtest/gtest.h>

#include <random>

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, CanConvertRgbtoHsv) {
    HsvColor red = ColorModelConverter::HsvFromRgb(RgbColor { 1.0f, 0.0f, 0.0f, 1.0f });
    EXPECT_EQ(red.Hue, HsvColor::RedHue);
    EXPECT_EQ(red.Saturation, 1.0f);
    EXPECT_EQ(red.Value, 1.0f);

    HsvColor yellow = ColorModelConverter::HsvFromRgb(RgbColor{ 1.0f, 1.0f, 0.0f, 1.0f });
    EXPECT_EQ(yellow.Hue, HsvColor::YellowHue);
    EXPECT_EQ(yellow.Saturation, 1.0f);
    EXPECT_EQ(yellow.Value, 1.0f);

    HsvColor green = ColorModelConverter::HsvFromRgb(RgbColor{ 0.0f, 1.0f, 0.0f, 1.0f });
    EXPECT_EQ(green.Hue, HsvColor::GreenHue);
    EXPECT_EQ(green.Saturation, 1.0f);
    EXPECT_EQ(green.Value, 1.0f);

    HsvColor cyan = ColorModelConverter::HsvFromRgb(RgbColor{ 0.0f, 1.0f, 1.0f, 1.0f });
    EXPECT_EQ(cyan.Hue, HsvColor::CyanHue);
    EXPECT_EQ(cyan.Saturation, 1.0f);
    EXPECT_EQ(cyan.Value, 1.0f);

    HsvColor blue = ColorModelConverter::HsvFromRgb(RgbColor{ 0.0f, 0.0f, 1.0f, 1.0f });
    EXPECT_EQ(blue.Hue, HsvColor::BlueHue);
    EXPECT_EQ(blue.Saturation, 1.0f);
    EXPECT_EQ(blue.Value, 1.0f);

    HsvColor magenta = ColorModelConverter::HsvFromRgb(RgbColor{ 1.0f, 0.0f, 1.0f, 1.0f });
    EXPECT_EQ(magenta.Hue, HsvColor::MagentaHue);
    EXPECT_EQ(magenta.Saturation, 1.0f);
    EXPECT_EQ(magenta.Value, 1.0f);

    HsvColor black = ColorModelConverter::HsvFromRgb(RgbColor{ 0.0f, 0.0f, 0.0f, 1.0f });
    EXPECT_EQ(black.Saturation, 0.0f);
    EXPECT_EQ(black.Value, 0.0f);

    HsvColor white = ColorModelConverter::HsvFromRgb(RgbColor{ 1.0f, 1.0f, 1.0f, 1.0f });
    EXPECT_EQ(white.Saturation, 0.0f);
    EXPECT_EQ(white.Value, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, CanConvertHsvToRgb) {
    HsvColor hsvRed = HsvColor { HsvColor::RedHue, 1.0f, 1.0f, 1.0f };
    RgbColor rgbRed = ColorModelConverter::RgbFromHsv(hsvRed);
    EXPECT_EQ(rgbRed.Red, 1.0f);
    EXPECT_EQ(rgbRed.Green, 0.0f);
    EXPECT_EQ(rgbRed.Blue, 0.0f);

    HsvColor hsvYellow = HsvColor { HsvColor::YellowHue, 1.0f, 1.0f, 1.0f };
    RgbColor rgbYellow = ColorModelConverter::RgbFromHsv(hsvYellow);
    EXPECT_EQ(rgbYellow.Red, 1.0f);
    EXPECT_EQ(rgbYellow.Green, 1.0f);
    EXPECT_EQ(rgbYellow.Blue, 0.0f);

    HsvColor hsvGreen = HsvColor { HsvColor::GreenHue, 1.0f, 1.0f, 1.0f };
    RgbColor rgbGreen = ColorModelConverter::RgbFromHsv(hsvGreen);
    EXPECT_EQ(rgbGreen.Red, 0.0f);
    EXPECT_EQ(rgbGreen.Green, 1.0f);
    EXPECT_EQ(rgbGreen.Blue, 0.0f);

    HsvColor hsvCyan = HsvColor { HsvColor::CyanHue, 1.0f, 1.0f, 1.0f };
    RgbColor rgbCyan = ColorModelConverter::RgbFromHsv(hsvCyan);
    EXPECT_EQ(rgbCyan.Red, 0.0f);
    EXPECT_EQ(rgbCyan.Green, 1.0f);
    EXPECT_EQ(rgbCyan.Blue, 1.0f);

    HsvColor hsvBlue = HsvColor { HsvColor::BlueHue, 1.0f, 1.0f, 1.0f };
    RgbColor rgbBlue = ColorModelConverter::RgbFromHsv(hsvBlue);
    EXPECT_EQ(rgbBlue.Red, 0.0f);
    EXPECT_EQ(rgbBlue.Green, 0.0f);
    EXPECT_EQ(rgbBlue.Blue, 1.0f);

    HsvColor hsvMagenta = HsvColor { HsvColor::MagentaHue, 1.0f, 1.0f, 1.0f };
    RgbColor rgbMagenta = ColorModelConverter::RgbFromHsv(hsvMagenta);
    EXPECT_EQ(rgbMagenta.Red, 1.0f);
    EXPECT_EQ(rgbMagenta.Green, 0.0f);
    EXPECT_EQ(rgbMagenta.Blue, 1.0f);

    HsvColor hsvBlack = HsvColor { 0.0f, 0.0f, 0.0f, 1.0f };
    RgbColor rgbBlack = ColorModelConverter::RgbFromHsv(hsvBlack);
    EXPECT_EQ(rgbBlack.Red, 0.0f);
    EXPECT_EQ(rgbBlack.Green, 0.0f);
    EXPECT_EQ(rgbBlack.Blue, 0.0f);

    HsvColor hsvWhite = HsvColor { 0.0f, 0.0f, 1.0f, 1.0f };
    RgbColor rgbWhite = ColorModelConverter::RgbFromHsv(hsvWhite);
    EXPECT_EQ(rgbWhite.Red, 1.0f);
    EXPECT_EQ(rgbWhite.Green, 1.0f);
    EXPECT_EQ(rgbWhite.Blue, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, RgbAndHsvConversionRoundTrips) {
    static constexpr float MaximumError = 0.000001f;

    std::default_random_engine randomNumberGenerator;
    std::uniform_real_distribution<float> uniformUnitDistribution(0.0f, 1.0f);

    for(std::size_t index = 0; index < 5000; ++index) {
      RgbColor randomRgb = {
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        1.0f
      };
      HsvColor convertedHsv = ColorModelConverter::HsvFromRgb(randomRgb);
      RgbColor convertedRgb = ColorModelConverter::RgbFromHsv(convertedHsv);

      EXPECT_NEAR(convertedRgb.Red, randomRgb.Red, MaximumError);
      EXPECT_NEAR(convertedRgb.Green, randomRgb.Green, MaximumError);
      EXPECT_NEAR(convertedRgb.Blue, randomRgb.Blue, MaximumError);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, CanConvertRgbtoHsl) {
    HslColor red = ColorModelConverter::HslFromRgb(RgbColor { 1.0f, 0.0f, 0.0f, 1.0f });
    EXPECT_EQ(red.Hue, HslColor::RedHue);
    EXPECT_EQ(red.Saturation, 1.0f);
    EXPECT_EQ(red.Lightness, 0.5f);

    HslColor yellow = ColorModelConverter::HslFromRgb(RgbColor{ 1.0f, 1.0f, 0.0f, 1.0f });
    EXPECT_EQ(yellow.Hue, HslColor::YellowHue);
    EXPECT_EQ(yellow.Saturation, 1.0f);
    EXPECT_EQ(yellow.Lightness, 0.5f);

    HslColor green = ColorModelConverter::HslFromRgb(RgbColor{ 0.0f, 1.0f, 0.0f, 1.0f });
    EXPECT_EQ(green.Hue, HslColor::GreenHue);
    EXPECT_EQ(green.Saturation, 1.0f);
    EXPECT_EQ(green.Lightness, 0.5f);

    HslColor cyan = ColorModelConverter::HslFromRgb(RgbColor{ 0.0f, 1.0f, 1.0f, 1.0f });
    EXPECT_EQ(cyan.Hue, HslColor::CyanHue);
    EXPECT_EQ(cyan.Saturation, 1.0f);
    EXPECT_EQ(cyan.Lightness, 0.5f);

    HslColor blue = ColorModelConverter::HslFromRgb(RgbColor{ 0.0f, 0.0f, 1.0f, 1.0f });
    EXPECT_EQ(blue.Hue, HslColor::BlueHue);
    EXPECT_EQ(blue.Saturation, 1.0f);
    EXPECT_EQ(blue.Lightness, 0.5f);

    HslColor magenta = ColorModelConverter::HslFromRgb(RgbColor{ 1.0f, 0.0f, 1.0f, 1.0f });
    EXPECT_EQ(magenta.Hue, HslColor::MagentaHue);
    EXPECT_EQ(magenta.Saturation, 1.0f);
    EXPECT_EQ(magenta.Lightness, 0.5f);

    HslColor black = ColorModelConverter::HslFromRgb(RgbColor{ 0.0f, 0.0f, 0.0f, 1.0f });
    EXPECT_EQ(black.Saturation, 0.0f);
    EXPECT_EQ(black.Lightness, 0.0f);

    HslColor white = ColorModelConverter::HslFromRgb(RgbColor{ 1.0f, 1.0f, 1.0f, 1.0f });
    EXPECT_EQ(white.Saturation, 0.0f);
    EXPECT_EQ(white.Lightness, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, CanConvertHslToRgb) {
    HslColor hslRed = HslColor { HslColor::RedHue, 1.0f, 0.5f, 1.0f };
    RgbColor rgbRed = ColorModelConverter::RgbFromHsl(hslRed);
    EXPECT_EQ(rgbRed.Red, 1.0f);
    EXPECT_EQ(rgbRed.Green, 0.0f);
    EXPECT_EQ(rgbRed.Blue, 0.0f);

    HslColor hslYellow = HslColor { HslColor::YellowHue, 1.0f, 0.5f, 1.0f };
    RgbColor rgbYellow = ColorModelConverter::RgbFromHsl(hslYellow);
    EXPECT_EQ(rgbYellow.Red, 1.0f);
    EXPECT_EQ(rgbYellow.Green, 1.0f);
    EXPECT_EQ(rgbYellow.Blue, 0.0f);

    HslColor hslGreen = HslColor { HslColor::GreenHue, 1.0f, 0.5f, 1.0f };
    RgbColor rgbGreen = ColorModelConverter::RgbFromHsl(hslGreen);
    EXPECT_EQ(rgbGreen.Red, 0.0f);
    EXPECT_EQ(rgbGreen.Green, 1.0f);
    EXPECT_EQ(rgbGreen.Blue, 0.0f);

    HslColor hslCyan = HslColor { HslColor::CyanHue, 1.0f, 0.5f, 1.0f };
    RgbColor rgbCyan = ColorModelConverter::RgbFromHsl(hslCyan);
    EXPECT_EQ(rgbCyan.Red, 0.0f);
    EXPECT_EQ(rgbCyan.Green, 1.0f);
    EXPECT_EQ(rgbCyan.Blue, 1.0f);

    HslColor hslBlue = HslColor { HslColor::BlueHue, 1.0f, 0.5f, 1.0f };
    RgbColor rgbBlue = ColorModelConverter::RgbFromHsl(hslBlue);
    EXPECT_EQ(rgbBlue.Red, 0.0f);
    EXPECT_EQ(rgbBlue.Green, 0.0f);
    EXPECT_EQ(rgbBlue.Blue, 1.0f);

    HslColor hslMagenta = HslColor { HslColor::MagentaHue, 1.0f, 0.5f, 1.0f };
    RgbColor rgbMagenta = ColorModelConverter::RgbFromHsl(hslMagenta);
    EXPECT_EQ(rgbMagenta.Red, 1.0f);
    EXPECT_EQ(rgbMagenta.Green, 0.0f);
    EXPECT_EQ(rgbMagenta.Blue, 1.0f);

    HslColor hslBlack = HslColor { 0.0f, 0.0f, 0.0f, 1.0f };
    RgbColor rgbBlack = ColorModelConverter::RgbFromHsl(hslBlack);
    EXPECT_EQ(rgbBlack.Red, 0.0f);
    EXPECT_EQ(rgbBlack.Green, 0.0f);
    EXPECT_EQ(rgbBlack.Blue, 0.0f);

    HslColor hslWhite = HslColor { 0.0f, 0.0f, 1.0f, 1.0f };
    RgbColor rgbWhite = ColorModelConverter::RgbFromHsl(hslWhite);
    EXPECT_EQ(rgbWhite.Red, 1.0f);
    EXPECT_EQ(rgbWhite.Green, 1.0f);
    EXPECT_EQ(rgbWhite.Blue, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, RgbAndHslConversionRoundTrips) {
    static constexpr float MaximumError = 0.000001f;

    std::default_random_engine randomNumberGenerator;
    std::uniform_real_distribution<float> uniformUnitDistribution(0.0f, 1.0f);

    for(std::size_t index = 0; index < 5000; ++index) {
      RgbColor randomRgb = {
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        1.0f
      };
      HslColor convertedHsl = ColorModelConverter::HslFromRgb(randomRgb);
      RgbColor convertedRgb = ColorModelConverter::RgbFromHsl(convertedHsl);

      EXPECT_NEAR(convertedRgb.Red, randomRgb.Red, MaximumError);
      EXPECT_NEAR(convertedRgb.Green, randomRgb.Green, MaximumError);
      EXPECT_NEAR(convertedRgb.Blue, randomRgb.Blue, MaximumError);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, CanConvertHsvToHsl) {
    HsvColor hsvRed = HsvColor { HsvColor::RedHue, 1.0f, 1.0f, 1.0f };
    HslColor hslRed = ColorModelConverter::HslFromHsv(hsvRed);
    EXPECT_EQ(hslRed.Hue, hsvRed.Hue);
    EXPECT_EQ(hslRed.Saturation, 1.0f);
    EXPECT_EQ(hslRed.Lightness, 0.5f);

    HsvColor hsvYellow = HsvColor { HsvColor::YellowHue, 1.0f, 1.0f, 1.0f };
    HslColor hslYellow = ColorModelConverter::HslFromHsv(hsvYellow);
    EXPECT_EQ(hslYellow.Hue, hsvYellow.Hue);
    EXPECT_EQ(hslYellow.Saturation, 1.0f);
    EXPECT_EQ(hslYellow.Lightness, 0.5f);

    HsvColor hsvGreen = HsvColor { HsvColor::GreenHue, 1.0f, 1.0f, 1.0f };
    HslColor hslGreen = ColorModelConverter::HslFromHsv(hsvGreen);
    EXPECT_EQ(hslGreen.Hue, hsvGreen.Hue);
    EXPECT_EQ(hslGreen.Saturation, 1.0f);
    EXPECT_EQ(hslGreen.Lightness, 0.5f);

    HsvColor hsvCyan = HsvColor { HsvColor::CyanHue, 1.0f, 1.0f, 1.0f };
    HslColor hslCyan = ColorModelConverter::HslFromHsv(hsvCyan);
    EXPECT_EQ(hslCyan.Hue, hsvCyan.Hue);
    EXPECT_EQ(hslCyan.Saturation, 1.0f);
    EXPECT_EQ(hslCyan.Lightness, 0.5f);

    HsvColor hsvBlue = HsvColor { HsvColor::BlueHue, 1.0f, 1.0f, 1.0f };
    HslColor hslBlue = ColorModelConverter::HslFromHsv(hsvBlue);
    EXPECT_EQ(hslBlue.Hue, hsvBlue.Hue);
    EXPECT_EQ(hslBlue.Saturation, 1.0f);
    EXPECT_EQ(hslBlue.Lightness, 0.5f);

    HsvColor hsvMagenta = HsvColor { HsvColor::MagentaHue, 1.0f, 1.0f, 1.0f };
    HslColor hslMagenta = ColorModelConverter::HslFromHsv(hsvMagenta);
    EXPECT_EQ(hslMagenta.Hue, hsvMagenta.Hue);
    EXPECT_EQ(hslMagenta.Saturation, 1.0f);
    EXPECT_EQ(hslMagenta.Lightness, 0.5f);

    HsvColor hsvBlack = HsvColor { 0.0f, 0.0f, 0.0f, 1.0f };
    HslColor hslBlack = ColorModelConverter::HslFromHsv(hsvBlack);
    EXPECT_EQ(hslBlack.Saturation, 0.0f);
    EXPECT_EQ(hslBlack.Lightness, 0.0f);

    HsvColor hsvWhite = HsvColor { 0.0f, 0.0f, 1.0f, 1.0f };
    HslColor hslWhite = ColorModelConverter::HslFromHsv(hsvWhite);
    EXPECT_EQ(hslWhite.Saturation, 0.0f);
    EXPECT_EQ(hslWhite.Lightness, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, CanConvertHslToHsv) {
    HslColor hslRed = HslColor { HslColor::RedHue, 1.0f, 0.5f, 1.0f };
    HsvColor hsvRed = ColorModelConverter::HsvFromHsl(hslRed);
    EXPECT_EQ(hsvRed.Hue, hsvRed.Hue);
    EXPECT_EQ(hsvRed.Saturation, 1.0f);
    EXPECT_EQ(hsvRed.Value, 1.0f);

    HslColor hslYellow = HslColor { HslColor::YellowHue, 1.0f, 0.5f, 1.0f };
    HsvColor hsvYellow = ColorModelConverter::HsvFromHsl(hslYellow);
    EXPECT_EQ(hsvYellow.Hue, hsvYellow.Hue);
    EXPECT_EQ(hsvYellow.Saturation, 1.0f);
    EXPECT_EQ(hsvYellow.Value, 1.0f);

    HslColor hslGreen = HslColor { HslColor::GreenHue, 1.0f, 0.5f, 1.0f };
    HsvColor hsvGreen = ColorModelConverter::HsvFromHsl(hslGreen);
    EXPECT_EQ(hsvGreen.Hue, hsvGreen.Hue);
    EXPECT_EQ(hsvGreen.Saturation, 1.0f);
    EXPECT_EQ(hsvGreen.Value, 1.0f);

    HslColor hslCyan = HslColor { HslColor::CyanHue, 1.0f, 0.5f, 1.0f };
    HsvColor hsvCyan = ColorModelConverter::HsvFromHsl(hslCyan);
    EXPECT_EQ(hsvCyan.Hue, hsvCyan.Hue);
    EXPECT_EQ(hsvCyan.Saturation, 1.0f);
    EXPECT_EQ(hsvCyan.Value, 1.0f);

    HslColor hslBlue = HslColor { HslColor::BlueHue, 1.0f, 0.5f, 1.0f };
    HsvColor hsvBlue = ColorModelConverter::HsvFromHsl(hslBlue);
    EXPECT_EQ(hsvBlue.Hue, hsvBlue.Hue);
    EXPECT_EQ(hsvBlue.Saturation, 1.0f);
    EXPECT_EQ(hsvBlue.Value, 1.0f);

    HslColor hslMagenta = HslColor { HslColor::MagentaHue, 1.0f, 0.5f, 1.0f };
    HsvColor hsvMagenta = ColorModelConverter::HsvFromHsl(hslMagenta);
    EXPECT_EQ(hsvMagenta.Hue, hsvMagenta.Hue);
    EXPECT_EQ(hsvMagenta.Saturation, 1.0f);
    EXPECT_EQ(hsvMagenta.Value, 1.0f);

    HslColor hslBlack = HslColor { 0.0f, 0.0f, 0.0f, 1.0f };
    HsvColor hsvBlack = ColorModelConverter::HsvFromHsl(hslBlack);
    EXPECT_EQ(hsvBlack.Saturation, 0.0f);
    EXPECT_EQ(hsvBlack.Value, 0.0f);

    HslColor hslWhite = HslColor { 0.0f, 0.0f, 1.0f, 1.0f };
    HsvColor hsvWhite = ColorModelConverter::HsvFromHsl(hslWhite);
    EXPECT_EQ(hsvWhite.Saturation, 0.0f);
    EXPECT_EQ(hsvWhite.Value, 1.0f);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, ThreePointRgbHslHsvRoundTripWorks) {
    static constexpr float MaximumError = 0.000001f;

    std::default_random_engine randomNumberGenerator;
    std::uniform_real_distribution<float> uniformUnitDistribution(0.0f, 1.0f);

    for(std::size_t index = 0; index < 5000; ++index) {
      RgbColor randomRgb = {
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        1.0f
      };
      HslColor convertedHsl = ColorModelConverter::HslFromRgb(randomRgb);
      HsvColor convertedHsv = ColorModelConverter::HsvFromHsl(convertedHsl);
      RgbColor convertedRgb = ColorModelConverter::RgbFromHsv(convertedHsv);

      EXPECT_NEAR(convertedRgb.Red, randomRgb.Red, MaximumError);
      EXPECT_NEAR(convertedRgb.Green, randomRgb.Green, MaximumError);
      EXPECT_NEAR(convertedRgb.Blue, randomRgb.Blue, MaximumError);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, ThreePointRgbHsvHslRoundTripWorks) {
    static constexpr float MaximumError = 0.000001f;

    std::default_random_engine randomNumberGenerator;
    std::uniform_real_distribution<float> uniformUnitDistribution(0.0f, 1.0f);

    for(std::size_t index = 0; index < 5000; ++index) {
      RgbColor randomRgb = {
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        uniformUnitDistribution(randomNumberGenerator),
        1.0f
      };
      HsvColor convertedHsv = ColorModelConverter::HsvFromRgb(randomRgb);
      HslColor convertedHsl = ColorModelConverter::HslFromHsv(convertedHsv);
      RgbColor convertedRgb = ColorModelConverter::RgbFromHsl(convertedHsl);

      EXPECT_NEAR(convertedRgb.Red, randomRgb.Red, MaximumError);
      EXPECT_NEAR(convertedRgb.Green, randomRgb.Green, MaximumError);
      EXPECT_NEAR(convertedRgb.Blue, randomRgb.Blue, MaximumError);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, RgbYuvRoundtripWorksInBt470) {
    static constexpr float MaximumError = 0.0000001f;

    YuvColor yuv = ColorModelConverter::YuvFromRgb(
      { 1.0f, 1.0f, 1.0f, 1.0f }, YuvColorSystem::Bt470
    );
    RgbColor rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt470);

    EXPECT_NEAR(rgb.Red, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 1.0f, 0.0f, 0.0f, 1.0f }, YuvColorSystem::Bt470
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt470);

    EXPECT_NEAR(rgb.Red, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 0.0f, 1.0f, 0.0f, 1.0f }, YuvColorSystem::Bt470
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt470);

    EXPECT_NEAR(rgb.Red, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 0.0f, 0.0f, 1.0f, 1.0f }, YuvColorSystem::Bt470
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt470);

    EXPECT_NEAR(rgb.Red, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, RgbYuvRoundtripWorksInBt709) {
    static constexpr float MaximumError = 0.0000001f;

    YuvColor yuv = ColorModelConverter::YuvFromRgb(
      { 1.0f, 1.0f, 1.0f, 1.0f }, YuvColorSystem::Bt709
    );
    RgbColor rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt709);

    EXPECT_NEAR(rgb.Red, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 1.0f, 0.0f, 0.0f, 1.0f }, YuvColorSystem::Bt709
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt709);

    EXPECT_NEAR(rgb.Red, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 0.0f, 1.0f, 0.0f, 1.0f }, YuvColorSystem::Bt709
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt709);

    EXPECT_NEAR(rgb.Red, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 0.0f, 0.0f, 1.0f, 1.0f }, YuvColorSystem::Bt709
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt709);

    EXPECT_NEAR(rgb.Red, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ColorModelConverterTest, RgbYuvRoundtripWorksInBt2020) {
    static constexpr float MaximumError = 0.0000001f;

    YuvColor yuv = ColorModelConverter::YuvFromRgb(
      { 1.0f, 1.0f, 1.0f, 1.0f }, YuvColorSystem::Bt2020
    );
    RgbColor rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt2020);

    EXPECT_NEAR(rgb.Red, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 1.0f, 0.0f, 0.0f, 1.0f }, YuvColorSystem::Bt2020
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt2020);

    EXPECT_NEAR(rgb.Red, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 0.0f, 1.0f, 0.0f, 1.0f }, YuvColorSystem::Bt2020
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt2020);

    EXPECT_NEAR(rgb.Red, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);

    yuv = ColorModelConverter::YuvFromRgb(
      { 0.0f, 0.0f, 1.0f, 1.0f }, YuvColorSystem::Bt2020
    );
    rgb = ColorModelConverter::RgbFromYuv(yuv, YuvColorSystem::Bt2020);

    EXPECT_NEAR(rgb.Red, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Green, 0.0f, MaximumError);
    EXPECT_NEAR(rgb.Blue, 1.0f, MaximumError);
    EXPECT_NEAR(rgb.Alpha, 1.0f, MaximumError);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels
