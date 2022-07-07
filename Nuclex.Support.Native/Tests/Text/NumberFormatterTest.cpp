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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/ParserHelper.h"
#include "./../../Source/Text/NumberFormatter.h"
#include "./../../Source/Text/DragonBox-1.1.2/dragonbox_to_chars.h"

#include <gtest/gtest.h>

#include <random> // for std::uniform_int_distribution, std::uniform_real_distribution

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Replaces the decimal point with its localized equivalent</summary>
  /// <param name="numberAsText">String containing an unlocalized decimal point</param>
  void localizeDecimalPoint(std::string &numberAsText) {
    char localizedDecimalPoint = u8','; // std::numpunct::decimal_point()
    std::string silly = std::to_string(1.2f);
    localizedDecimalPoint = silly[1];

    std::string::size_type length = numberAsText.length();
    for(std::string::size_type index = 0; index < length; ++index) {
      if(numberAsText[index] == u8'.') {
        numberAsText[index] = localizedDecimalPoint;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Since we can't check all integers within a reasonable time, this is the number
  ///   of random checks we'll do to compare our integer formatter with std::to_string()
  /// </summary>
  const constexpr std::size_t SampleCount = 1'000;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, ThirtyTwoBitUnsignedIntegersAreFormattedCorrectly) {
    std::mt19937 randomNumberGenerator;
    std::uniform_int_distribution<std::uint32_t> randomNumberDistribution32;

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::uint32_t number = static_cast<std::uint32_t>(
        randomNumberDistribution32(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, ThirtyTwoBitSignedIntegersAreFormattedCorrectly) {
    std::mt19937 randomNumberGenerator;
    std::uniform_int_distribution<std::int32_t> randomNumberDistribution32(
      std::numeric_limits<std::int32_t>::min(),
      std::numeric_limits<std::int32_t>::max()
    );

    for(std::size_t index = 8; index < 13; ++index) {
      std::string expected = std::to_string(index);

      char buffer[40];
      char *end = FormatInteger(buffer, static_cast<std::uint32_t>(index));
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::int32_t number = static_cast<std::int32_t>(
        randomNumberDistribution32(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, SixtyFourBitUnsignedIntegersAreFormattedCorrectly) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_int_distribution<std::uint64_t> randomNumberDistribution64;

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::uint64_t number = static_cast<std::uint64_t>(
        randomNumberDistribution64(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, SixtyFourBitSignedIntegersAreFormattedCorrectly) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_int_distribution<std::int64_t> randomNumberDistribution64(
      std::numeric_limits<std::int64_t>::min(),
      std::numeric_limits<std::int64_t>::max()
    );

    for(std::size_t index = 0; index < SampleCount; ++index) {
      std::int64_t number = static_cast<std::int64_t>(
        randomNumberDistribution64(randomNumberGenerator)
      );

      std::string expected = std::to_string(number);

      char buffer[40];
      char *end = FormatInteger(buffer, number);
      std::string actual(buffer, end);

      EXPECT_EQ(expected, actual);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, LowestThirtyTwoBitSignedIntegersIsFormatted) {
    std::int32_t lowestValue = std::numeric_limits<std::int32_t>::min();

    std::string expected = std::to_string(lowestValue);

    char buffer[40];
    char *end = FormatInteger(buffer, lowestValue);
    std::string actual(buffer, end);

    EXPECT_EQ(expected, actual);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, LowestSixtyFourBitSignedIntegersIsFormatted) {
    std::int64_t lowestValue = std::numeric_limits<std::int64_t>::min();

    std::string expected = std::to_string(lowestValue);

    char buffer[40];
    char *end = FormatInteger(buffer, lowestValue);
    std::string actual(buffer, end);

    EXPECT_EQ(expected, actual);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, FloatingPointValuesCanBePrinted) {
    float numbers[] = {
      // Different odd/even digit counts before and after the decimal point
      0.123456f, 1.23456f, 12.3456f, 123.456f, 1234.56f, 12345.6f, 123456.0f,
      // Alternate odd/even digit counts before and after the decimal point
      0.12345f, 1.2345f, 12.345f, 123.45f, 1234.5f, 12345.0f,
      // Tiniest and largest negative float values
      -0.58775e-038f, -1.1755e-038f, -1.7014E+038f, -3.4028e+038f,
      // Tiniest and largest positive float values
      0.58775e-038f, 1.1755e-038f, 1.7014E+038f, 3.4028e+038f,
      // Large digit counts before and after the decimal point
      0.16777215f, 1.6777215f, 16777215.0f, 1677721.5f
    };

    for(float number : numbers) {
      char buffer[48];
      std::memset(buffer, 0, 48);

      char *end = FormatFloat(buffer, number);
      std::string formatted(buffer, end);
      localizeDecimalPoint(formatted);

      float actual = std::strtof(formatted.c_str(), &end);
      float expected = number;
      EXPECT_FLOAT_EQ(actual, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, DoublePrecisionFloatingPointValuesCanBePrinted) {
    double numbers[] = {
      // Different odd/even digit counts before and after the decimal point
      0.123456, 1.23456, 12.3456, 123.456, 1234.56, 12345.6, 123456.0,
      // Alternate odd/even digit counts before and after the decimal point
      0.12345, 1.2345, 12.345, 123.45, 1234.5, 12345.0,
      // Tiniest and largest negative double values
      -2.2251e-308, -1.11255e-308, -1.7976931348623157E+308, -0.8988465674311579E+308,
      // Tiniest and largest positive double values
      2.2251e-308, 1.11255e-308, 1.7976931348623157E+308, 0.8988465674311579E+308,
      // Large digit counts before and after the decimal point
      0.4503599627370495, 4.503599627370495, 4503599627370495.0, 450359962737049.5
    };

    for(double number : numbers) {
      char buffer[325];
      std::memset(buffer, 0, 325);

      char *end = FormatFloat(buffer, number);
      std::string formatted(buffer, end);
      localizeDecimalPoint(formatted);

      double actual = std::strtod(formatted.c_str(), &end);
      double expected = number;
      EXPECT_DOUBLE_EQ(actual, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, SmallFloatingPointValuesCanBePrinted) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_real_distribution<float> randomNumberDistribution(-1.0f, +1.0f);

    for(std::size_t index = 0; index < SampleCount; ++index) {
      float number = static_cast<float>(randomNumberDistribution(randomNumberGenerator));

      char buffer[48];
      char *end = FormatFloat(buffer, number);
      std::string formatted(buffer, end);
      localizeDecimalPoint(formatted);

      float actual = std::strtof(formatted.c_str(), &end);
      float expected = number;
      EXPECT_EQ(actual, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, LargeFloatingPointValuesCanBePrinted) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_real_distribution<float> randomNumberDistribution(
      std::numeric_limits<float>::lowest() / 2.1f, std::numeric_limits<float>::max() / 2.1f
    );

    for(std::size_t index = 0; index < SampleCount; ++index) {
      float number = static_cast<float>(randomNumberDistribution(randomNumberGenerator));

      char buffer[48];
      char *end = FormatFloat(buffer, number);
      std::string formatted(buffer, end);
      localizeDecimalPoint(formatted);

      float actual = std::strtof(formatted.c_str(), &end);
      float expected = number;
      EXPECT_FLOAT_EQ(actual, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, SmallDoublePrecisionFloatingPointValuesCanBePrinted) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_real_distribution<double> randomNumberDistribution(-1.0f, +1.0f);

    for(std::size_t index = 0; index < SampleCount; ++index) {
      double number = static_cast<double>(randomNumberDistribution(randomNumberGenerator));

      char buffer[325];
      char *end = FormatFloat(buffer, number);
      std::string formatted(buffer, end);
      localizeDecimalPoint(formatted);

      double actual = std::strtod(formatted.c_str(), &end);
      double expected = number;
      EXPECT_EQ(actual, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(NumberFormatterTest, LargeDoublePrecisionFloatingPointValuesCanBePrinted) {
    std::mt19937_64 randomNumberGenerator;
    std::uniform_real_distribution<double> randomNumberDistribution(
      std::numeric_limits<double>::lowest() / 2.1, std::numeric_limits<double>::max() / 2.1
    );

    for(std::size_t index = 0; index < SampleCount; ++index) {
      double number = static_cast<float>(randomNumberDistribution(randomNumberGenerator));

      char buffer[325];
      char *end = FormatFloat(buffer, number);
      std::string formatted(buffer, end);
      localizeDecimalPoint(formatted);

      double actual = std::strtod(formatted.c_str(), &end);
      double expected = number;
      EXPECT_DOUBLE_EQ(actual, expected);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
