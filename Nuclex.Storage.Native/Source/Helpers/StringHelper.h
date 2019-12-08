#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_STORAGE_HELPERS_STRINGHELPER_H
#define NUCLEX_STORAGE_HELPERS_STRINGHELPER_H

#include "Nuclex/Storage/Config.h"

#include <string>
#include <cstdint>

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Some helper methods for dealing with strings</summary>
  class StringHelper {

    /// <summary>Converts an UTF-8 string into a wide char string</summary>
    /// <param name="utf8String">UTF-8 string that will be converted</param>
    /// <returns>A wide char version of the provided ansi string</returns>
    public: static std::wstring WideCharFromUtf8(
      const std::string &utf8String
    );

    /// <summary>Converts a wide char string into an UTF-8 string</summary>
    /// <param name="wideCharString">Wide char string that will be converted</param>
    /// <returns>An UTF-8 version of the provided wide char string</returns>
    public: static std::string Utf8FromWideChar(
      const std::wstring &wideCharString
    );

    /// <summary>Converts the specified wide chracter array into an UTF-8 string</summary>
    /// <param name="begin">Address at which the wide character array begins</param>
    /// <param name="end">Address at which the wide character array ends</param>
    /// <returns>The UTF-8 version of the string</returns>
    public: static std::string Utf8FromWideChar(
      const std::uint16_t *begin, const std::uint16_t *end
    );

    // TODO: The wildcard match likely does not handle UTF-8 correctly
    /// <summary>Checks if a string matches a wild card</summary>
    /// <param name="text">Text that will be checked against the wild card</param>
    /// <param name="wildcard">Wild card against which the text will be matched</param>
    /// <returns>True if the text matches the wild card, false otherwise</returns>
    public: static bool MatchesWildcard(
      const std::string &text, const std::string &wildcard
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers

#endif // NUCLEX_STORAGE_HELPERS_STRINGHELPER_H
