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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/StringMatcher.h"

#include <cmath>
#include <clocale>

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

}}} // namespace Nuclex::Support::Text
