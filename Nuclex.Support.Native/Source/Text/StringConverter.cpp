#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#include "Nuclex/Support/Text/StringConverter.h"
#include "Nuclex/Support/Text/UnicodeHelper.h" // UTF encoding and decoding

#include <stdexcept> // for std::invalid_argument

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Invidual UTF-8 character type (until C++20 introduces char8_t)</summary>
  typedef unsigned char my_char8_t;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  std::string::size_type StringConverter::CountUtf8Letters(const std::string &from) {
    const my_char8_t *current = reinterpret_cast<const my_char8_t *>(from.c_str());
    const my_char8_t *end = current + from.length();

    std::string::size_type count = 0;
    while(current < end) {
      std::size_t sequenceLength = UnicodeHelper::GetSequenceLength(*current);
      if(sequenceLength == std::size_t(-1)) {
        throw std::invalid_argument(u8"String contains invalid UTF-8");
      }

      ++count;
      current += sequenceLength;
    }

    return count;
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring StringConverter::WideFromUtf8(const std::string &utf8String) {
    std::wstring result;
    {
      const my_char8_t *read = reinterpret_cast<const my_char8_t *>(utf8String.c_str());
      const my_char8_t *readEnd = read + utf8String.length();

      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      // Variant for 16 bit wchar_t as established by Windows compilers
      if constexpr(sizeof(wchar_t) == sizeof(char16_t)) {
        char16_t *write = reinterpret_cast<char16_t *>(result.data());

        while(read < readEnd) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
          UnicodeHelper::WriteCodePoint(codePoint, write);
        }

        result.resize(write - reinterpret_cast<char16_t *>(result.data()));
      } else { // Variant for 32 bit wchar_t used everywhere except Windows
        char32_t *write = reinterpret_cast<char32_t *>(result.data());

        while(read < readEnd) {
          *write = UnicodeHelper::ReadCodePoint(read, readEnd);
          ++write;
        }

        result.resize(write - reinterpret_cast<char32_t *>(result.data()));
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromWide(const std::wstring &wideString) {
    std::string result;
    {
      // Let's assume 1 UTF-16/UTF-32 character maps to 2 UTF-16 characters. For ASCII
      // strings, we'll allocate twice as much as we need, for international string it will
      // be exactly right, old egyptians and celts may need another allocation along the way.
      result.resize(wideString.length() * 2);

      my_char8_t *write = reinterpret_cast<my_char8_t *>(result.data());
      my_char8_t *writeEnd = write + result.length();

      // Variant for 16 bit wchar_t as established by Windows compilers
      if constexpr(sizeof(wchar_t) == sizeof(char16_t)) {
        const char16_t *read = reinterpret_cast<const char16_t *>(wideString.c_str());
        const char16_t *readEnd = read + wideString.length();

        while(read < readEnd) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
          UnicodeHelper::WriteCodePoint(codePoint, write);

          if(unlikely(write + 4 >= writeEnd)) {
            std::string::size_type writeIndex = (
              write - reinterpret_cast<my_char8_t *>(result.data())
            );
            result.resize(result.size() * 2);
            write = reinterpret_cast<my_char8_t *>(result.data());
            writeEnd = write + result.length();
            write += writeIndex;
          }
        }

        result.resize(write - reinterpret_cast<my_char8_t *>(result.data()));
      } else { // Variant for 32 bit wchar_t used everywhere except Windows
        const char32_t *read = reinterpret_cast<const char32_t *>(wideString.c_str());
        const char32_t *readEnd = read + wideString.length();

        while(read < readEnd) {
          UnicodeHelper::WriteCodePoint(*read, write);
          ++read;

          if(unlikely(write + 4 >= writeEnd)) {
            std::string::size_type writeIndex = (
              write - reinterpret_cast<my_char8_t *>(result.data())
            );
            result.resize(result.size() * 2);
            write = reinterpret_cast<my_char8_t *>(result.data());
            writeEnd = write + result.length();
            write += writeIndex;
          }
        }

        result.resize(write - reinterpret_cast<my_char8_t *>(result.data()));
      }
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u16string StringConverter::Utf16FromUtf8(const std::string &utf8String) {
    std::u16string result;
    {
      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      const my_char8_t *read = reinterpret_cast<const my_char8_t *>(utf8String.c_str());
      const my_char8_t *readEnd = read + utf8String.length();

      char16_t *write = result.data();
      while(read < readEnd) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
        UnicodeHelper::WriteCodePoint(codePoint, write);
      }

      result.resize(write - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromUtf16(const std::u16string &utf16String) {
    std::string result;
    {
      // Let's assume 1 UTF-16/UTF-32 character maps to 2 UTF-16 characters. For ASCII
      // strings, we'll allocate twice as much as we need, for international string it will
      // be exactly right, old egyptians and celts may need another allocation along the way.
      result.resize(utf16String.length() * 2);

      my_char8_t *write = reinterpret_cast<my_char8_t *>(result.data());
      my_char8_t *writeEnd = write + result.length();

      const char16_t *read = utf16String.c_str();
      const char16_t *readEnd = read + utf16String.length();

      while(read < readEnd) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(read, readEnd);
        UnicodeHelper::WriteCodePoint(codePoint, write);

        if(unlikely(write + 4 >= writeEnd)) {
          std::string::size_type writeIndex = (
            write - reinterpret_cast<my_char8_t *>(result.data())
          );
          result.resize(result.size() * 2);
          write = reinterpret_cast<my_char8_t *>(result.data());
          writeEnd = write + result.length();
          write += writeIndex;
        }
      }

      result.resize(write - reinterpret_cast<my_char8_t *>(result.data()));
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::u32string StringConverter::Utf32FromUtf8(const std::string &utf8String) {
    std::u32string result;
    {
      // Let's assume 1 UTF-8 characters maps to 1 UTF-16 character. For ASCII strings,
      // this will be an exact fit, for asian languages, it's probably twice what we need.
      // In any case, it will never come up short, so we don't have to worry about running
      // out of space when writing transcoded UTF characters into the string.
      result.resize(utf8String.length());

      const my_char8_t *read = reinterpret_cast<const my_char8_t *>(utf8String.c_str());
      const my_char8_t *readEnd = read + utf8String.length();

      char32_t *write = result.data();
      while(read < readEnd) {
        *write = UnicodeHelper::ReadCodePoint(read, readEnd);
        ++write;
      }

      result.resize(write - result.data());
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::Utf8FromUtf32(const std::u32string &utf32String) {
    std::string result;
    {
      // Let's assume 1 UTF-16/UTF-32 character maps to 2 UTF-16 characters. For ASCII
      // strings, we'll allocate twice as much as we need, for international string it will
      // be exactly right, old egyptians and celts may need another allocation along the way.
      result.resize(utf32String.length() * 2);

      my_char8_t *write = reinterpret_cast<my_char8_t *>(result.data());
      my_char8_t *writeEnd = write + result.length();

      const char32_t *read = utf32String.c_str();
      const char32_t *readEnd = read + utf32String.length();

      while(read < readEnd) {
        UnicodeHelper::WriteCodePoint(*read, write);
        ++read;

        if(unlikely(write + 4 >= writeEnd)) {
          std::string::size_type writeIndex = (
            write - reinterpret_cast<my_char8_t *>(result.data())
          );
          result.resize(result.size() * 2);
          write = reinterpret_cast<my_char8_t *>(result.data());
          writeEnd = write + result.length();
          write += writeIndex;
        }
      }

      result.resize(write - reinterpret_cast<my_char8_t *>(result.data()));
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string StringConverter::FoldedLowercaseFromUtf8(const std::string &utf8String) {
    std::string result;
    {
      const my_char8_t *current = reinterpret_cast<const my_char8_t *>(utf8String.c_str());
      const my_char8_t *end;
      {
        std::string::size_type length = utf8String.length();
        end = current + length;

        result.resize(length);
      }

      my_char8_t *target = reinterpret_cast<my_char8_t *>(result.data());

      while(current < end) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
        codePoint = UnicodeHelper::ToFoldedLowercase(codePoint);
        UnicodeHelper::WriteCodePoint(codePoint, target);
      }

      // Remove the excess characters in case the string became shorter
      result.resize(target - reinterpret_cast<my_char8_t *>(result.data()));
    }

    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
