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

#include "Nuclex/Support/Text/LexicalAppend.h"

#include <limits> // for std::numeric_limits

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the n-th power of 10</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  template<typename TResult>
  inline TResult Pow10(std::size_t power) = delete;

  /// <summary>Calculates the n-th power of 10 as a 32 bit integer</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  template<>
  inline std::uint16_t Pow10<std::uint16_t>(std::size_t power) {
    static const std::uint16_t powersOfTen[] = { 1U, 10U, 100U, 1000U, 10000U };
    return powersOfTen[power];
  }

  /// <summary>Calculates the n-th power of 10 as a 32 bit integer</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  template<>
  inline std::uint32_t Pow10<std::uint32_t>(std::size_t power) {
    static const std::uint32_t powersOfTen[] = {
      1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U
    };
    return powersOfTen[power];
  }

  /// <summary>Calculates the n-th power of 10 as a 64 bit integer</summary>
  /// <param name="power">What power of 10 will be calculated</param>
  /// <returns>10 to the power of the specified value</returns>
  /// <remarks>
  ///   Doing this with std::pow() will start to yield imprecise results at the higher
  ///   ranges of 64 bit integers (at least with fast math enabled), this integer-only
  ///   version of the method gives an accurate result in all cases
  /// </remarks>
  template<>
  inline std::uint64_t Pow10<std::uint64_t>(std::size_t power) {
    static const std::uint64_t powersOfTen[20] = {
      1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL, 100000ULL, 1000000ULL, 10000000ULL,
      100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL, 1000000000000ULL,
      10000000000000ULL, 100000000000000ULL, 1000000000000000ULL, 10000000000000000ULL,
      100000000000000000ULL, 1000000000000000000ULL, 10000000000000000000ULL
    };
    return powersOfTen[power];
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendBooleanToString) {
    std::string trueString(u8"is ");
    lexical_append(trueString, true);
    EXPECT_EQ(trueString.length(), 7U);
    EXPECT_EQ(trueString, u8"is true");

    std::string falseString(u8"might be ");
    lexical_append(falseString, false);
    EXPECT_EQ(falseString.length(), 14U);
    EXPECT_EQ(falseString, u8"might be false");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendBooleanToCharArray) {
    char characters[7] = { 0 };

    characters[0] = 121;
    characters[5] = 122;
    EXPECT_EQ(lexical_append(characters + 1, 4U, true), 4U);
    EXPECT_EQ(characters[0], 121);
    EXPECT_EQ(characters[1], 't');
    EXPECT_EQ(characters[2], 'r');
    EXPECT_EQ(characters[3], 'u');
    EXPECT_EQ(characters[4], 'e');
    EXPECT_EQ(characters[5], 122);

    characters[0] = 122;
    characters[6] = 123;
    EXPECT_EQ(lexical_append(characters + 1, 5U, false), 5U);
    EXPECT_EQ(characters[0], 122);
    EXPECT_EQ(characters[1], 'f');
    EXPECT_EQ(characters[2], 'a');
    EXPECT_EQ(characters[3], 'l');
    EXPECT_EQ(characters[4], 's');
    EXPECT_EQ(characters[5], 'e');
    EXPECT_EQ(characters[6], 123);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForBoolean) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, true), 4U);
    EXPECT_EQ(lexical_append(characters, 1U, false), 5U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendCharactersToString) {
    const char *appended = u8"Hello World";

    std::string messageString("Hello Sky, ");
    lexical_append(messageString, appended);
    EXPECT_EQ(messageString.length(), 22U);
    EXPECT_EQ(messageString, u8"Hello Sky, Hello World");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendCharactersToCharArray) {
    const char *appended = u8"Hello World";
    char characters[14] = { 0 };

    characters[0] = 123;
    characters[12] = 124;
    EXPECT_EQ(lexical_append(characters + 1, 11U, appended), 11U);

    EXPECT_EQ(characters[0], 123);
    for(std::size_t index = 0; index < 11; ++index) {
      EXPECT_EQ(characters[index + 1], appended[index]);
    }
    EXPECT_EQ(characters[12], 124);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForCharacters) {
    const char *appended = u8"Hello World";
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, appended), 11U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendNullPointerToString) {
    const char *appended = nullptr;

    std::string resultString(u8"The appended part is a ");
    lexical_append(resultString, appended);
    EXPECT_EQ(resultString.length(), 32U);
    EXPECT_EQ(resultString, u8"The appended part is a <nullptr>");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendNullPointerToCharArray) {
    const char *appended = nullptr;
    char characters[14] = { 0 };

    characters[0] = 124;
    characters[10] = 125;
    EXPECT_EQ(lexical_append(characters + 1, 9U, appended), 9U);

    const char *expected = u8"<nullptr>";

    EXPECT_EQ(characters[0], 124);
    for(std::size_t index = 0; index < 9; ++index) {
      EXPECT_EQ(characters[index + 1], expected[index]);
    }
    EXPECT_EQ(characters[10], 125);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForNullPointer) {
    const char *appended = nullptr;
    char characters[1] = { 0 };
    EXPECT_EQ(lexical_append(characters, 1U, appended), 9U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt8ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::uint8_t(234));
    EXPECT_EQ(resultString.length(), 16U);
    EXPECT_EQ(resultString, u8"Value equals 234");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt8ToCharacterArray) {
    char characters[5] = { 0 };

    characters[0] = 125;
    characters[4] = 126;
    EXPECT_EQ(lexical_append(characters + 1, 3U, std::uint8_t(234)), 3U);
    EXPECT_EQ(characters[0], 125);
    EXPECT_EQ(characters[1], '2');
    EXPECT_EQ(characters[2], '3');
    EXPECT_EQ(characters[3], '4');
    EXPECT_EQ(characters[4], 126);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt8) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::uint8_t(9)), 1U);
    EXPECT_EQ(lexical_append(characters, 1U, std::uint8_t(10)), 2U);
    EXPECT_EQ(lexical_append(characters, 1U, std::uint8_t(99)), 2U);
    EXPECT_EQ(lexical_append(characters, 1U, std::uint8_t(100)), 3U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt8ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::int8_t(-123));
    EXPECT_EQ(resultString.length(), 17U);
    EXPECT_EQ(resultString, u8"Value equals -123");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt8ToCharacterArray) {
    char characters[6] = { 0 };

    characters[0] = 126;
    characters[5] = 127;
    EXPECT_EQ(lexical_append(characters + 1, 4U, std::int8_t(-123)), 4U);
    EXPECT_EQ(characters[0], 126);
    EXPECT_EQ(characters[1], '-');
    EXPECT_EQ(characters[2], '1');
    EXPECT_EQ(characters[3], '2');
    EXPECT_EQ(characters[4], '3');
    EXPECT_EQ(characters[5], 127);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt8) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::int8_t(-9)), 2U);
    EXPECT_EQ(lexical_append(characters, 1U, std::int8_t(-10)), 3U);
    EXPECT_EQ(lexical_append(characters, 1U, std::int8_t(-99)), 3U);
    EXPECT_EQ(lexical_append(characters, 1U, std::int8_t(-100)), 4U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt16ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::uint16_t(56789));
    EXPECT_EQ(resultString.length(), 18U);
    EXPECT_EQ(resultString, u8"Value equals 56789");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt16ToCharacterArray) {
    char characters[7] = { 0 };

    characters[0] = 120;
    characters[6] = 121;
    EXPECT_EQ(lexical_append(characters + 1, 5U, std::uint16_t(56789)), 5U);
    EXPECT_EQ(characters[0], 120);
    EXPECT_EQ(characters[1], '5');
    EXPECT_EQ(characters[2], '6');
    EXPECT_EQ(characters[3], '7');
    EXPECT_EQ(characters[4], '8');
    EXPECT_EQ(characters[5], '9');
    EXPECT_EQ(characters[6], 121);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt16) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::uint16_t(0)), 1U);

    EXPECT_EQ(lexical_append(characters, 1U, std::uint16_t(1)), 1U);
    for(std::size_t log10 = 1; log10 < 5; ++log10) {
      std::uint16_t nextHigher = Pow10<std::uint16_t>(log10);
      std::uint16_t nextLower = nextHigher - 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt16ToString) {
    std::string resultString(u8"Value equals ");
    lexical_append(resultString, std::int16_t(-23456));
    EXPECT_EQ(resultString.length(), 19U);
    EXPECT_EQ(resultString, u8"Value equals -23456");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt16ToCharacterArray) {
    char characters[8] = { 0 };

    characters[0] = 121;
    characters[7] = 122;
    EXPECT_EQ(lexical_append(characters + 1, 6U, std::int16_t(-23456)), 6U);
    EXPECT_EQ(characters[0], 121);
    EXPECT_EQ(characters[1], '-');
    EXPECT_EQ(characters[2], '2');
    EXPECT_EQ(characters[3], '3');
    EXPECT_EQ(characters[4], '4');
    EXPECT_EQ(characters[5], '5');
    EXPECT_EQ(characters[6], '6');
    EXPECT_EQ(characters[7], 122);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt16) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::int16_t(0)), 1U);

    EXPECT_EQ(lexical_append(characters, 1U, std::int16_t(1)), 1U);
    for(std::size_t log10 = 1; log10 < 5; ++log10) {
      std::int16_t nextHigher = static_cast<std::int16_t>(Pow10<std::uint16_t>(log10));
      std::int16_t nextLower = nextHigher - 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }

    EXPECT_EQ(lexical_append(characters, 1U, std::int16_t(-1)), 2U);
    for(std::size_t log10 = 1; log10 < 5; ++log10) {
      std::int16_t nextLower = -static_cast<std::int16_t>(Pow10<std::uint16_t>(log10));
      std::int16_t nextHigher = nextLower + 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10 + 2);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt32ToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, std::uint32_t(0));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, std::uint32_t(1234567890));
    EXPECT_EQ(resultString.length(), 24U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt32ToCharacterArray) {
    char characters[12] = { 0 };

    {
      characters[0] = 122;
      characters[2] = 123;
      EXPECT_EQ(lexical_append(characters + 1, 1U, std::uint32_t(0)), 1U);
      EXPECT_EQ(characters[0], 122);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 123);
    }

    {
      characters[0] = 123;
      characters[11] = 124;
      EXPECT_EQ(lexical_append(characters + 1, 10U, std::uint32_t(1234567890)), 10U);

      const char *expected = u8"1234567890";

      EXPECT_EQ(characters[0], 123);
      for(std::size_t index = 0; index < 10; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[11], 124);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt32) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::uint32_t(0)), 1U);

    EXPECT_EQ(lexical_append(characters, 1U, std::uint32_t(1)), 1U);
    for(std::size_t log10 = 1; log10 < 10; ++log10) {
      std::uint32_t nextHigher = Pow10<std::uint32_t>(log10);
      std::uint32_t nextLower = nextHigher - 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt32ToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, std::int32_t(0));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, std::int32_t(1234567890));
    EXPECT_EQ(resultString.length(), 24U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890");

    lexical_append(resultString, std::int32_t(-1234567890));
    EXPECT_EQ(resultString.length(), 35U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890-1234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt32ToCharacterArray) {
    char characters[13] = { 0 };

    {
      characters[0] = 124;
      characters[2] = 125;
      EXPECT_EQ(lexical_append(characters + 1, 1U, std::int32_t(0)), 1U);
      EXPECT_EQ(characters[0], 124);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 125);
    }

    {
      characters[0] = 125;
      characters[11] = 126;
      EXPECT_EQ(lexical_append(characters + 1, 10U, std::int32_t(1234567890)), 10U);

      const char *expected = u8"1234567890";

      EXPECT_EQ(characters[0], 125);
      for(std::size_t index = 0; index < 10; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[11], 126);
    }

    {
      characters[0] = 126;
      characters[12] = 127;
      EXPECT_EQ(lexical_append(characters + 1, 11U, std::int32_t(-1234567890)), 11U);

      const char *expected = u8"-1234567890";

      EXPECT_EQ(characters[0], 126);
      for(std::size_t index = 0; index < 11; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[12], 127);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt32) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::int32_t(0)), 1U);

    EXPECT_EQ(lexical_append(characters, 1U, std::int32_t(1)), 1U);
    for(std::size_t log10 = 1; log10 < 10; ++log10) {
      std::int32_t nextHigher = static_cast<std::int32_t>(Pow10<std::uint32_t>(log10));
      std::int32_t nextLower = nextHigher - 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }

    EXPECT_EQ(lexical_append(characters, 1U, std::int32_t(-1)), 2U);
    for(std::size_t log10 = 1; log10 < 10; ++log10) {
      std::int32_t nextLower = -static_cast<std::int32_t>(Pow10<std::uint32_t>(log10));
      std::int32_t nextHigher = nextLower + 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10 + 2);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt64ToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, std::uint64_t(0));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, std::uint64_t(12345678901234567890U));
    EXPECT_EQ(resultString.length(), 34U);
    EXPECT_EQ(resultString, u8"Value equals 012345678901234567890");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendUInt64ToCharacterArray) {
    char characters[22] = { 0 };

    {
      characters[0] = 120;
      characters[2] = 121;
      std::size_t characterCount = lexical_append(characters + 1, 1U, std::uint64_t(0));
      EXPECT_EQ(characterCount, 1U);

      EXPECT_EQ(characters[0], 120);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 121);
    }

    {
      characters[0] = 121;
      characters[21] = 122;
      std::size_t characterCount = lexical_append(
        characters + 1, 20U, std::uint64_t(12345678901234567890U)
      );
      EXPECT_EQ(characterCount, 20U);

      const char *expected = u8"12345678901234567890";

      EXPECT_EQ(characters[0], 121);
      for(std::size_t index = 0; index < 20; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[21], 122);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForUInt64) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::uint64_t(0)), 1U);

    EXPECT_EQ(lexical_append(characters, 1U, std::uint64_t(1)), 1U);
    for(std::size_t log10 = 1; log10 < 20; ++log10) {
      std::uint64_t nextHigher = Pow10<std::uint64_t>(log10);
      std::uint64_t nextLower = nextHigher - 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt64ToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, std::int64_t(0));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, std::int64_t(1234567890123456789));
    EXPECT_EQ(resultString.length(), 33U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890123456789");

    lexical_append(resultString, std::int64_t(-1234567890123456789));
    EXPECT_EQ(resultString.length(), 53U);
    EXPECT_EQ(resultString, u8"Value equals 01234567890123456789-1234567890123456789");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendInt64ToCharacterArray) {
    char characters[23] = { 0 };

    {
      characters[0] = 122;
      characters[2] = 123;
      EXPECT_EQ(lexical_append(characters + 1, 1U, std::int64_t(0)), 1U);
      EXPECT_EQ(characters[0], 122);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 123);
    }

    {
      characters[0] = 123;
      characters[21] = 124;
      EXPECT_EQ(lexical_append(characters + 1, 19U, std::int64_t(1234567890123456789)), 19U);

      const char *expected = u8"1234567890123456789";

      EXPECT_EQ(characters[0], 123);
      for(std::size_t index = 0; index < 19; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[21], 124);
    }

    {
      characters[0] = 125;
      characters[22] = 126;
      EXPECT_EQ(lexical_append(characters + 1, 20U, std::int64_t(-1234567890123456789)), 20U);

      const char *expected = u8"-1234567890123456789";

      EXPECT_EQ(characters[0], 125);
      for(std::size_t index = 0; index < 20; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[22], 126);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForInt64) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, std::int64_t(0)), 1U);

    EXPECT_EQ(lexical_append(characters, 1U, std::int64_t(1)), 1U);
    for(std::size_t log10 = 1; log10 < 19; ++log10) {
      std::int64_t nextHigher = static_cast<std::int64_t>(Pow10<std::uint64_t>(log10));
      std::int64_t nextLower = nextHigher - 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }

    EXPECT_EQ(lexical_append(characters, 1U, std::int64_t(-1)), 2U);
    for(std::size_t log10 = 1; log10 < 19; ++log10) {
      std::int64_t nextLower = -static_cast<std::int64_t>(Pow10<std::uint64_t>(log10));
      std::int64_t nextHigher = nextLower + 1;
      EXPECT_EQ(lexical_append(characters, 1U, nextLower), log10 + 2);
      EXPECT_EQ(lexical_append(characters, 1U, nextHigher), log10 + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendFloatToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, float(0.0f));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, float(123.456f));
    EXPECT_EQ(resultString.length(), 21U);
    EXPECT_EQ(resultString, u8"Value equals 0123.456");

    lexical_append(resultString, float(-123.456f));
    EXPECT_EQ(resultString.length(), 29U);
    EXPECT_EQ(resultString, u8"Value equals 0123.456-123.456");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendFloatToCharacterArray) {
    char characters[10] = { 0 };

    {
      characters[0] = 126;
      characters[2] = 127;
      EXPECT_EQ(lexical_append(characters + 1, 1U, float(0.0f)), 1U);
      EXPECT_EQ(characters[0], 126);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 127);
    }

    {
      characters[0] = 120;
      characters[8] = 121;
      EXPECT_EQ(lexical_append(characters + 1, 7U, float(123.456f)), 7U);

      const char *expected = u8"123.456";

      EXPECT_EQ(characters[0], 120);
      for(std::size_t index = 0; index < 7; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[8], 121);
    }

    {
      characters[0] = 121;
      characters[9] = 122;
      EXPECT_EQ(lexical_append(characters + 1, 8U, float(-123.456f)), 8U);

      const char *expected = u8"-123.456";

      EXPECT_EQ(characters[0], 121);
      for(std::size_t index = 0; index < 8; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[9], 122);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForFloat) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, float(0.0f)), 1U);
    EXPECT_EQ(lexical_append(characters, 1U, float(123.456f)), 7U);
    EXPECT_EQ(lexical_append(characters, 1U, float(-123.456f)), 8U);
  }


  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, BufferFitsExtremeFloatLengths) {
    std::string resultString(u8"Longest float: ");

    // Doubles can print as up to 64 decimal digits, so test these edge cases
    lexical_append(resultString, std::numeric_limits<float>::min());
    lexical_append(resultString, std::numeric_limits<float>::max());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendDoubleToString) {
    std::string resultString(u8"Value equals ");

    lexical_append(resultString, double(0.0));
    EXPECT_EQ(resultString.length(), 14U);
    EXPECT_EQ(resultString, u8"Value equals 0");

    lexical_append(resultString, double(12345.06789));
    EXPECT_EQ(resultString.length(), 25U);
    EXPECT_EQ(resultString, u8"Value equals 012345.06789");

    lexical_append(resultString, double(-12345.06789));
    EXPECT_EQ(resultString.length(), 37U);
    EXPECT_EQ(resultString, u8"Value equals 012345.06789-12345.06789");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, CanAppendDoubleToCharacterArray) {
    char characters[14] = { 0 };

    {
      characters[0] = 122;
      characters[2] = 123;
      EXPECT_EQ(lexical_append(characters + 1, 1U, double(0.0)), 1U);
      EXPECT_EQ(characters[0], 122);
      EXPECT_EQ(characters[1], '0');
      EXPECT_EQ(characters[2], 123);
    }

    {
      characters[0] = 123;
      characters[12] = 124;
      EXPECT_EQ(lexical_append(characters + 1, 11U, double(12345.06789)), 11U);

      const char *expected = u8"12345.06789";

      EXPECT_EQ(characters[0], 123);
      for(std::size_t index = 0; index < 11; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[12], 124);
    }

    {
      characters[0] = 124;
      characters[13] = 125;
      EXPECT_EQ(lexical_append(characters + 1, 12U, double(-12345.06789)), 12U);

      const char *expected = u8"-12345.06789";

      EXPECT_EQ(characters[0], 124);
      for(std::size_t index = 0; index < 12; ++index) {
        EXPECT_EQ(characters[index + 1], expected[index]);
      }
      EXPECT_EQ(characters[13], 125);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, ReturnsNeededByteCountForDouble) {
    char characters[1] = { 0 };

    EXPECT_EQ(lexical_append(characters, 1U, double(0.0)), 1U);
    EXPECT_EQ(lexical_append(characters, 1U, double(12345.06789)), 11U);
    EXPECT_EQ(lexical_append(characters, 1U, double(-12345.06789)), 12U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LexicalAppendTest, BufferFitsExtremeDoubleLengths) {
    std::string resultString(u8"Longest double: ");

    // Doubles can print as up to 256 decimal digits, so test these edge cases
    lexical_append(resultString, std::numeric_limits<double>::min());
    lexical_append(resultString, std::numeric_limits<double>::max());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
