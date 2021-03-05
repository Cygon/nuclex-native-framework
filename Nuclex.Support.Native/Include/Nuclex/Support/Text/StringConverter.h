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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H
#define NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts strings between explicitly specified UTF-formats</summary>
  /// <remarks>
  ///   <para>
  ///     On Windows, 8 bit char strings are usually assumed to be ANSI (that is,
  ///     the 127 standard ASCII characters for the values 1-127 and a set of special
  ///     characters that is defined by the &quot;code page&quot; in the remaining values
  ///     from 128 to 255. Showing strings with the wrong code page displays the wrong
  ///     special characters (but ASCII characters remain intact).
  ///   </para>
  ///   <para>
  ///     On Windows, wchar_t is 16 bits wide and unicode generally means UTF-16,
  ///     so &quot;wide strings&quot; (strings holding wchar_ts) are UTF-16 strings.
  ///     Microsoft's unicode APIs and UI tooling uses this for all i18n support.
  ///   </para>
  ///   <para>
  ///     In the rest of the computing world, the preferred format for unicode is UTF-8.
  ///     It's shorter and survives transmission through non-UTF-8 APIs.
  ///   </para>
  ///   <para>
  ///     The fun begins with wchar_t. It's 32 bits wide on most non-Microsoft systems
  ///     and compilers like GCC default to using UTF-32 when seeing a string like
  ///     L&quot;hello&quot;. You can force GCC to put UTF-16 in the 32 bit wide wchar_ts
  ///     (via -fwide-exec-charset=UTF-16) but then every other library accepting
  ///     &quot;wide strings&quot; will be confused by your UTF-16-inside-UTF-32-strings.
  ///   </para>
  ///   <para>
  ///     So, to summarize: u8&quot;Hello&quot; works everywhere. L&quot;Hello&quot; works
  ///     for calls from Windows applications to Microsoft APIs but gives you a headache
  ///     in any other case, you don't even know if you'll get UTF-16 or UTF-32 until you
  ///     compile it.
  ///   </para>
  ///   <para>
  ///     So use u8&quot;Hello&quot; everywhere. When interacting with Microsoft APIs,
  ///     translate via Utf16FromUtf8(). If you need &quot;wide strings&quot; (you shouldn't)
  ///     use WideCharFromUtf8() - it will select UTF-16 or UTF-32 to match wchar_t.
  ///   </para>
  /// </remarks>
  class StringConverter {

    /// <summary>Counts the number of UTF-8 characters in a string</summary>
    /// <param name="from">UTF-8 string whose characters will be counted</param>
    /// <returns>The number of UTF-8 characters the string is holding</returns>
    public: NUCLEX_SUPPORT_API static std::string::size_type CountUtf8Characters(
      const std::string &from
    );

    /// <summary>Converts a UTF-8 string into a wide (UTF-16 or UTF-32) string</summary>
    /// <param name="from">UTF-8 string that will be converted</param>
    /// <returns>A wide version of the provided UTF-8 string</returns>
    /// <remarks>
    ///   Assumes std::wstring has to carry either UTF-16 or UTF-32 based on the size of
    ///   the compiler's wchar_t, thereby matching the default encoding used by your compiler
    ///   and the defaults of any wide-character APIs on your platform.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::wstring WideFromUtf8(const std::string &from);

    /// <summary>Converts a wide (UTF-16 or UTF-32) string into a UTF-8 string</summary>
    /// <param name="from">Wide string that will be converted</param>
    /// <returns>A UTF-8 version of the provided wide string</returns>
    /// <remarks>
    ///   Assumes the std::wstring is carrying either UTF-16 or UTF-32 based on the size of
    ///   the compiler's wchar_t, thereby matching the default encoding used by your compiler
    ///   when you write L&quot;Hello&quot; in your code.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::string Utf8FromWide(const std::wstring &from);

    /// <summary>Converts a UTF-8 string into a UTF-16 string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A UTF-16 version of the provided UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API static std::u16string Utf16FromUtf8(
      const std::string &utf8String
    );

    /// <summary>Converts a UTF-16 string into a UTF-8 string</summary>
    /// <param name="utf16String">UTF-16 string that will be converted</param>
    /// <returns>A UTF-8 version of the provided UTF-16 string</returns>
    public: NUCLEX_SUPPORT_API static std::string Utf8FromUtf16(
      const std::u16string &utf16String
    );

    /// <summary>Converts a UTF-8 string into a UTF-32 string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A UTF-32 version of the provided UTF-8 string</returns>
    public: NUCLEX_SUPPORT_API static std::u32string Utf32FromUtf8(
      const std::string &utf8String
    );

    /// <summary>Converts a UTF-32 string into a UTF-8 string</summary>
    /// <param name="utf32String">UTF-32 string that will be converted</param>
    /// <returns>A UTF-8 version of the provided UTF-32 string</returns>
    public: NUCLEX_SUPPORT_API static std::string Utf8FromUtf32(
      const std::u32string &utf32String
    );

    /// <summary>Converts the specified UTF-8 string to &quot;folded lowercase&quot;</summary>
    /// <param name="utf8String">String that will be converted</param>
    /// <returns>An equivalent-ish string using only lowercase characters</returns>
    /// <remarks>
    ///   Folded lowercase is a special variant of lowercase that will result in a string of
    ///   equal or shorter length (codepoint-wise). It is not guaranteed to always give the
    ///   correct result for a human reading the string (though in the vast majority of cases
    ///   it does) -- its purpose is to enable case-insensitive comparison of strings.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::string FoldedLowercaseFromUtf8(
      const std::string &utf8String
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGCONVERTER_H
