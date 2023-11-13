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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/ParserHelper.h"
#include "Nuclex/Support/Text/UnicodeHelper.h"

#include <cstdlib> // for std::strtoul(), std::strtoull(), std::strtol(), std::strtoll()

namespace {

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)
  /// <summary>Skips over an integer in textual form</summary>
  /// <param name="start">Pointer to the start of the text, will be updated</param>
  /// <param name="end">Pointer one past the last character of the text</param>
  /// <returns>
  ///   True if <paramref cref="start" /> has skipped a valid integer,
  ///   false if not valid integer was found (and <paramref cref="start" /> contains junk)
  /// </returns>
  bool skipInteger(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {

    // An optional plus or minus sign can follow
    if((*start == u8'+') || (*start == u8'-')) {
      ++start;
      if(start == end) { // ...but not only the plus/minus sign!
        return false;
      }
    }

    // If we now get a digit, we can already deliver an integer!
    if(std::isdigit(*start)) {
      ++start;
    }
    while(start != end) {
      if(!std::isdigit(*start)) {
        break;
      }
      ++start;
    }

    return true;

  }
#endif // defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  bool ParserHelper::IsBlankOrEmpty(const std::string &text) {
    const char8_t *current = reinterpret_cast<const char8_t *>(text.c_str());
    const char8_t *end = current + text.length();

    while(current < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
      if(codePoint == char32_t(-1)) {
        return false; // Broken UTF-8 counts as non-empty
      }

      if(!IsWhitespace(codePoint)) {
        return false;
      }
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  void ParserHelper::SkipWhitespace(const char8_t *&start, const char8_t *end) {
    const char8_t *current = start;
    while(current < end) {
      char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
      if(codePoint == char32_t(-1)) {
        break;
      }

      if(IsWhitespace(codePoint)) {
        start = current;
      } else {
        break;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  template<>
  std::optional<std::uint32_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    const std::uint8_t *firstNonSpace = start;

    // Skip whitespaces at the beginning if there are any
    SkipWhitespace(firstNonSpace, end);
    if(firstNonSpace == end) {
      return false;
    }

    // Now look for an integer
    const std::uint8_t *current = firstNonSpace;
    if(skipInteger(current)) {
      start = current;
      if(*firstNonSpace == '-') {
        return static_cast<std::uint32_t>(std::strtoll(firstNonSpace, end, 10));
      } else {
        return static_cast<std::uint32_t>(std::strtoul(firstNonSpace, end, 10));
      }
    } else {
      return std::optional<std::uint32_t>();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  std::optional<std::int32_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    throw u8"Not implemented yet";
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  std::optional<std::uint64_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    throw u8"Not implemented yet";
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  std::optional<std::int64_t> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    throw u8"Not implemented yet";
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  std::optional<float> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    throw u8"Not implemented yet";
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  std::optional<double> ParserHelper::ParseNumber(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    throw u8"Not implemented yet";
  }
#endif // defined(NUCLEX_SUPPORT_CUSTOM_PARSENUMBER)

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
