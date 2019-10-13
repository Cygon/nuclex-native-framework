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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/Lexical.h"

#include "Dragon4/PrintFloat.h"
#include "Erthink/erthink_u2a.h"

#include <cstdlib>

// Goal: print floating-point values accurately, locale-independent and without exponent
//
// std::to_string()
//   o No control over accuracy
//   o Locale-dependent (except Windows, std::setlocale(LC_NUMERIC, "de_DE.UTF-8"))
//   o Uses slow sprintf() internally
//
// Ryu https://github.com/ulfjack/ryu
//   o Always outputs exact number
//   o No control over exponential notation (and results such as "1E0" common)
//   o One of the fastest formatters at this time
//
// Dragon4 http://www.ryanjuckett.com
//   o Always outputs exact number
//   o Can force non-exponential notation
//   o Slower than typical libc implementations
//
// Grisu3 https://github.com/google/double-conversion
//   o Is not always exact
//
// Errol https://github.com/marcandrysco/Errol
//   o Alway outputs exact number
//   o No control over exponential notation
//

// Goal: print integral values accurately and fast
//   https://stackoverflow.com/questions/7890194/
//
// Terje http://computer-programming-forum.com/46-asm/7aa4b50bce8dd985.htm
//   o Fast
//
// Inge https://stackoverflow.com/questions/7890194/
//   o Faster
//
// Vitaut https://github.com/fmtlib/fmt
//   o Fastest
//
// Erthink https://github.com/leo-yuriev/erthink
//   o Faster
//   o Handles signed and 64 bit integers
//

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const std::string &from) {
    if(from.length() >= 3) {
      return (
        ((from[0] == 't') || (from[0] == 'T')) &&
        ((from[1] == 'r') || (from[1] == 'R')) &&
        ((from[2] == 'u') || (from[2] == 'U')) &&
        ((from[3] == 'e') || (from[3] == 'E'))
      );
    } else {
      return false;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return false;
    } else {
      return lexical_cast<bool>(std::string(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const bool &from) {
    static const std::string trueString("true");
    static const std::string falseString("false");

    if(from) {
      return trueString;
    } else {
      return falseString;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::uint8_t &from) {
    char characters[4];
    const char *end = erthink::u2a(static_cast<std::uint32_t>(from), characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::uint8_t>(std::strtoul(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint8_t lexical_cast<>(const std::string &from) {
    return static_cast<std::uint8_t>(std::strtoul(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::int8_t &from) {
    char characters[5];
    const char *end = erthink::i2a(static_cast<std::int32_t>(from), characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::int8_t>(std::strtol(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int8_t lexical_cast<>(const std::string &from) {
    return static_cast<std::int8_t>(std::strtol(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::uint16_t &from) {
    char characters[6];
    const char *end = erthink::u2a(static_cast<std::uint32_t>(from), characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::uint16_t>(std::strtoul(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint16_t lexical_cast<>(const std::string &from) {
    return static_cast<std::uint16_t>(std::strtoul(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::int16_t &from) {
    char characters[7];
    const char *end = erthink::i2a(static_cast<std::int32_t>(from), characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::int16_t>(std::strtol(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int16_t lexical_cast<>(const std::string &from) {
    return static_cast<std::int16_t>(std::strtol(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::uint32_t &from) {
    char characters[11];
    const char *end = erthink::u2a(from, characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0U;
    } else {
      return static_cast<std::uint32_t>(std::strtoul(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint32_t lexical_cast<>(const std::string &from) {
    return static_cast<std::uint32_t>(std::strtoul(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::int32_t &from) {
    char characters[12];
    const char *end = erthink::i2a(from, characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0;
    } else {
      return static_cast<std::int32_t>(std::strtol(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int32_t lexical_cast<>(const std::string &from) {
    return static_cast<std::int32_t>(std::strtol(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::uint64_t &from) {
    char characters[21];
    const char *end = erthink::u2a(from, characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0ULL;
    } else {
      return static_cast<std::uint64_t>(std::strtoull(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::uint64_t lexical_cast<>(const std::string &from) {
    return static_cast<std::uint64_t>(std::strtoull(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const std::int64_t &from) {
    char characters[21];
    const char *end = erthink::i2a(from, characters);
    return std::string(static_cast<const char *>(characters), end);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0LL;
    } else {
      return static_cast<std::int64_t>(std::strtoll(from, nullptr, 10));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::int64_t lexical_cast<>(const std::string &from) {
    return static_cast<std::int64_t>(std::strtoll(from.c_str(), nullptr, 10));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const float &from) {
    char characters[64];
    tU32 length = PrintFloat32(
      characters, sizeof(characters), from, PrintFloatFormat_Positional, -1
    );

    return std::string(static_cast<const char *>(characters), static_cast<std::size_t>(length));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0.0f;
    } else {
      return std::stof(std::string(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::string &from) {
    return std::stof(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const double &from) {
    char characters[256];
    tU32 length = PrintFloat64(
      characters, sizeof(characters), from, PrintFloatFormat_Positional, -1
    );

    return std::string(static_cast<const char *>(characters), static_cast<std::size_t>(length));
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const char *from) {
    if(from == nullptr) {
      return 0.0;
    } else {
      return std::stod(std::string(from));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::string &from) {
    return std::stod(from);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
