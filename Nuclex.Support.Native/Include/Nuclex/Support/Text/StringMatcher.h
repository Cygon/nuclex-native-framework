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

#ifndef NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H
#define NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H

#include "Nuclex/Support/Config.h"

#include <string>

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compares strings using different matching algorithms</summary>
  class StringMatcher {

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

    /// <summary>Checks whether a UTF-8 string matches a wildcard</summary>
    /// <param name="text">Text that will be matches against the wildcard</param>
    /// <param name="wildcard">Wildcard against which the text will be matched</param>
    /// <param name="caseSensitive">Whether the comparison will be case sensitive</param>
    /// <returns>True if the specified text matches the wildcard</returns>
    public: NUCLEX_SUPPORT_API static bool FitsWildcard(
      const std::string &text, const std::string &wildcard, bool caseSensitive = false
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_STRINGMATCHER_H
