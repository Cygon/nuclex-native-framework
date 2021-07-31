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

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanDetectSingleSingleByteWhitespace) {
    EXPECT_TRUE(ParserHelper::IsWhitespace(std::uint8_t(' ')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(std::uint8_t('\t')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(std::uint8_t('\r')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(std::uint8_t('\n')));

    EXPECT_FALSE(ParserHelper::IsWhitespace(std::uint8_t('a')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(std::uint8_t('?')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(std::uint8_t('\'')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(std::uint8_t(0)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanDetectUtf32Whitespace) {
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(U' ')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(U'\t')));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(0x00a0)));
    EXPECT_TRUE(ParserHelper::IsWhitespace(char32_t(0x2003)));

    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(U'a')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(U'Ø')));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(0x200b)));
    EXPECT_FALSE(ParserHelper::IsWhitespace(char32_t(0)));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanSkipWhitespaces) {
    const char text[] = u8"\t Hellø Ünicøde Wórld ";
    const std::uint8_t *start = reinterpret_cast<const std::uint8_t *>(text);
    const std::uint8_t *end = start + sizeof(text);

    // Beginning w/multiple whitespaces
    {
      const std::uint8_t *current = start;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current, start + 2);
    }

    // On letter
    {
      const std::uint8_t *current = start + 3;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current, start + 3);
    }

    // Inside two-byte encoded code point
    {
      const std::uint8_t *current = start + 21;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current, start + 21);
    }

    // On last character
    {
      const std::uint8_t *current = start + 25;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current, start + 26);
    }

    // Past last character
    {
      const std::uint8_t *current = start + 26;
      ParserHelper::SkipWhitespace(current, end);
      EXPECT_EQ(current, start + 26);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ParserHelperTest, CanParseUInt32) {
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
