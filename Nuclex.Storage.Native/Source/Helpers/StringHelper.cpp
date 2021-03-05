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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_STORAGE_SOURCE 1

#include "StringHelper.h"

#include "Utf8/checked.h"

#include <vector>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>C-style function that checks if a string matches a wild card</summary>
  /// <param name="text">Text that will be checked against the wild card</param>
  /// <param name="wildcard">Wild card against which the text will be matched</param>
  /// <returns>True if the text matches the wild card, false otherwise</returns>
  bool pointerBasedWildcardMatch(const char *text, const char *wildcard) {
    if(!text)
      return false;

    for(; '*' ^ *wildcard; ++wildcard, ++text) {
      if(!*text)
        return (!*wildcard);

      if(::toupper(*text) ^ ::toupper(*wildcard) && '?' ^ *wildcard)
        return false;
    }

    while('*' == wildcard[1])
      wildcard++;

    do {
      if(pointerBasedWildcardMatch(text, wildcard + 1))
        return true;

    } while(*text++);

    return false;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringHelper::WideCharFromUtf8(const std::string &utf8String) {
    if(utf8String.empty()) {
      return std::wstring();
    }

    std::vector<wchar_t> wideCharacters;
    wideCharacters.reserve(utf8String.length());

    utf8::utf8to16(
      utf8String.begin(), utf8String.end(), std::back_inserter(wideCharacters)
    );

    return std::wstring(&wideCharacters[0], wideCharacters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringHelper::Utf8FromWideChar(const std::wstring &wideCharString) {
    if(wideCharString.empty()) {
      return std::string();
    }

    std::vector<char> utf8Characters;
    utf8Characters.reserve(wideCharString.length());

    utf8::utf16to8(
      wideCharString.begin(), wideCharString.end(), std::back_inserter(utf8Characters)
    );

    return std::string(&utf8Characters[0], utf8Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringHelper::Utf8FromWideChar(
    const std::uint16_t *begin, const std::uint16_t *end
  ) {
    std::vector<char> utf8Characters;
    utf8Characters.reserve(end - begin);

    utf8::utf16to8(begin, end, std::back_inserter(utf8Characters));

    return std::string(&utf8Characters[0], utf8Characters.size());
  }

  // ------------------------------------------------------------------------------------------- //

  bool StringHelper::MatchesWildcard(const std::string &text, const std::string &wildcard) {
    return pointerBasedWildcardMatch(text.c_str(), wildcard.c_str());
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers
