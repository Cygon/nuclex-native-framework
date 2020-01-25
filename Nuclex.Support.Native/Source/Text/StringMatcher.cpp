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

#include "Utf8/checked.h"
#include "Utf8/unchecked.h"
#include "Utf8Fold/Utf8Fold.h"

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if 0 // This is a memento for my old, trusty Ascii wildcard checker. It's retired now.
  /// <summary>C-style function that checks if a string matches a wild card</summary>
  /// <param name="text">Text that will be checked against the wild card</param>
  /// <param name="wildcard">Wild card against which the text will be matched</param>
  /// <returns>True if the text matches the wild card, false otherwise</returns>
  bool matchWildcardAscii(const char *text, const char *wildcard) {
    if(!text) {
      return false;
    }

    for(; '*' ^ *wildcard; ++wildcard, ++text) {
      if(!*text) {
        return (!*wildcard);
      }

      if(::toupper(*text) ^ ::toupper(*wildcard) && '?' ^ *wildcard) {
        return false;
      }
    }

    while('*' == wildcard[1]) {
      wildcard++;
    }

    do {
      if(matchWildcardAscii(text, wildcard + 1)) {
        return true;
      }
    } while(*text++);

    return false;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string matches a wild card</summary>
  /// <param name="text">Text that will be checked against the wild card</param>
  /// <param name="wildcard">Wild card against which the text will be matched</param>
  /// <returns>True if the text matches the wild card, false otherwise</returns>
  bool matchWildcardUtf8(const char *text, const char *wildcard) {
    using Nuclex::ToFoldedLowercase;

    assert((text != nullptr) && u8"Text must not be a NULL pointer");

    std::uint32_t wildcardCodepoint;
    std::uint32_t textCodepoint;

    // Compare text with wildcard 1:1
    const char *textAtStart;
    for(;;) {
      wildcardCodepoint = utf8::unchecked::next(wildcard);
      textAtStart = text;
      textCodepoint = utf8::unchecked::next(text);

      // If the wildcard contains a star, leave this loop and stark skipping
      if(wildcardCodepoint == '*') {
        break;
      }

      // If the end of the text was reached and the wildcard wasn't on a star,
      // if we're also at the end of the wildcard it's a match
      if(textCodepoint == 0) {
        return (wildcardCodepoint == 0);
      } else if(wildcardCodepoint == 0) { // End of wildcard but still have text?
        return false;
      }

      // Otherwise, the text must match the wildcard character
      if(wildcardCodepoint != '?') {
        if(ToFoldedLowercase(textCodepoint) != ToFoldedLowercase(wildcardCodepoint)) {
          return false;
        }
      }
    }

    // If we encountered a star, first skip any redundant stars directly following
    const char *wildcardAfterStar;
    do {
      wildcardAfterStar = wildcard;
      wildcardCodepoint = utf8::unchecked::next(wildcard);
    } while(wildcardCodepoint == '*');

    // Then retry the wildcard match skipping any number of characters from text
    // (the star can match anything from zero to all characters)
    do {
      if(matchWildcardUtf8(textAtStart, wildcardAfterStar)) {
        return true;
      }

      textCodepoint = utf8::unchecked::next(textAtStart);
    } while(textCodepoint != 0);

    // No amount of skipping helped, there's not match
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string matches a wild card</summary>
  /// <param name="text">Text that will be checked against the wild card</param>
  /// <param name="wildcard">Wild card against which the text will be matched</param>
  /// <returns>True if the text matches the wild card, false otherwise</returns>
  bool matchWildcardUtf8CaseSensitive(const char *text, const char *wildcard) {
    using Nuclex::ToFoldedLowercase;

    assert((text != nullptr) && u8"Text must not be a NULL pointer");

    std::uint32_t wildcardCodepoint;
    std::uint32_t textCodepoint;

    // Compare text with wildcard 1:1
    const char *textAtStart;
    for(;;) {
      wildcardCodepoint = utf8::unchecked::next(wildcard);
      textAtStart = text;
      textCodepoint = utf8::unchecked::next(text);

      // If the wildcard contains a star, leave this loop and stark skipping
      if(wildcardCodepoint == '*') {
        break;
      }

      // If the end of the text was reached and the wildcard wasn't on a star,
      // if we're also at the end of the wildcard it's a match
      if(textCodepoint == 0) {
        return (wildcardCodepoint == 0);
      } else if(wildcardCodepoint == 0) { // End of wildcard but still have text?
        return false;
      }

      // Otherwise, the text must match the wildcard character
      if(wildcardCodepoint != '?') {
        if(textCodepoint != wildcardCodepoint) {
          return false;
        }
      }
    }

    const char *wildcardAfterStar;
    do {
      wildcardAfterStar = wildcard;
      wildcardCodepoint = utf8::unchecked::next(wildcard);
    } while(wildcardCodepoint == '*');

    do {
      if(matchWildcardUtf8(textAtStart, wildcardAfterStar)) {
        return true;
      }

      textCodepoint = utf8::unchecked::next(textAtStart);
    } while(textCodepoint != 0);

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  bool StringMatcher::AreEqual(
    const std::string &left, const std::string &right, bool caseSensitive /* = false */
  ) {
    if(caseSensitive) {
      return (left == right);
    } else {
      return (ToFoldedLowercase(left) == ToFoldedLowercase(right));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool StringMatcher::FitsWildcard(
    const std::string &text, const std::string &wildcard, bool caseSensitive /* = false */
  ) {
    if(caseSensitive) {
      return matchWildcardUtf8CaseSensitive(text.c_str(), wildcard.c_str());
    } else {
      return matchWildcardUtf8(text.c_str(), wildcard.c_str());
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
