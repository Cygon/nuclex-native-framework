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

#include "WindowsApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/Text/StringConverter.h" // to convert UTF-16 wholesome
#include "Nuclex/Support/Text/ParserHelper.h" // to skip trailing whitespace
#include "Nuclex/Support/Text/LexicalAppend.h" // to append error codes to strings
#include "Nuclex/Support/Text/UnicodeHelper.h" // for UTF-16 <-> UTF-8 conversion

#include <vector> // for std::vector
#include <system_error> // for std::system_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Releases memory that has been allocated by LocalAlloc()</summary>
  class LocalAllocScope {

    /// <summary>Initializes a new local memory releaser for the specified pointer</summary>
    /// <param name="localAddress">Pointer that will be released</param>
    public: LocalAllocScope(void *localAddress) : localAddress(localAddress) {}

    /// <summary>Frees the memory the memory releaser is responsible for</summary>
    public: ~LocalAllocScope() {
      ::LocalFree(localAddress);
    }

    /// <summary>Pointer to the memory the memory releaser will release</summary>
    private: void *localAddress;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Searches a string for a zero terminator and truncates everything after</summary>
  /// <param name="stringToTrim">String that will be trimmed</param>
  void trimStringToZeroTerminator(std::wstring &stringToTrim) {
    std::wstring::size_type terminatorIndex = stringToTrim.find(L'\0');
    if(terminatorIndex != std::wstring::npos) {
      stringToTrim.resize(terminatorIndex);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(int errorNumber) {
    std::wstring buffer(256, L'\0');
    for(;;) {

      // Try to obtain the error message relating to the POSIX error number. In order to be
      // unicode-safe, we have to use Microsoft's UTF-16 methods.
      errno = 0;
      int lookupErrorNumber = ::_wcserror_s(buffer.data(), buffer.length(), errorNumber);
      if(lookupErrorNumber == 0) {
        int errorNumberFromStrError = errno;
        if(errorNumberFromStrError == 0) {
          trimStringToZeroTerminator(buffer);
          return Nuclex::Support::Text::StringConverter::Utf8FromWide(buffer);
        }

        // If the buffer was too small, try again with 1024 bytes, 4096 bytes and
        // 16384 bytes, then blow up.
        if(errorNumberFromStrError == ERANGE) {
          std::size_t bufferSize = buffer.length();
          if(bufferSize < 16384) {
            buffer.resize(bufferSize * 4);
            continue;
          }
        }
      }

      // We failed to look up the error message. At least output the original
      // error number and remark that we weren't able to look up the error message.
      std::string errorMessage(u8"Error ");
      Text::lexical_append(errorMessage, errorNumber);
      errorMessage.append(u8" (and error message lookup failed)");
      return errorMessage;

    } // for(;;)
  }

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(DWORD errorCode) {

    // Use FormatMessage() to ask Windows for a human-readable error message
    LPWSTR errorMessageBuffer;
    DWORD errorMessageLength = ::FormatMessageW(
      (
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS
      ),
      nullptr, // message string, ignored with passed flags
      errorCode,
      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Try for an english message first
      reinterpret_cast<LPWSTR>(&errorMessageBuffer), // MS wants us to cast off pointer levels!
      0,
      nullptr
    );
    if(errorMessageLength == 0) {
      // MSDN states that "Last-Error" will be set to ERROR_RESOURCE_LANG_NOT_FOUND,
      // but that doesn't really happen, so we recheck on any FormatMessage() failure
      errorMessageLength = ::FormatMessageW(
        (
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS
        ),
        nullptr, // message string, ignored with passed flags
        errorCode,
        0, // Let FormatMessage search: neutral, thread locale, user locale and system locale
        reinterpret_cast<LPWSTR>(&errorMessageBuffer), // MS wants us to cast off pointer levels!
        0,
        nullptr
      );
      if(errorMessageLength == 0) {
        std::string message(u8"Windows API error ");
        Text::lexical_append(message, static_cast<std::uint32_t>(errorCode));
        return message;
      }
    }

    // We don't want UTF-16 anywhere - at all. So convert this mess to UTF-8.
    std::string utf8ErrorMessage;
    {
      using Nuclex::Support::Text::UnicodeHelper;

      LocalAllocScope errorMessageScope(errorMessageBuffer);

      utf8ErrorMessage.resize(errorMessageLength);
      {
        const char16_t *current = reinterpret_cast<const char16_t *>(errorMessageBuffer);
        const char16_t *end = current + errorMessageLength;
        UnicodeHelper::char8_t *write = reinterpret_cast<UnicodeHelper::char8_t *>(
          utf8ErrorMessage.data()
        );
        while(current < end) {
          char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
          if(codePoint == char32_t(-1)) {
            break;
          }
          UnicodeHelper::WriteCodePoint(codePoint, write);
        }

        utf8ErrorMessage.resize(
          write - reinterpret_cast<UnicodeHelper::char8_t *>(utf8ErrorMessage.data())
        );
      }
    }

    // Microsoft likes to end their error messages with various spaces and newlines,
    // cut these off so we have a single-line error message
    std::string::size_type length = utf8ErrorMessage.length();
    while(length > 0) {
      if(!Text::ParserHelper::IsWhitespace(std::uint8_t(utf8ErrorMessage[length - 1]))) {
        break;
      }
      --length;
    }

    // If the error message is empty, return a generic one
    if(length == 0) {
      std::string message(u8"Windows API error ");
      Text::lexical_append(message, static_cast<std::uint32_t>(errorCode));
      return message;
    } else { // Error message had content, return it
      utf8ErrorMessage.resize(length);
      return utf8ErrorMessage;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(HRESULT resultHandle) {

    // The _com_error class has a bit of special code when the error message could
    // not be looked up. If the error code is greater than or equal to
    // WCODE_HRESULT_FIRST and also less than or equal to WCODE_HRESULT_LAST,
    // the error is a dispatch error (IDispatch, late-binding).
    //
    //     return (hr >= WCODE_HRESULT_FIRST && hr <= WCODE_HRESULT_LAST)
    //         ? WORD(hr - WCODE_HRESULT_FIRST)
    //         : 0;
    //
    // I don't think we'll encounter IDispatch errors in this library.
    return GetErrorMessage(static_cast<DWORD>(resultHandle));

  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForSystemError(
    const std::string &errorMessage, DWORD errorCode
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(errorCode));

    throw std::system_error(
      std::error_code(errorCode, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForHResult(
    const std::string &errorMessage, HRESULT resultHandle
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(resultHandle));

    throw std::system_error(
      std::error_code(resultHandle, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
