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

#ifndef NUCLEX_SUPPORT_TEXT_LEXICALCAST_H
#define NUCLEX_SUPPORT_TEXT_LEXICALCAST_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/StringConverter.h" // UTF-8 and wide char conversion

#include <string> // for std::string
#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts from a string to a non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the string will be converted</typeparam>
  /// <param name="from">String that will be converted</param>
  /// <returns>The value converted to the specified string</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated functions
  ///     such as gcvt() or itoa().
  ///   </para>
  ///   <para>
  ///     <list type="bullet">
  ///       <item><term>No iostreams dependency</term></item>
  ///       <item><term>Ignores system locale</term></item>
  ///       <item><term>Full float-string-float round tripping</term></item>
  ///     </list>
  ///   </para>
  /// </remarks>
  template<typename TTarget>
  inline TTarget lexical_cast(const char *from) = delete;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated and functions
  ///     such as gcvt() or itoa().
  ///   </para>
  ///   <para>
  ///     Lexical casts are guaranteed to completely ignore system locale and any other
  ///     localization settings. Primitive types can be converted without pulling in iostreams
  ///     (which is a bit of a heavyweight part of the SC++L).
  ///   </para>
  /// </remarks>
  template<typename TTarget, typename TSource>
  inline TTarget lexical_cast(const TSource &from) = delete;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lexically casts from a string to a non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the string will be converted</typeparam>
  /// <param name="from">String that will be converted</param>
  /// <returns>The value converted to the specified string</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated functions such
  ///     as gcvt() or itoa().
  ///   </para>
  ///   <para>
  ///     Lexical casts are guaranteed to completely ignore system locale and any other
  ///     localization settings. Primitive types can be converted without pulling in iostreams
  ///     (which is a bit of a heavyweight part of the SC++L).
  ///   </para>
  /// </remarks>
  template<typename TTarget>
  inline TTarget wlexical_cast(const wchar_t *from) {
    return lexical_cast<TTarget>(StringConverter::Utf8FromWide(from));
  }

  // ------------------------------------------------------------------------------------------- //

  namespace Private {

    /// <summary>
    ///   Performs a lexical_cast where one side is a UTF-16/UTF-32 string (std::wstring)
    /// </summary>
    /// <typeparam name="TTarget">Target type for the lexical cast</typeparam>
    /// <typeparam name="TSource">Source type for the lexical cast</typeparam>
    template<typename TTarget, typename TSource>
    struct LexicalCastWithConversionHelper {
      /// <summary>Casts from the source type to the target type</summary>
      /// <param name="from">Value that will be cast or a UTF-16 or UTF-32 string</param>
      /// <returns>Resulting UTF-16 or UTF-32 string or value</returns>
      inline static TTarget _(const TSource &from) = delete;
    };

    /// <summary>
    ///   Performs a lexical_cast but converts the input string from UTF-16 or UTF-32
    /// </summary>
    /// <typeparam name="TTarget">Target type for the lexical cast</typeparam>
    template<typename TTarget>
    struct LexicalCastWithConversionHelper<TTarget, std::wstring> {
      /// <summary>Lexically casts from a UTF-16 or UTF-32 string to the target type</summary>
      /// <param name="from">UTF-16 or UTF-32 string to cast to the target type</param>
      /// <returns>The resulting value</returns>
      inline static TTarget _(const std::wstring &from) {
        return lexical_cast<TTarget>(StringConverter::Utf8FromWide(from));
      }
    };

    /// <summary>
    ///   Performs a lexical_cast but converts the resulting string to UTF-16 or UTF-32
    // </summary>
    /// <typeparam name="TSource">Source type for the lexical cast</typeparam>
    template<typename TSource>
    struct LexicalCastWithConversionHelper<std::wstring, TSource> {
      /// <summary>Casts from the source type to a UTF-16 or UTF-32 string</summary>
      /// <param name="from">Value that will be cast to a UTF-16 or UTF-32 string</param>
      /// <returns>Resulting UTF-16 or UTF-32 string</returns>
      inline static std::wstring _(const TSource &from) {
        return StringConverter::WideFromUtf8(lexical_cast<std::string>(from));
      }
    };

  } // namespace Private

  /// <summary>Lexically casts between a string and non-string data type</summary>
  /// <typeparam name="TTarget">Type into which the value will be converted</typeparam>
  /// <typeparam name="TSource">Type that will be converted</typeparam>
  /// <param name="from">Value that will be converted</param>
  /// <returns>The value converted to the specified type</returns>
  /// <remarks>
  ///   <para>
  ///     This cast offers a portable way to convert between numeric and string types without
  ///     resorting to cumbersome sprintf() constructs or relying on deprecated functions such
  ///     as gcvt() or itoa().
  ///   </para>
  ///   <para>
  ///     Lexical casts are guaranteed to completely ignore system locale and any other
  ///     localization settings. It also performs all conversion without pulling in iostreams
  ///     (which is a bit of a heavyweight part of the SC++L).
  ///   </para>
  /// </remarks>
  template<typename TTarget, typename TSource>
  inline TTarget wlexical_cast(const TSource &from) {
    return Private::LexicalCastWithConversionHelper<TTarget, TSource>::_(from);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a boolean value into a string</summary>
  /// <param name="from">Boolean value that will be converted</param>
  /// <returns>A string containing the printed boolean value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const bool &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a boolean value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The boolean value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API bool lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a boolean value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The boolean value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API bool lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 8 bit unsigned integer into a string</summary>
  /// <param name="from">8 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 8 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint8_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint8_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint8_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 8 bit unsigned integer into a string</summary>
  /// <param name="from">8 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 8 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int8_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int8_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into an 8 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 8 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int8_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 16 bit unsigned integer into a string</summary>
  /// <param name="from">16 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 16 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint16_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint16_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint16_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts an 16 bit signed integer into a string</summary>
  /// <param name="from">16 bit signed integer that will be converted</param>
  /// <returns>A string containing the printed 16 bit signed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int16_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int16_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 16 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 16 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int16_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 32 bit unsigned integer into a string</summary>
  /// <param name="from">32 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 32 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint32_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint32_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint32_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 32 bit signed integer into a string</summary>
  /// <param name="from">32 bit signed integer that will be converted</param>
  /// <returns>A string containing the printed 32 bit signed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int32_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int32_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 32 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 32 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int32_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 64 bit unsigned integer into a string</summary>
  /// <param name="from">64 bit unsigned integer that will be converted</param>
  /// <returns>A string containing the printed 64 bit unsigned integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::uint64_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint64_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit unsigned integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit unsigned integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::uint64_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a 64 bit signed integer into a string</summary>
  /// <param name="from">64 bit signed integer that will be converted</param>
  /// <returns>A string containing the printed 64 bit signed integer value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const std::int64_t &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int64_t lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a 64 bit signed integer</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The 64 bit signed integer parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API std::int64_t lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a floating point value into a string</summary>
  /// <param name="from">Floating point value that will be converted</param>
  /// <returns>A string containing the printed floating point value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const float &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API float lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API float lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a double precision floating point value into a string</summary>
  /// <param name="from">Double precision Floating point value that will be converted</param>
  /// <returns>A string containing the printed double precision floating point value</returns>
  template<> NUCLEX_SUPPORT_API std::string lexical_cast<>(const double &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a double precision floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API double lexical_cast<>(const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a string into a double precision floating point value</summary>
  /// <param name="from">String that will be converted</param>
  /// <returns>The floating point value parsed from the specified string</returns>
  template<> NUCLEX_SUPPORT_API double lexical_cast<>(const std::string &from);

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_LEXICALCAST_H
