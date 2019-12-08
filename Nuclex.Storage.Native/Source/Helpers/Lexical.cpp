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
#define NUCLEX_STORAGE_SOURCE 1

#include "Lexical.h"

#include <cstdlib>

// Temporary workaround, will switch to common lexical_cast from Nuclex.Support.Native
#if !defined(_CVTBUFSIZE)
#define _CVTBUFSIZE 256
#endif

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const float &from) {
    throw std::logic_error("Not implemented yet");
    /*
    char characters[_CVTBUFSIZE];
    return std::string(::_gcvt(from, 0, characters));
    */
  }

  // ------------------------------------------------------------------------------------------- //

  template<> float lexical_cast<>(const std::string &from) {
    return std::stof(from);
  }

  // ------------------------------------------------------------------------------------------- //

  template<> std::string lexical_cast<>(const double &from) {
    throw std::logic_error("Not implemented yet");
    /*
    char characters[_CVTBUFSIZE];
    return std::string(::_gcvt(from, 0, characters));
    */
  }

  // ------------------------------------------------------------------------------------------- //

  template<> double lexical_cast<>(const std::string &from) {
    return std::stod(from);
  }

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ITOA)
  template<> std::string lexical_cast<>(const int &from) {
    char characters[21]; // Max number of characters in exotic 64 bit integer platform
    return std::string(::itoa(from, characters, 10));
  }
#endif

  // ------------------------------------------------------------------------------------------- //

  template<> int lexical_cast<>(const std::string &from) {
    return std::stoi(from);
  }

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ULTOA)
  template<> std::string lexical_cast<>(const unsigned long &from) {
    char characters[21];
    return std::string(::ultoa(from, characters, 10));
  }
#endif

  // ------------------------------------------------------------------------------------------- //

  template<> bool lexical_cast<>(const std::string &from) {
    if(from.length() == 4) {
      static const char lowerChars[] = { 't', 'r', 'u', 'e' };
      static const char upperChars[] = { 'T', 'R', 'U', 'E' };

      for(std::size_t index = 0; index < 4; ++index) {
        if((from[index] != lowerChars[index]) && (from[index] != upperChars[index])) {
          return false;
        }
      }

      return true;
    }
    
    return false;
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

}}} // namespace Nuclex::Storage::Helpers
