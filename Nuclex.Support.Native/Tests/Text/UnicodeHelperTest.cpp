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

#include "Nuclex/Support/Text/UnicodeHelper.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, DetectsInvalidCodePoints) {
    char32_t validCodePoint = U'Ø';
    EXPECT_TRUE(UnicodeHelper::IsValidCodePoint(validCodePoint));

    char32_t invalidCodePoint = 1114111; // the first invalid code point
    EXPECT_FALSE(UnicodeHelper::IsValidCodePoint(invalidCodePoint));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, CalculatesUtf8CharacterCount) {
    char32_t asciiCodePoint = U'x';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(asciiCodePoint), 1U);

    char32_t centCodePoint = U'¢';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(centCodePoint), 2U);

    char32_t euroCodePoint = U'€';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(euroCodePoint), 3U);

    char32_t gothicCodePoint = U'𐍈';
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(gothicCodePoint), 4U);

    char32_t invalidCodePoint = 1114111;
    EXPECT_EQ(UnicodeHelper::CountUtf8Characters(invalidCodePoint), std::size_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, CalculatesUtf16CharacterCount) {
    char32_t asciiCodePoint = U'x';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(asciiCodePoint), 1U);

    char32_t centCodePoint = U'¢';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(centCodePoint), 1U);

    char32_t euroCodePoint = U'€';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(euroCodePoint), 1U);

    char32_t gothicCodePoint = U'𐍈';
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(gothicCodePoint), 2U);

    char32_t surrogateCodePoint = char32_t(0xDD00); // surrogate range
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(surrogateCodePoint), std::size_t(-1));

    char32_t invalidCodePoint = 1114111;
    EXPECT_EQ(UnicodeHelper::CountUtf16Characters(invalidCodePoint), std::size_t(-1));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, TellsSequenceLengthFromUtf8LeadCharacter) {
    using my_char8_t = UnicodeHelper::char8_t;

    const char *ascii = u8"A";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(*reinterpret_cast<const my_char8_t *>(ascii)), 1U
    );

    const char *cents = u8"¢";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(*reinterpret_cast<const my_char8_t *>(cents)), 2U
    );

    const char *euros = u8"€";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(*reinterpret_cast<const my_char8_t *>(euros)), 3U
    );

    const char *gothic = u8"𐍈";
    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(*reinterpret_cast<const my_char8_t *>(gothic)), 4U
    );

    EXPECT_EQ(
      UnicodeHelper::GetSequenceLength(UnicodeHelper::char8_t(0x80)), std::size_t(-1)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadsCodePointFromUtf8) {
    using my_char8_t = UnicodeHelper::char8_t;

    {
      const char ascii[] = u8"A";
      const my_char8_t *start = reinterpret_cast<const my_char8_t *>(ascii);
      const my_char8_t *end = reinterpret_cast<const my_char8_t *>(ascii) + sizeof(ascii);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'A');
      EXPECT_EQ(start, reinterpret_cast<const my_char8_t *>(ascii) + 1);
    }

    {
      const char cents[] = u8"¢";
      const my_char8_t *start = reinterpret_cast<const my_char8_t *>(cents);
      const my_char8_t *end = reinterpret_cast<const my_char8_t *>(cents) + sizeof(cents);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'¢');
      EXPECT_EQ(start, reinterpret_cast<const my_char8_t *>(cents) + 2);
    }

    {
      const char euros[] = u8"€";
      const my_char8_t *start = reinterpret_cast<const my_char8_t *>(euros);
      const my_char8_t *end = reinterpret_cast<const my_char8_t *>(euros) + sizeof(euros);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'€');
      EXPECT_EQ(start, reinterpret_cast<const my_char8_t *>(euros) + 3);
    }

    {
      const char gothic[] = u8"𐍈";
      const my_char8_t *start = reinterpret_cast<const my_char8_t *>(gothic);
      const my_char8_t *end = reinterpret_cast<const my_char8_t *>(gothic) + sizeof(gothic);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'𐍈');
      EXPECT_EQ(start, reinterpret_cast<const my_char8_t *>(gothic) + 4);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadingInvalidCodePointFromUtf8Fails) {
    using my_char8_t = UnicodeHelper::char8_t;

    // Invalid second byte should be detected
    {
      char invalid[] = u8"𐍈";
      *reinterpret_cast<std::uint8_t *>(&invalid[1]) = 0xC0; // 0b11xxxxxx
      const my_char8_t *start = reinterpret_cast<const my_char8_t *>(invalid);
      const my_char8_t *end = reinterpret_cast<const my_char8_t *>(invalid) + sizeof(invalid);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, char32_t(-1));
      EXPECT_EQ(start, reinterpret_cast<const my_char8_t *>(invalid));
    }

    // Invalid length (5 bytes, possible by encoding, but always invalid since
    // it's either an out-of-range code point or an overlong code point).
    {
      char invalid[] = u8"𐍈";
      *reinterpret_cast<std::uint8_t *>(&invalid[0]) = 0xF8; // 0b11111000
      const my_char8_t *start = reinterpret_cast<const my_char8_t *>(invalid);
      const my_char8_t *end = reinterpret_cast<const my_char8_t *>(invalid) + sizeof(invalid);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, char32_t(-1));
      EXPECT_EQ(start, reinterpret_cast<const my_char8_t *>(invalid));
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadsCodePointFromUtf16) {
    {
      const char16_t ascii[] = u"A";
      const char16_t *start = reinterpret_cast<const char16_t *>(ascii);
      const char16_t *end = reinterpret_cast<const char16_t *>(ascii) + sizeof(ascii);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'A');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(ascii) + 1);
    }

    {
      const char16_t cent[] = u"¢";
      const char16_t *start = reinterpret_cast<const char16_t *>(cent);
      const char16_t *end = reinterpret_cast<const char16_t *>(cent) + sizeof(cent);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'¢');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(cent) + 1);
    }

    {
      const char16_t euro[] = u"€";
      const char16_t *start = reinterpret_cast<const char16_t *>(euro);
      const char16_t *end = reinterpret_cast<const char16_t *>(euro) + sizeof(euro);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'€');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(euro) + 1);
    }

    {
      const char16_t gothic[] = u"𐍈";
      const char16_t *start = reinterpret_cast<const char16_t *>(gothic);
      const char16_t *end = reinterpret_cast<const char16_t *>(gothic) + sizeof(gothic);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, U'𐍈');
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(gothic) + 2);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, ReadingInvalidCodePointFromUtf16Fails) {
    {
      char16_t gothic[] = u"𐍈";
      { // Flip lead/trail surrogates
        char16_t temp = gothic[0];
        gothic[0] = gothic[1];
        gothic[1] = temp;
      }
      const char16_t *start = reinterpret_cast<const char16_t *>(gothic);
      const char16_t *end = reinterpret_cast<const char16_t *>(gothic) + sizeof(gothic);
      char32_t codePoint = UnicodeHelper::ReadCodePoint(start, end);
      EXPECT_EQ(codePoint, char32_t(-1));
      EXPECT_EQ(start, reinterpret_cast<const char16_t *>(gothic));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, EncodesCodePointsToUtf8) {
    using my_char8_t = UnicodeHelper::char8_t;

    {
      my_char8_t ascii[4] = { 255, 255, 255, 255 };
      my_char8_t *start = reinterpret_cast<my_char8_t *>(ascii);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'A', start);
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<my_char8_t *>(ascii) + 1);
      EXPECT_EQ(ascii[0], u8'A');
    }

    {
      my_char8_t cent[4] = { 255, 255, 255, 255 };
      my_char8_t *start = reinterpret_cast<my_char8_t *>(cent);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'¢', start);
      EXPECT_EQ(count, 2U);
      EXPECT_EQ(start, reinterpret_cast<my_char8_t *>(cent) + 2);

      const my_char8_t expected[] = u8"¢";
      EXPECT_EQ(cent[0], expected[0]);
      EXPECT_EQ(cent[1], expected[1]);
    }

    {
      my_char8_t euro[4] = { 255, 255, 255, 255 };
      my_char8_t *start = reinterpret_cast<my_char8_t *>(euro);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'€', start);
      EXPECT_EQ(count, 3U);
      EXPECT_EQ(start, reinterpret_cast<my_char8_t *>(euro) + 3);

      const my_char8_t expected[] = u8"€";
      EXPECT_EQ(euro[0], expected[0]);
      EXPECT_EQ(euro[1], expected[1]);
      EXPECT_EQ(euro[2], expected[2]);
    }

    {
      my_char8_t gothic[4] = { 255, 255, 255, 255 };
      my_char8_t *start = reinterpret_cast<my_char8_t *>(gothic);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'𐍈', start);
      EXPECT_EQ(count, 4U);
      EXPECT_EQ(start, reinterpret_cast<my_char8_t *>(gothic) + 4);

      const my_char8_t expected[] = u8"𐍈";
      EXPECT_EQ(gothic[0], expected[0]);
      EXPECT_EQ(gothic[1], expected[1]);
      EXPECT_EQ(gothic[2], expected[2]);
      EXPECT_EQ(gothic[3], expected[3]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, EncodesCodePointsToUtf16) {
    {
      char16_t ascii[2] = { 65535, 65535};
      char16_t *start = reinterpret_cast<char16_t *>(ascii);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'A', start);
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(ascii) + 1);
      EXPECT_EQ(ascii[0], u'A');
    }

    {
      char16_t cent[4] = { 255, 255, 255, 255 };
      char16_t *start = reinterpret_cast<char16_t *>(cent);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'¢', start);
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(cent) + 1);

      const char16_t expected[] = u"¢";
      EXPECT_EQ(cent[0], expected[0]);
    }

    {
      char16_t euro[4] = { 255, 255, 255, 255 };
      char16_t *start = reinterpret_cast<char16_t *>(euro);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'€', start);
      EXPECT_EQ(count, 1U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(euro) + 1);

      const char16_t expected[] = u"€";
      EXPECT_EQ(euro[0], expected[0]);
    }

    {
      char16_t gothic[4] = { 255, 255, 255, 255 };
      char16_t *start = reinterpret_cast<char16_t *>(gothic);
      std::size_t count = UnicodeHelper::WriteCodePoint(U'𐍈', start);
      EXPECT_EQ(count, 2U);
      EXPECT_EQ(start, reinterpret_cast<char16_t *>(gothic) + 2);

      const char16_t expected[] = u"𐍈";
      EXPECT_EQ(gothic[0], expected[0]);
      EXPECT_EQ(gothic[1], expected[1]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(UnicodeHelperTest, CaseFoldingAllowsCaseInsensitiveComparison) {
    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'A'),
      UnicodeHelper::ToFoldedLowercase(U'a')
    );

    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'Ā'),
      UnicodeHelper::ToFoldedLowercase(U'ā')
    );

    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'Ω'),
      UnicodeHelper::ToFoldedLowercase(U'ω')
    );

    EXPECT_EQ(
      UnicodeHelper::ToFoldedLowercase(U'𑢰'),
      UnicodeHelper::ToFoldedLowercase(U'𑣐')
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
