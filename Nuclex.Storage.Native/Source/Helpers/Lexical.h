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

#ifndef NUCLEX_STORAGE_HELPERS_LEXICAL_H
#define NUCLEX_STORAGE_HELPERS_LEXICAL_H

#include "Nuclex/Storage/Config.h"

#include <string>
#include <sstream>

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  template<typename TTarget, typename TSource>
  inline TTarget lexical_cast(const TSource &from) {
    std::stringstream stringStream;
    stringStream << from;

    TTarget to;
    stringStream >> to;

    if(stringStream.fail() || stringStream.bad()) {
      std::string message("Could not convert from \"");
      stringStream >> message;
      message.append("\" (");
      message.append(typeid(TSource).name());
      message.append(") to (");
      message.append(typeid(TTarget).name());
      message.append(")");
      throw std::invalid_argument(message.c_str());
    }

    return to;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a floating point value into a string</summary>
  /// <param name="from">Floating point value that will be converted</param>
  /// <returns>A string containing the printed floating point value</returns>
  template<> std::string lexical_cast<>(const float &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> float lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a double precision floating point value into a string</summary>
  /// <param name="from">Double precision Floating point value that will be converted</param>
  /// <returns>A string containing the printed double precision floating point value</returns>
  template<> std::string lexical_cast<>(const double &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> double lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ITOA)
  /// <summary>Converts an integer value into a string</summary>
  /// <param name="from">Integer value that will be converted</param>
  /// <returns>A string containing the printed integer value</returns>
  template<> std::string lexical_cast<>(const int &from);
#endif

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an integer value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The integer value parsed from the specified string</returns>
  template<> int lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

#if defined(HAVE_ULTOA)
  /// <summary>Converts an unsigned long value into a string</summary>
  /// <param name="from">Unsigned long value that will be converted</param>
  /// <returns>A string containing the printed unsigned long value</returns>
  template<> std::string lexical_cast<>(const unsigned long &from);
#endif

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a boolean value into a string</summary>
  /// <param name="from">Boolean value that will be converted</param>
  /// <returns>A string containing the printed boolean value</returns>
  template<> std::string lexical_cast<>(const bool &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a boolean value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The boolean value parsed from the specified string</returns>
  template<> bool lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers

#endif // NUCLEX_STORAGE_HELPERS_LEXICAL_H
