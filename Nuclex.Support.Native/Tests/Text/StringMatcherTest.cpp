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

#include "Nuclex/Support/Text/StringMatcher.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, ComparisonDefaultsToCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::AreEqual(u8"Hello", u8"hello"));
    EXPECT_TRUE(StringMatcher::AreEqual(u8"hello", u8"hello"));
    EXPECT_TRUE(StringMatcher::AreEqual(u8"Ünicøde", u8"üNICØDE"));
    EXPECT_TRUE(StringMatcher::AreEqual(u8"ünicøde", u8"ünicøde"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseSensitiveComparisonIsPossible) {
    EXPECT_FALSE(StringMatcher::AreEqual(u8"Hello", u8"hello", true));
    EXPECT_TRUE(StringMatcher::AreEqual(u8"hello", u8"hello", true));
    EXPECT_FALSE(StringMatcher::AreEqual(u8"Ünicøde", u8"ünicØde", true));
    EXPECT_FALSE(StringMatcher::AreEqual(u8"ÜNICØDE", u8"üNICøDE", true));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckForContainmentCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"hello"));
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"world"));

    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"hellø"));
    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"wørld"));

    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"h"));
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"w"));

    EXPECT_FALSE(StringMatcher::Contains(u8"H", u8"hello"));
    EXPECT_FALSE(StringMatcher::Contains(u8"W", u8"world"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, ContainmentCheckHandlesEmptyNeedleCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8""));
    EXPECT_TRUE(StringMatcher::Contains(u8"", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckForContainmentCaseSensitive) {
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"Hello", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"Hello World", u8"hello", true));
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"World", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"Hello World", u8"world", true));

    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"HellØ", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"HellØ WØrld", u8"hellø", true));
    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"WØrld", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"HellØ WØrld", u8"wørld", true));

    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"H", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"HellØ WØrld", u8"h", true));
    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"W", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"HellØ WØrld", u8"w", true));

    EXPECT_FALSE(StringMatcher::Contains(u8"H", u8"Hello", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"H", u8"hello", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"W", u8"World", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"W", u8"world", true));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, ContainmentCheckHandlesEmptyNeedleCaseSensitive) {
    EXPECT_TRUE(StringMatcher::Contains(u8"Hello World", u8"", true));
    EXPECT_TRUE(StringMatcher::Contains(u8"", u8"", true));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckIfStringStartsWithAnotherCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith(u8"Hello World", u8"Hello"));
    EXPECT_TRUE(StringMatcher::StartsWith(u8"Hello World", u8"hello"));
    EXPECT_FALSE(StringMatcher::StartsWith(u8"Hello World", u8"World"));

    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"HellØ"));
    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"hellø"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StartsWithHandlesEmptyNeedleCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith(u8"Hello World", u8""));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanCheckIfStringStartsWithAnotherCaseSensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith(u8"Hello World", u8"Hello", true));
    EXPECT_FALSE(StringMatcher::StartsWith(u8"Hello World", u8"hello", true));
    EXPECT_FALSE(StringMatcher::StartsWith(u8"Hello World", u8"World", true));

    EXPECT_TRUE(StringMatcher::Contains(u8"HellØ WØrld", u8"HellØ", true));
    EXPECT_FALSE(StringMatcher::Contains(u8"HellØ WØrld", u8"hellø", true));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, StartsWithHandlesEmptyNeedleCaseSensitive) {
    EXPECT_TRUE(StringMatcher::StartsWith(u8"Hello World", u8"", true));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, WilcardMatchDefaultsToCaseInsensitive) {
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"Hello World", u8"hello world"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HellØ WØrld", u8"hellø wørld"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, WildcardMatchCanBeCaseSensitive) {
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"Hello World", u8"hello world", true));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"HellØ WØrld", u8"hellø wørld", true));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanMatchAsciiStringToWildcard) {
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hello World"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", ""));
    EXPECT_TRUE(StringMatcher::FitsWildcard("", ""));
    EXPECT_FALSE(StringMatcher::FitsWildcard("", "Hello World"));

    EXPECT_TRUE(StringMatcher::FitsWildcard("", "*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "He*o World"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hell*o World"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "*"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", "W*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "*W*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hello World*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "*Hello World"));
    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hello***World"));

    EXPECT_TRUE(StringMatcher::FitsWildcard("Hello World", "Hell? W?rld"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", "?Hello World"));
    EXPECT_FALSE(StringMatcher::FitsWildcard("Hello World", "Hello World?"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CanMatchUtf8StringToWildcard) {
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"He*ø Wørld"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hell*ø Wørld"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"*"));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"DLRØW ØLLEH", u8"ø*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"*ø*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hellø Wørld*"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"*Hellø Wørld"));
    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hellø***Wørld"));

    EXPECT_TRUE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"H?llø Wør?d"));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"?Hellø Wørld"));
    EXPECT_FALSE(StringMatcher::FitsWildcard(u8"HELLØ WØRLD", u8"Hellø Wørld?"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseInsensitiveStringHashWorks) {
    CaseInsensitiveUtf8Hash hasher;
    std::size_t hash1 = hasher(u8"Hellø Wørld This is a test for the hashing method");
    std::size_t hash2 = hasher(u8"Hellø Wørld This is another test for the hashing method");
    std::size_t hash3 = hasher(u8"HELLØ WØRLD This is a test for the hashing method");

    EXPECT_EQ(hash1, hash3);
    EXPECT_NE(hash1, hash2);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseInsensitiveStringEqualsToWorks) {
    CaseInsensitiveUtf8EqualTo equals;

    EXPECT_TRUE(equals(u8"Hello", u8"hello"));
    EXPECT_TRUE(equals(u8"hello", u8"hello"));
    EXPECT_TRUE(equals(u8"Ünicøde", u8"üNICØDE"));
    EXPECT_TRUE(equals(u8"ünicøde", u8"ünicøde"));
    EXPECT_FALSE(equals(u8"hello", u8"olleh"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(StringMatcherTest, CaseInsensitiveStringLessWorks) {
    //std::less<std::string> lesser;
    CaseInsensitiveUtf8Less lesser;

    EXPECT_TRUE(lesser(u8"a", u8"b"));
    EXPECT_FALSE(lesser(u8"b", u8"b"));
    EXPECT_TRUE(lesser(u8"a9999", u8"b0000"));
    EXPECT_TRUE(lesser(u8"a9999", u8"b0"));
    EXPECT_TRUE(lesser(u8"a", u8"aa"));

    // Neither is less because in lowercase, they're identical
    EXPECT_FALSE(lesser(u8"Ünicøde", u8"üNICØDE"));
    EXPECT_FALSE(lesser(u8"üNICØDE", u8"Ünicøde"));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
