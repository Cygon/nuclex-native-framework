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
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/ParserHelper.h"

#include "Utf8/checked.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  void ParserHelper::SkipWhitespace(
    const std::uint8_t *&start, const std::uint8_t *end
  ) {
    const std::uint8_t *next = start;
    for(;;) {
      char32_t codePoint;
      utf8::internal::utf_error result = utf8::internal::validate_next(
        next, end, reinterpret_cast<std::uint32_t *>(&codePoint)
      );
      if(result == utf8::internal::UTF8_OK) {
        if(IsWhitespace(codePoint)) {
          start = next;
        } else {
          return;
        }
      } else { // End or invalid character encountered
        return;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
