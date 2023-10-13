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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H
#define NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H

#include "Nuclex/Support/Config.h"

#include <string> // for std::string
#include <functional> //for std::hash, std::equal_to, std::less
#include <cstdint> // for std::uint32_t, std::uint64_t

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compares strings using different matching algorithms</summary>
  class NUCLEX_SUPPORT_TYPE StringMatcher {

    /// <summary>Compares two UTF-8 strings for equality, optionally ignoring case</summary>
    /// <param name="left">String that will be compared on the left side</param>
    /// <param name="right">String that will be compared on the right side</param>
    /// <param name="caseSensitive">Whether the comparison will be case sensitive</param>
    /// <returns>True if the two strings are equal, false otherwise</returns>
    /// <remarks>
    ///   This method is ideal for one-off comparisons. If you have to compare one string
    ///   against multiple strings or want to create a case-insensitive string map,
    ///   consider using the <see cref="StringConverter.ToFoldedLowercase" /> method.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static bool AreEqual(
      const std::string &left, const std::string &right, bool caseSensitive = false
    );

    /// <summary>Checks whether one UTF-8 string contains another UTF-8 string</summary>
    /// <param name="haystack">
    ///   String that will be scanned for instances of another string
    /// </param>
    /// <param name="needle">String which might appear inside the other string</param>
    /// <param name="caseSensitive">Whether the comparison will be case sensitive</param>
    /// <returns>
    ///   True if the 'needle' string appears at least once in the 'haystack' string
    /// </returns>
    public: NUCLEX_SUPPORT_API static bool Contains(
      const std::string &haystack, const std::string &needle, bool caseSensitive = false
    );

    /// <summary>Checks whether one UTF-8 string starts with another UTF-8 string</summary>
    /// <param name="haystack">
    ///   String whose beginning will be compared with the searched-for string
    /// </param>
    /// <param name="needle">String with which the checked string must begin</param>
    /// <param name="caseSensitive">Whether the comparison will be case sensitive</param>
    /// <returns>
    ///   True if the 'haystack' string starts with the 'needle' string
    /// </returns>
    public: NUCLEX_SUPPORT_API static bool StartsWith(
      const std::string &haystack, const std::string &needle, bool caseSensitive = false
    );

    /// <summary>Checks whether a UTF-8 string matches a wildcard</summary>
    /// <param name="text">Text that will be matched against the wildcard</param>
    /// <param name="wildcard">Wildcard against which the text will be matched</param>
    /// <param name="caseSensitive">Whether the comparison will be case sensitive</param>
    /// <returns>True if the specified text matches the wildcard</returns>
    /// <remarks>
    ///   Wildcards refer to the simple placeholder symbols employed by many shells,
    ///   where a '?' acts as a stand-in for one UTF-8 character and a '*' acts as
    ///   a stand-in for zero or more UTF-8 characters. For example &quot;*l?o*&quot;
    ///   would match &quot;Hello&quot; and &quot;lion&quot; but not &quot;glow&quot;.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static bool FitsWildcard(
      const std::string &text, const std::string &wildcard, bool caseSensitive = false
    );

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Case-insensitive UTF-8 version of std::hash&lt;std::string&gt;</summary>
  /// <remarks>
  ///   You can use this to construct a case-insensitive <code>std::unordered_map</code>.
  /// </remarks>
  struct NUCLEX_SUPPORT_TYPE CaseInsensitiveUtf8Hash {

    /// <summary>Calculates a case-insensitive hash of an UTF-8 string</summary>
    /// <param name="text">UTF-8 string of which a hash value will be calculated</param>
    /// <returns>The case-insensitive hash value of the provided string</returns>
    public: NUCLEX_SUPPORT_API std::size_t operator()(
      const std::string &text
    ) const noexcept;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Case-insensitive UTF-8 version of std::equal_to&lt;std::string&gt;</summary>
  /// <remarks>
  ///   You can use this to construct a case-insensitive <code>std::unordered_map</code>.
  /// </remarks>
  struct NUCLEX_SUPPORT_TYPE CaseInsensitiveUtf8EqualTo {

    /// <summary>Checks if two UTF-8 strings are equal, ignoring case</summary>
    /// <param name="left">First UTF-8 string to compare</param>
    /// <param name="right">Other UTF-8 string to compare</param>
    /// <returns>True if both UTF-8 strings have equal contents</returns>
    public: NUCLEX_SUPPORT_API bool operator()(
      const std::string &left, const std::string &right
    ) const noexcept;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Case-insensitive UTF-8 version of std::less&lt;std::string&gt;</summary>
  /// <remarks>
  ///   You can use this to construct a case-insensitive <code>std::map</code>.
  /// </remarks>
  struct NUCLEX_SUPPORT_TYPE CaseInsensitiveUtf8Less {

    /// <summary>Checks if the first UTF-8 string is 'less' than the second</summary>
    /// <param name="left">First UTF-8 string to compare</param>
    /// <param name="right">Other UTF-8 string to compare</param>
    /// <returns>True if the first UTF-8 string is 'less', ignoring case</returns>
    public: NUCLEX_SUPPORT_API bool operator()(
      const std::string &left, const std::string &right
    ) const noexcept;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H
