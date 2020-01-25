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

#include "Nuclex/Support/Text/StringConverter.h"

#include "Utf8/checked.h"
#include "Utf8Fold/Utf8Fold.h"

#include <vector>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Selects the appropriate conversion for the compiler's wchar_t</summary>
  template<std::size_t WCharWidth>
  struct StringConverterCharWidthHelper {

    /// <summary>Converts a UTF-8 string into a wide (UTF-16 or UTF-32) string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A wide version of the provided UTF-8 string</returns>
    /// <remarks>
    ///   Assumes std::wstring has to carry either UTF-16 or UTF-32 based on the size of
    ///   the compiler's wchar_t, thereby matching the default encoding used by your compiler
    ///   and the defaults of any wide-character APIs on your platform.
    /// </remarks>
    inline static std::wstring WideFromUtf8(const std::string &from) = delete;

    /// <summary>Converts a wide (UTF-16 or UTF-32) string into a UTF-8 string</summary>
    /// <param name="wideString">Wide string that will be converted</param>
    /// <returns>A UTF-8 version of the provided wide string</returns>
    /// <remarks>
    ///   Assumes the std::wstring is carrying either UTF-16 or UTF-32 based on the size of
    ///   the compiler's wchar_t, thereby matching the default encoding used by your compiler
    ///   when you write L&quot;Hello&quot; in your code.
    /// </remarks>
    inline static std::string Utf8FromWide(const std::wstring &from) = delete;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides conversion methods between UTF-8 and UTF-16</summary>
  template<>
  struct StringConverterCharWidthHelper<sizeof(char16_t)> {

    /// <summary>Converts a UTF-8 string into a UTF-16 string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A wide version of the provided UTF-8 string</returns>
    inline static std::wstring WideFromUtf8(const std::string &utf8String) {
      if(utf8String.empty()) {
        return std::wstring();
      }

      // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
      // based on the assumption that most text will only use ascii characters.
      std::vector<wchar_t> utf16Characters;
      utf16Characters.reserve(utf8String.length());

      // Do the conversion. If the vector was too short, it will be grown in factors
      // of 2 usually (depending on the standard library implementation)
      utf8::utf8to16(utf8String.begin(), utf8String.end(), std::back_inserter(utf16Characters));

      return std::wstring(&utf16Characters[0], utf16Characters.size());
    }

    /// <summary>Converts a UTF-16 string into a UTF-8 string</summary>
    /// <param name="wideString">UTF-16 string that will be converted</param>
    /// <returns>A UTF-8 version of the provided UTF-16 string</returns>
    inline static std::string Utf8FromWide(const std::wstring &utf16String) {
      if(utf16String.empty()) {
        return std::string();
      }

      // We guess that we need as many UTF-8 characters as we needed UTF-16 characters
      // based on the assumption that most text will only use ascii characters.
      std::vector<char> utf8Characters;
      utf8Characters.reserve(utf16String.length());

      // Do the conversion. If the vector was too short, it will be grown in factors
      // of 2 usually (depending on the standard library implementation)
      utf8::utf16to8(
        utf16String.begin(), utf16String.end(), std::back_inserter(utf8Characters)
      );

      return std::string(&utf8Characters[0], utf8Characters.size());
    }

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides conversion methods between UTF-8 and UTF-32</summary>
  template<>
  struct StringConverterCharWidthHelper<sizeof(char32_t)> {

    /// <summary>Converts a UTF-8 string into a UTF-32 string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A UTF-32 version of the provided UTF-8 string</returns>
    inline static std::wstring WideFromUtf8(const std::string &utf8String) {
      if(utf8String.empty()) {
        return std::wstring();
      }

      // We guess that we need as many UTF-32 characters as we needed UTF-8 characters
      // based on the assumption that most text will only use ascii characters.
      std::vector<char32_t> utf32Characters;
      utf32Characters.reserve(utf8String.length());

      // Do the conversion. If the vector was too short, it will be grown in factors
      // of 2 usually (depending on the standard library implementation)
      utf8::utf8to32(utf8String.begin(), utf8String.end(), std::back_inserter(utf32Characters));

      // This reinterpret_cast() avoids warnings on platforms with 16 bit wide chars
      // (where it is never executed) and does nothing on platforms with 32 bit wide chars.
      const wchar_t *first = reinterpret_cast<const wchar_t *>(&utf32Characters[0]);
      return std::wstring(first, utf32Characters.size());
    }

    /// <summary>Converts a UTF-32 string into a UTF-8 string</summary>
    /// <param name="wideString">UTF-32 string that will be converted</param>
    /// <returns>A UTF-8 version of the provided UTF-32 string</returns>
    inline static std::string Utf8FromWide(const std::wstring &utf32String) {
      if(utf32String.empty()) {
        return std::string();
      }

      // We guess that we need as many UTF-8 characters as we needed UTF-32 characters
      // based on the assumption that most text will only use ascii characters.
      std::vector<char> utf8Characters;
      utf8Characters.reserve(utf32String.length());

      // Do the conversion. If the vector was too short, it will be grown in factors
      // of 2 usually (depending on the standard library implementation)
      utf8::utf32to8(
        utf32String.begin(), utf32String.end(), std::back_inserter(utf8Characters)
      );

      return std::string(&utf8Characters[0], utf8Characters.size());
    }

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::WideFromUtf8(const std::string &utf8String) {
    return StringConverterCharWidthHelper<sizeof(wchar_t)>::WideFromUtf8(utf8String);
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromWide(const std::wstring &wideString) {
    return StringConverterCharWidthHelper<sizeof(wchar_t)>::Utf8FromWide(wideString);
  }

  // ------------------------------------------------------------------------------------------- //

  std::u16string StringConverter::Utf16FromUtf8(const std::string &utf8String) {
    if(utf8String.empty()) {
      return std::u16string();
    }

    // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<char16_t> utf16Characters;
    utf16Characters.reserve(utf8String.length());

    // Do the conversion. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf8to16(utf8String.begin(), utf8String.end(), std::back_inserter(utf16Characters));

    return std::u16string(&utf16Characters[0], utf16Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromUtf16(const std::u16string &utf16String) {
    if(utf16String.empty()) {
      return std::string();
    }

    // We guess that we need as many UTF-8 characters as we needed UTF-16 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<char> utf8Characters;
    utf8Characters.reserve(utf16String.length());

    // Do the conversion. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf16to8(
      utf16String.begin(), utf16String.end(), std::back_inserter(utf8Characters)
    );

    return std::string(&utf8Characters[0], utf8Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::u32string StringConverter::Utf32FromUtf8(const std::string &utf8String) {
    if(utf8String.empty()) {
      return std::u32string();
    }

    // We guess that we need as many UTF-32 characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<char32_t> utf32Characters;
    utf32Characters.reserve(utf8String.length());

    // Do the conversion. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf8to32(utf8String.begin(), utf8String.end(), std::back_inserter(utf32Characters));

    return std::u32string(&utf32Characters[0], utf32Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromUtf32(const std::u32string &utf32String) {
    if(utf32String.empty()) {
      return std::string();
    }

    // We guess that we need as many UTF-8 characters as we needed UTF-32 characters
    // based on the assumption that most text will only use ascii characters.
    std::vector<char> utf8Characters;
    utf8Characters.reserve(utf32String.length());

    // Do the conversion. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf32to8(
      utf32String.begin(), utf32String.end(), std::back_inserter(utf8Characters)
    );

    return std::string(&utf8Characters[0], utf8Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::FoldedLowercaseFromUtf8(const std::string &utf8String) {
    return ToFoldedLowercase(utf8String);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
