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

#ifndef NUCLEX_SUPPORT_TEXT_LEXICALAPPEND_H
#define NUCLEX_SUPPORT_TEXT_LEXICALAPPEND_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Text/StringConverter.h" // UTF-8 and wide char conversion

#include <string> // for std::string
#include <cstdint> // for std::uint8_t

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends strings and numeric types as text to an UTF-8 string</summary>
  /// <param name="target">String to which the UTF-8 characters will be appended</param>
  /// <param name="from">What will be appended to the UTF-8 string</param>
  /// <returns>
  ///   The number of bytes appended to the UTF-8 string or the number of bytes needed
  /// </returns>
  /// <remarks>
  ///   <para>
  ///     This method conveniently appends various data types, lexically converted into
  ///     UTF-8 strings, to another UTF-8 string. It is useful if you want to avoid
  ///     allocations and unneccessary copies.
  ///   </para>
  ///   <para>
  ///     <code>
  ///       int currentScore = 31241;
  ///       std::string scoreText(14 + 11 + 1, '\0'); // optional: reserve exact length
  ///
  ///       scoreText.append(u8"The score is: ");
  ///       lexical_append(scoreText, currentScore);
  ///     </code>
  ///   </para>
  ///   <para>
  ///     Compared to lexical_cast and/or std::to_string(), using this method avoids any
  ///     temporary string copies and/or memory allocations.
  ///   </para>
  ///   <para>
  ///     <list type="bullet">
  ///       <item><term>No iostreams dependency</term></item>
  ///       <item><term>Ignores system locale</term></item>
  ///       <item><term>No memory allocations (if string capacity suffices)</term></item>
  ///     </list>
  ///   </para>
  /// </remarks>
  template<typename TValue>
  inline void lexical_append(std::string &target, const TValue &from) = delete;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends strings and numeric types as text to an UTF-8 string</summary>
  /// <param name="target">Address at which the UTF-8 characters will be stored</param>
  /// <param name="availableBytes">Number of bytes available at the provided address</param>
  /// <param name="from">What will be appended to the UTF-8 string</param>
  /// <returns>
  ///   The number of bytes appended to the UTF-8 string or the number of bytes needed
  /// </returns>
  /// <remarks>
  ///   <para>
  ///     This method conveniently appends various data types, lexically converted into
  ///     UTF-8 strings, to another UTF-8 string. It is useful if you want to avoid
  ///     allocations and unneccessary copies.
  ///   </para>
  ///   <para>
  ///     In this variant, the text is written to a caller-provided memory address. If there
  ///     is not enough space available, the method instead returns the number of bytes
  ///     that would be required.
  ///   </para>
  ///   <para>
  ///     In the latter case, the contents of the caller-provided memory may or may not have
  ///     been overwritten with a portion of the generated text (in short: neither rely on
  ///     them staying unchanged nor realy on getting a partial result).
  ///   </para>
  ///   <para>
  ///     <code>
  ///       int currentScore = 31241;
  ///       std::vector&lt;char&gt; scoreCharacters(14 + 11 = 1);
  ///
  ///       {
  ///         static const std::string message(u8"The score is: ");
  ///
  ///         std::copy_n(message.c_str(), 14, scoreCharacters.data());
  ///         std::size_t characterCount = lexical_append(
  ///           scoreCharacters.data() + 14, scoreCharacters.size() - 14, currentScore
  ///         );
  ///         assert(characterCount < scoreCharacters.size() - 14);
  ///
  ///         scoreCharacters[charactersCount + 14] = '\0';
  ///       }
  ///     </code>
  ///   </para>
  ///   <para>
  ///     Note that only the characters written to the user-provided memory block.
  ///     No terminating zero byte is appended.
  ///   </para>
  ///   <para>
  ///     <list type="bullet">
  ///       <item><term>No iostreams dependency</term></item>
  ///       <item><term>Ignores system locale</term></item>
  ///       <item><term>No memory allocations</term></item>
  ///       <item><term>No terminating zero byte appended(!)</term></item>
  ///     </list>
  ///   </para>
  /// </remarks>
  template<typename TValue>
  inline std::size_t lexical_append(
    char *target, std::size_t availableBytes, const TValue &from
  ) = delete;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast boolean to an existing string</summary>
  /// <param name="target">String to which the boolean will be appended</param>
  /// <param name="from">Boolean that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(std::string &target, const bool &from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast boolean to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Boolean that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const bool &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a zero-terminated string to an existing string</summary>
  /// <param name="target">String to which to append</param>
  /// <param name="from">Zero-terminated string that will be appended</param>
  NUCLEX_SUPPORT_API void lexical_append(std::string &target, const char *from);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a zero-terminated string to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Zero-terminated string that will be appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  NUCLEX_SUPPORT_API std::size_t lexical_append(
    char *target, std::size_t availableBytes, const char *from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends another string to an existing string</summary>
  /// <param name="target">String to which to append</param>
  /// <param name="from">Other string that will be appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::string &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends another string to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Other string that will be appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::string &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends an 8 bit unsigned integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::uint8_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 8 bit unsigned integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::uint8_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends an 8 bit signed integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::int8_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 8 bit signed integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::int8_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a 16 bit unsigned integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::uint16_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 16 bit unsigned integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::uint16_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a 16 bit signed integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::int16_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 16 bit signed integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::int16_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a 32 bit unsigned integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::uint32_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 32 bit unsigned integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::uint32_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a 32 bit signed integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::int32_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 32 bit signed integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::int32_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a 64 bit unsigned integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::uint64_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 64 bit unsigned integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::uint64_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a 64 bit signed integer to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const std::int64_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast 64 bit signed integer to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Integer that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const std::int64_t &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a floating point value to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Floating point value that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const float &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a lexically cast floating point value to an existing string</summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Floating point value that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const float &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends a double precision floating point value to an existing string</summary>
  /// <param name="target">String to which the integer will be appended</param>
  /// <param name="from">Floating point value that will be lexically cast and appended</param>
  template<> NUCLEX_SUPPORT_API void lexical_append<>(
    std::string &target, const double &from
  );

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Appends a lexically cast double precision floating point value to an existing string
  /// </summary>
  /// <param name="target">Memory address at which text will be stored</param>
  /// <param name="availableBytes">Number of bytes available at that memory address</param>
  /// <param name="from">Floating point value that will be lexically cast and appended</param>
  /// <returns>The number of bytes written at the provided address</returns>
  template<> NUCLEX_SUPPORT_API std::size_t lexical_append<>(
    char *target, std::size_t availableBytes, const double &from
  );

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text

#endif // NUCLEX_SUPPORT_TEXT_LEXICALAPPEND_H
