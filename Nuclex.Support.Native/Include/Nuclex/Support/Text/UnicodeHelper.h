#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_TEXT_UNICODEHELPER_H
#define NUCLEX_SUPPORT_TEXT_UNICODEHELPER_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
//#include <stdexcept> // for std::runtime_error
#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper methods for dealing with unicode and its different encodings</summary>
  /// <remarks>
  ///   <para>
  ///     Short overview of unicode: the &quot;unicode consortium&quot; has taken symbols from
  ///     all languages of the world and put them into a giant table. Said table is defined
  ///     with finite room for about 1,1 million symbols, but only some 140,000 symbols have
  ///     been filled so far. Nominally, the table is divided into 17 &quot;planes&quot; of
  ///     65,536 characters each, separating latin-based languages from asian ones and from
  ///     funny poop symbols but that's only important for font designers.
  ///   </para>
  ///   <para>
  ///     An index in the unicode table is called a &quot;code point&quot;. To store text using
  ///     unicode, each code point has to be stored. The easiest way to do that would be to just
  ///     save a stream of 32 bit integers indicating code points. And that's what UTF-32 is.
  ///     While simple to deal with, its downsides are wasted space and endian issues.
  ///   </para>
  ///   <para>
  ///     That's why UTF-8 became the global standard. It is a variable-length encoding where
  ///     the upper bits of the leading byte indicate the number of bytes that form one
  ///     code point. ASCII code points use only one UTF-8 byte (in fact, they map 1:1),
  ///     other latin letters and most asian ones use two bytes and only rarely does
  ///     a code point require 3 or 4 bytes in UTF-8.
  ///   </para>
  ///   <para>
  ///     UTF-16 combines the worst of both, endian issues and wasted space. So naturally
  ///     Microsoft used it for unicode in Windows. A code point is represented by one or two
  ///     16 bit integers, again using the leading integer's high bits to indicate whether
  ///     the code point is complete or formed together with the 16 bit integer that follows.
  ///   </para>
  ///   <para>
  ///     Now one last confusing thing: where I write that UTF-8 encodes unicode code points
  ///     as 1-4 bytes, UTF-16 as one or two 16 bit integers and UTF-32 as a 32 bit integer,
  ///     the correct term to use would be &quot;characters&quot;. That's why in C++ the new
  ///     data types are <code>char8_t</code>, <code>char16_t</code> and <code>char32_t</code>.
  ///     So &quot;character&quot; has been defined to mean &quot;encoding unit&quot; and is
  ///     not always enough to represent a full letter (aka code point).
  ///   </para>
  ///   <para>
  ///     A series of characters encoding a unicode code point is called a sequence.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE UnicodeHelper {

    /// <summary>UTF-8 character of which either 1, 2, 3 or 4 specify one codepoint</summary>
    /// <remarks>
    ///   Under C++20, this will be a native type like char16_t and char32_t. There will also
    ///   be an std::u8string using this character type to unambiguously indicate that
    ///   the contents of the string are supposed to be UTF-8 encoded.
    /// </remarks>
    public: typedef unsigned char char8_t;

    /// <summary>The symbol used to indicate a code point is invalid or corrupted</summary>
    public: static const constexpr char32_t ReplacementCodePoint = char32_t(0xFFFD);

    /// <summary>Checks whether the specified unicode code point is valid</summary>
    /// <param name="codePoint">Code point that will be checked</param>
    /// <returns>True if the code point is valid, false otherwise</returns>
    public: NUCLEX_SUPPORT_API static constexpr bool IsValidCodePoint(char32_t codePoint);

    /// <summary>
    ///   Returns the number of characters in a sequence by looking at the lead character
    /// </summary>
    /// <param name="leadCharacter">Lead character of the UTF-8 sequence</param>
    /// <returns>
    ///   The length of the sequence or <code>std::size_t(-1)</code> if the character
    ///   is not the lead character of a sequence (or is not valid UTF-8 at all).
    /// </returns>
    /// <remarks>
    ///   This method can be used to figure out if a character is the lead character, too.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t GetSequenceLength(
      char8_t leadCharacter
    );

    /// <summary>
    ///   Returns the number of characters in a sequence by looking at the lead character
    /// </summary>
    /// <param name="leadCharacter">Lead character of the UTF-16 sequence</param>
    /// <returns>
    ///   The length of the sequence or <code>std::size_t(-1)</code> if the character
    ///   is not the lead character of a sequence (or is not valid UTF-16 at all).
    /// </returns>
    /// <remarks>
    ///   This method can be used to figure out if a character is the lead character, too.
    ///   It doesn't do any big/little endian conversion. If you know the input is from
    ///   in the endianness opposite of the current platform, byte-swap each char16_t.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t GetSequenceLength(
      char16_t leadCharacter
    );

    /// <summary>
    ///   Counts the number of UTF-8 characters needed to represent a code point
    /// </summary>
    /// <param name="codePoint">
    ///   Code point for which the needed UTF-8 characters will be counted
    /// </param>
    /// <returns>The number of characters needed to encode the code point in UTF-8</returns>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t CountUtf8Characters(
      char32_t codePoint
    );

    /// <summary>
    ///   Counts the number of UTF-16 characters needed to represent a code point
    /// </summary>
    /// <param name="codePoint">
    ///   Code point for which the needed UTF-16 characters will be counted
    /// </param>
    /// <returns>The number of characters needed to encode the code point in UTF-16</returns>
    public: NUCLEX_SUPPORT_API static constexpr std::size_t CountUtf16Characters(
      char32_t codePoint
    );

    /// <summary>Reads a code point from a variable-length UTF-8 sequence</summary>
    /// <param name="current">
    ///   Address of the UTF-8 lead character, will be updated to the next lead
    ///   character if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-8 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-8 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      const char8_t *&current, const char8_t *end
    );

    /// <summary>Reads a code point from a variable-length UTF-16 sequence</summary>
    /// <param name="current">
    ///   Address of the UTF-16 lead character, will be updated to the next lead
    ///   character if the read succeeds.
    /// </param>
    /// <param name="end">Character at which the UTF-16 string ends</param>
    /// <returns>The unicode code point index, identical to UTF-32.</returns>
    /// <remarks>
    ///   If the end is reached or if the character is incomplete or invalid, this method
    ///   returns char32_t(-1) to indicate failure. You should check the position of your
    ///   read pointer before calling to distinguish between a normal end of the string and
    ///   bad UTF-16 data.
    /// </returns>
    public: NUCLEX_SUPPORT_API static char32_t ReadCodePoint(
      const char16_t *&current, const char16_t *end
    );

    /// <summary>Encodes the specified code point into UTF-8 characters</summary>
    /// <param name="codePoint">Code point that will be encoded as UTF-8</param>
    /// <param name="target">
    ///   Address at which the UTF-8 characters will be deposited. Needs to have at
    ///   least 4 bytes of usable space and will be moved to after the encoded characters
    /// </param>
    /// <returns>
    ///   The number of characters that have been encoded or std::size_t(-1) if
    ///   you specified an invalid code point.
    /// </returns>
    public: NUCLEX_SUPPORT_API static std::size_t WriteCodePoint(
      char32_t codePoint, char8_t *&target
    );

    /// <summary>Encodes the specified code point into UTF-16 characters</summary>
    /// <param name="codePoint">Code point that will be encoded as UTF-16</param>
    /// <param name="target">
    ///   Address at which the UTF-16 characters will be deposited. Needs to have at
    ///   least 4 bytes of usable space and will be moved to after the encoded characters
    /// </param>
    /// <returns>
    ///   The number of characters that have been encoded or std::size_t(-1) if
    ///   you specified an invalid code point.
    /// </returns>
    public: NUCLEX_SUPPORT_API static std::size_t WriteCodePoint(
      char32_t codePoint, char16_t *&target
    );

    /// <summary>Converts the specified Unicode code point to folded lowercase</summary>
    /// <param name="codePoint">
    ///   Unicode code point that will be converted to folded lowercase
    /// </param>
    /// <returns>The character or its folded lowercase equivalent</returns>
    /// <remarks>
    ///   <para>
    ///     Folded lowercase is a special variant of lowercase that will result in a string of
    ///     equal or shorter length when encoded to UTF-8 or UTF-16. It is not intended for
    ///     display and some mappings may lead to incorrect lowercase characters for such.
    ///   </para>
    ///   <para>
    ///     Comparing the case-folded translations of two strings will produce the result of
    ///     a case-insensitive comparison. This makes case folding very useful for case
    ///     insensitive comparison logic and associative containers which can store
    ///     pre-case-folded strings for their indexes if they need to be case insensitive.
    ///   </para>
    ///   <para>
    ///     Warning: really, don't use this for displayed strings. It may even replace
    ///     lowercase characters with something weird in case their UTF-8-encoded code point
    ///     would be longer than its uppercase variant.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static char32_t ToFoldedLowercase(char32_t codePoint);

  };

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool UnicodeHelper::IsValidCodePoint(char32_t codePoint) {
    return (
      (codePoint < 0xD800) ||
      (
        (codePoint >= 0xE000) &&
        (codePoint < 1114111)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::GetSequenceLength(char8_t leadCharacter) {
    if(leadCharacter < 128) {
      return 1;
    } else if((leadCharacter & 0xE0) == 0xC0) {
      return 2;
    } else if((leadCharacter & 0xF0) == 0xE0) {
      return 3;
    } else if((leadCharacter & 0xF8) == 0xF0) {
      return 4;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::GetSequenceLength(char16_t leadCharacter) {
    if(leadCharacter < char16_t(0xD800)) {
      return 1; // Single character code point, below surrogate range
    } else if(leadCharacter >= char16_t(0xE000)) {
      return 1; // Single character code point, above surrogate range
    } else if(leadCharacter < char16_t(0xDC00)) {
      return 2; // Two-character code point, lead surrogate
    } else {
      return std::size_t(-1); // It's a trail surrogate, thus no lead character
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::CountUtf8Characters(char32_t codePoint) {
    if(codePoint < 128) {
      return 1;
    } else if(codePoint < 2048) {
      return 2;
    } else if(codePoint < 65536) {
      return 3;
    } else if(codePoint < 1114111) {
      return 4;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr std::size_t UnicodeHelper::CountUtf16Characters(char32_t codePoint) {
    if(codePoint < 0xD800) {
      return 1;
    } else if((codePoint >= 0xE000) && (codePoint < 1114111)) {
      return 2;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline char32_t UnicodeHelper::ReadCodePoint(const char8_t *&current, const char8_t *end) {
    assert((current < end) && u8"At least one byte of input must be available");

    char8_t leadCharacter = *current;
    if(leadCharacter < 128) {
      ++current;
      return static_cast<char32_t>(leadCharacter);
    } else if((leadCharacter & 0xE0) == 0xC0) {
      if(current + 1 < end) {
        char8_t secondCharacter = *(current + 1);
        if((secondCharacter & 0xC0) == 0x80) {
          current += 2;
          return (
            (static_cast<char32_t>(leadCharacter & 0x1F) << 6) |
            (static_cast<char32_t>(secondCharacter & 0x3F))
          );
        }
      }
    } else if((leadCharacter & 0xF0) == 0xE0) {
      if(current + 2 < end) {
        char8_t secondCharacter = *(current + 1);
        char8_t thirdCharacter = *(current + 2);
        bool allCharactersValid = (
          (static_cast<char8_t>(secondCharacter & 0xC0) == 0x80) &&
          (static_cast<char8_t>(thirdCharacter & 0xC0) == 0x80)
        );
        if(allCharactersValid) {
          current += 3;
          return (
            (static_cast<char32_t>(leadCharacter & 0x0F) << 12) |
            (static_cast<char32_t>(secondCharacter & 0x3F) << 6) |
            (static_cast<char32_t>(thirdCharacter & 0x3F))
          );
        }
      }
    } else if((leadCharacter & 0xF8) == 0xF0) {
      if(current + 3 < end) {
        char8_t secondCharacter = *(current + 1);
        char8_t thirdCharacter = *(current + 2);
        char8_t fourthCharacter = *(current + 3);
        bool allCharactersValid = (
          (static_cast<char8_t>(secondCharacter & 0xC0) == 0x80) &&
          (static_cast<char8_t>(thirdCharacter & 0xC0) == 0x80) &&
          (static_cast<char8_t>(fourthCharacter & 0xC0) == 0x80)
        );
        if(allCharactersValid) {
          current += 4;
          return (
            (static_cast<char32_t>(leadCharacter & 0x07) << 18) |
            (static_cast<char32_t>(secondCharacter & 0x3F) << 12) |
            (static_cast<char32_t>(thirdCharacter & 0x3F) << 6) |
            (static_cast<char32_t>(fourthCharacter & 0x3F))
          );
        }
      }
    }

    // Invalid character encountered oder ran out of input
    return char32_t(-1);
  }

  // ------------------------------------------------------------------------------------------- //

  inline char32_t UnicodeHelper::ReadCodePoint(const char16_t *&current, const char16_t *end) {
    assert((current < end) && u8"At least one byte of input must be available");

    char16_t leadCharacter = *current;
    if(leadCharacter < char16_t(0xD800)) {
      ++current;
      return static_cast<char32_t>(leadCharacter);
    } else if(leadCharacter >= char16_t(0xE000)) {
      ++current;
      return static_cast<char32_t>(leadCharacter);
    } else if(leadCharacter < char16_t(0xDC00)) {
      if(current + 1 < end) {
        char16_t trailCharacter = *(current + 1);
        bool allCharactersValid = (
          (static_cast<char16_t>(leadCharacter & 0xFC00) == 0xD800) &&
          (static_cast<char16_t>(trailCharacter & 0xFC00) == 0xDC00)
        );
        if(allCharactersValid) {
          current += 2;
          return char32_t(65536) + (
            (static_cast<char32_t>(leadCharacter & 0x03FF) << 10) |
            (static_cast<char32_t>(trailCharacter & 0x03FF))
          );
        }
      }
    }

    // Invalid character encountered or ran out of input
    return char32_t(-1);
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::size_t UnicodeHelper::WriteCodePoint(char32_t codePoint, char8_t *&target) {
    if(codePoint < 128) {
      *target = static_cast<char8_t>(codePoint);
      ++target;
      return 1;
    } else if(codePoint < 2048) {
      *target = char8_t(0xC0) | static_cast<char8_t>(codePoint >> 6);
      ++target;
      *target = char8_t(0x80) | static_cast<char8_t>(codePoint & 0x3F);
      ++target;
      return 2;
    } else if(codePoint < 65536) {
      *target = char8_t(0xE0) | static_cast<char8_t>(codePoint >> 12);
      ++target;
      *target = char8_t(0x80) | static_cast<char8_t>((codePoint >> 6) & 0x3F);
      ++target;
      *target = char8_t(0x80) | static_cast<char8_t>(codePoint & 0x3F);
      ++target;
      return 3;
    } else if(codePoint < 1114111) {
      *target = char8_t(0xF0) | static_cast<char8_t>(codePoint >> 18);
      ++target;
      *target = char8_t(0x80) | static_cast<char8_t>((codePoint >> 12) & 0x3F);
      ++target;
      *target = char8_t(0x80) | static_cast<char8_t>((codePoint >> 6) & 0x3F);
      ++target;
      *target = char8_t(0x80) | static_cast<char8_t>(codePoint & 0x3F);
      ++target;
      return 4;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  inline std::size_t UnicodeHelper::WriteCodePoint(char32_t codePoint, char16_t *&target) {
    if(codePoint < 65536) {
      assert(
        ((codePoint < 0xDC00) || (codePoint >= 0xE000)) &&
        u8"Unicode code point has to be outside surrogate range (0xDC00-0xDFFF)"
      );
      *target = static_cast<char16_t>(codePoint);
      ++target;
      return 1;
    } else if(codePoint < 1114111) {
      codePoint -= char32_t(65536);
      *(target) = 0xD800 | static_cast<char16_t>(codePoint >> 10);
      *(target + 1) = 0xDC00 | static_cast<char16_t>(codePoint & 0x03FF);
      target += 2;
      return 2;
    } else {
      return std::size_t(-1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_UNICODEHELPER_H
