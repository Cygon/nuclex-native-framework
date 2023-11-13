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
  ///     provides several methods to classify ASCII characters. Since all bytes in the ASCII
  ///     range remain unique in UTF-8 (all 2, 3 and 4 byte sequences have the highest bit
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

    /// <summary>UTF-8 character of which either 1, 2, 3 or 4 specify one codepoint</summary>
    /// <remarks>
    ///   Under C++20, this will be a native type like char16_t and char32_t. There will also
    ///   be an std::u8string using this character type to unambiguously indicate that
    ///   the contents of the string are supposed to be UTF-8 encoded.
    /// </remarks>
    public: typedef unsigned char char8_t;

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="utf8Byte">
    ///   UTF-8 byte or single-byte character that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    /// <remarks>
    ///   This will obviously only cover whitespace variants in the ASCII range, but may
    ///   be sufficient if you're parsing a structured format such as XML, JSON or .ini
    ///   where either the specification limits the allowed whitespace variants outside of
    ///   strings/data or in cases where you're providing the input files yourself rather
    ///   than parsing data from the web or another application.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      char8_t utf8Character
    );

    /// <summary>Checks whether the specified character is a whitespace</summary>
    /// <param name="codePoint">
    ///   Unicode code point that will be checked for being a whitespace
    /// </param>
    /// <returns>True if the character was a whitespace, false otherwise</returns>
    public: NUCLEX_SUPPORT_API static constexpr bool IsWhitespace(
      char32_t codePoint
    );

    /// <summary>Checks if an UTF-8 string is either blank or contains only whitespace</summary
    public: NUCLEX_SUPPORT_API static bool IsBlankOrEmpty(const std::string &text);

    /// <summary>
    ///   Moves <paramref cref="start" /> ahead until the first non-whitespace UTF-8
    ///   character or until hitting <paramref cref="end" />
    /// </summary>
    /// <param name="start">Start pointer from which on whitespace will be skipped</param>
    /// <param name="end">End pointer that may not be overrun</param>
    public: NUCLEX_SUPPORT_API static void SkipWhitespace(
      const char8_t *&start, const char8_t *end
    );

#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)
    /// <summary>Attempts to parse the specified numeric type from the provided text</summary>
    /// <typeparam name="TScalar">
    ///   Type that will be parsed from the text. Must be either a 32 bit integer,
    ///   64 bit integer, float or double. Other types are not supported.
    /// </typeparam>
    /// <param name="start">
    ///   Pointer to the start of the textual data. Will be updated to the next byte
    ///   after the numeric type if parsing succeeds.
    /// </param>
    /// <param name="end">Byte at which the text ends</param>
    /// <returns>The parsed numeric type or an empty std::optional instance</returns>
    public: template<typename TScalar>
    inline static std::optional<TScalar> ParseNumber(
      const std::uint8_t *&start, const std::uint8_t *end
    );
#endif
  };

  // ------------------------------------------------------------------------------------------- //

  inline constexpr bool ParserHelper::IsWhitespace(char8_t utf8Character) {
    return (
      (
        (utf8Character >= std::uint8_t(0x09)) && // (see below)
        (utf8Character < std::uint8_t(0x0e))
      ) ||
      (utf8Character == std::uint8_t(0x20)) // space
    );
    // Covered via range:
    // (utf8Character == std::uint8_t(0x09)) || // tab
    // (utf8Character == std::uint8_t(0x0a)) || // line feed
    // (utf8Character == std::uint8_t(0x0b)) || // line tabulation
    // (utf8Character == std::uint8_t(0x0c)) || // form feed
    // (utf8Character == std::uint8_t(0x0d)) || // carriage return
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
#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  template<typename TScalar>
  inline std::optional<TScalar> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    static_assert(
      (
        std::is_same<TScalar, std::uint32_t>::value ||
        std::is_same<TScalar, std::int32_t>::value ||
        std::is_same<TScalar, std::uint64_t>::value ||
        std::is_same<TScalar, std::int64_t>::value ||
        std::is_same<TScalar, float>::value ||
        std::is_same<TScalar, double>::value
      ) &&
      u8"Only 32/64 bit unsigned/signed integers, floats and doubles are supported"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API std::optional<std::uint32_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<std::int32_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<std::uint64_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<std::int64_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<float> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  );

  template<>
  NUCLEX_SUPPORT_API std::optional<double> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  );
#endif // defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_PARSERHELPER_H
