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

#ifndef NUCLEX_SUPPORT_TEXT_PARSERHELPER_H
#define NUCLEX_SUPPORT_TEXT_PARSERHELPER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string
#include <optional> // for std::optional
#include <cstdint> // for std::uint32_t, std::int32_t, std::uint64_t, std::int64_t

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods for parsing text-based file formats</summary>
  /// <remarks>
  ///   <para>
  ///     For generic character classification, also see the cctype header in C++ which
  ///     provides several methods to classify ASCII characters. Since all bytes in ASCII
  ///     range remain unqiue in UTF-8 (all 2, 3 and 4 byte sequences have the highest bit
  ///     set), even if you feed each byte of an UTF-8 string to, say, ::isdigit(), it will
  ///     correctly identify all numbers.
  ///   </para>
  ///   <para>
  ///     The methods in this class offer alternatives for UTF-8 parsing. If the full UTF-8
  ///     range is required, the character is passed as a single UTF-32 unit (char32_t)
  ///     which can encode any unicode character in a fixed length (most UTF-8 libraries,
  ///     such as Nemanja Trifunovic's &quot;utfcpp&quot; library let you iterate through
  ///     an UTF-8 string by returning individual characters as UTF-32).
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE ParserHelper {

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="utf8Byte">
    ///   UTF-8 byte or single-byte character that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      std::uint8_t utf8Byte
    );

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="codepoint">
    ///   Unicode code point that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      char32_t codepoint
    );

    /// <summary>
    ///   Moves <paramref cref="start" /> ahead until the first non-whitespace UTF-8
    ///   character or until hitting <paramref cref="end" />
    /// </summary>
    /// <param name="start">Start pointer from which on whitespace will be skipped</param>
    /// <param name="end">End pointer that may not be overrun</param>
    public: NUCLEX_SUPPORT_API static void SkipWhitespace(
      const std::uint8_t *&start, const std::uint8_t *end
    );

  };

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool ParserHelper::IsWhitespace(std::uint8_t utf8Byte) {
    return (
      (
        (utf8Byte >= std::uint8_t(0x09)) && // (see below)
        (utf8Byte < std::uint8_t(0x0e))
      ) ||
      (utf8Byte == std::uint8_t(0x20)) // space
    );
    // Covered via range:
    // (utf8Byte == std::uint8_t(0x09)) || // tab
    // (utf8Byte == std::uint8_t(0x0a)) || // line feed
    // (utf8Byte == std::uint8_t(0x0b)) || // line tabulation
    // (utf8Byte == std::uint8_t(0x0c)) || // form feed
    // (utf8Byte == std::uint8_t(0x0d)) || // carriage return
  }

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool ParserHelper::IsWhitespace(char32_t unicodeCharacter) {
    switch(unicodeCharacter & char32_t(0xffffff00)) {
      case char32_t(0x0000): {
        return (
          (
            (unicodeCharacter >= char32_t(0x0009)) && // (see below)
            (unicodeCharacter < char32_t(0x000e))
          ) ||
          (unicodeCharacter == char32_t(0x0020)) || // space
          (unicodeCharacter == char32_t(0x0085)) || // next line
          (unicodeCharacter == char32_t(0x00A0))    // no-break space
        );
        // Covered via range:
        // (unicodeCharacter == char32_t(0x0009)) || // tab
        // (unicodeCharacter == char32_t(0x000a)) || // line feed
        // (unicodeCharacter == char32_t(0x000b)) || // line tabulation
        // (unicodeCharacter == char32_t(0x000c)) || // form feed
        // (unicodeCharacter == char32_t(0x000d)) || // carriage return
      }
      case char32_t(0x1600): {
        return (unicodeCharacter == char32_t(0x1680)); // ogham space mark
      }
      case char32_t(0x2000): {
        return (
          (unicodeCharacter < char32_t(0x200b)) || // (see below)
          (unicodeCharacter == char32_t(0x2028)) || // line separator
          (unicodeCharacter == char32_t(0x2029)) || // paragraph separator
          (unicodeCharacter == char32_t(0x202f)) || // narrow no-break space
          (unicodeCharacter == char32_t(0x205f))    // medium mathematical space
        );
        // Covered via range:
        // (utf8Character == char32_t(0x2000)) || // en quad
        // (utf8Character == char32_t(0x2001)) || // em quad
        // (utf8Character == char32_t(0x2002)) || // en space
        // (utf8Character == char32_t(0x2003)) || // em space
        // (utf8Character == char32_t(0x2004)) || // three-per-em space
        // (utf8Character == char32_t(0x2005)) || // four-per-em-space
        // (utf8Character == char32_t(0x2006)) || // six-per-em-space
        // (utf8Character == char32_t(0x2007)) || // figure space
        // (utf8Character == char32_t(0x2008)) || // punctuation space
        // (utf8Character == char32_t(0x2009)) || // thin space
        // (utf8Character == char32_t(0x200a))    // hair space
      }
      case char32_t(0x3000): {
        return (unicodeCharacter == char32_t(0x3000)); // ideographic space
      }
      default: {
        return false;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_PARSERHELPER_H
