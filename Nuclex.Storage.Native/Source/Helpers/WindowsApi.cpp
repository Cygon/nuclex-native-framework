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

#include "WindowsApi.h"

#if defined(NUCLEX_STORAGE_WIN32)

#include "Nuclex/Storage/Errors/FileAccessError.h"
#include "Nuclex/Storage/Errors/PermissionError.h"
#include "Nuclex/Storage/Errors/BadPathError.h"

#include "Nuclex/Support/Text/StringConverter.h"

#include "Utf8/checked.h"

#include <vector> // for std::vector

// --------------------------------------------------------------------------------------------- //

namespace {

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

} // anonymous namespace

// --------------------------------------------------------------------------------------------- //

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  std::string WindowsApi::GetErrorMessage(int errorNumber) {
    std::vector<wchar_t> buffer;
    buffer.resize(256);
    for(;;) {

      // Try to obtain the error number. The return value of strerror_r() is different
      // between GNU and Posix. There's no reliable error return, so we reset errno for
      // the current thread and check it again after calling strerror_r()
      errno = 0;
      int lookupErrorNumber = ::_wcserror_s(&buffer[0], buffer.size(), errorNumber);
      if(lookupErrorNumber == 0) {
        int errorNumberFromStrError = errno;
        if(errorNumberFromStrError == 0) {
          return Nuclex::Support::Text::StringConverter::Utf8FromWide(
            std::wstring(&buffer[0])
          );
        }

        // If the buffer was too small, try again with 1024 bytes, 4096 bytes and
        // 16384 bytes, then blow up.
        if(errorNumberFromStrError == ERANGE) {
          std::size_t bufferSize = buffer.size();
          if(bufferSize < 16384) {
            buffer.resize(bufferSize * 4);
            continue;
          }
        }
      }

      // We failed to look up the error message. At least output the original
      // error number and remark that we weren't able to look up the error message.
      std::string errorMessage(u8"Error ");
      errorMessage.append(std::to_string(errorNumber));
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
        message.append(std::to_string(errorCode));
        return message;
      }
    }

    // We don't want UTF-16 anywhere - at all. So convert this mess to UTF-8.
    std::vector<char> utf8ErrorMessage;
    {
      LocalAllocScope errorMessageScope(errorMessageBuffer);

      utf8ErrorMessage.reserve(errorMessageLength);
      utf8::utf16to8(
        errorMessageBuffer, errorMessageBuffer + errorMessageLength,
        std::back_inserter(utf8ErrorMessage)
      );
    }

    // Microsoft likes to end their error messages with various spaces and newlines,
    // cut these off so we have a single-line error message
    std::size_t size = utf8ErrorMessage.size();
    while(size > 0) {
      bool isWhitespace = (
        (utf8ErrorMessage[size - 1] == ' ') ||
        (utf8ErrorMessage[size - 1] == '\t') ||
        (utf8ErrorMessage[size - 1] == '\r') ||
        (utf8ErrorMessage[size - 1] == '\n')
      );
      if(!isWhitespace) {
        break;
      }
      --size;
    }

    // If the error message is empty, return a generic one
    if(size == 0) {
      std::string message(u8"Windows API error ");
      message.append(std::to_string(errorCode));
      return message;
    } else { // Error message had content, return it
      return std::string(&utf8ErrorMessage[0], size);
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

  bool WindowsApi::IsPermissionError(DWORD errorCode) {
    return (
      (errorCode == ERROR_ACCESS_DENIED) ||
      (errorCode == ERROR_NETWORK_ACCESS_DENIED) ||
      (errorCode == ERROR_VIRUS_INFECTED) ||
      (errorCode == ERROR_WRITE_PROTECT) ||
      (errorCode == ERROR_SHARING_VIOLATION) ||
      (errorCode == ERROR_LOCK_VIOLATION) ||
      (errorCode == ERROR_SHARING_PAUSED) ||
      (errorCode == ERROR_REQ_NOT_ACCEP) ||
      (errorCode == ERROR_FILE_CHECKED_OUT) ||
      (errorCode == ERROR_OPLOCK_NOT_GRANTED) || // questionable...
      (errorCode == ERROR_NOT_ALLOWED_ON_SYSTEM_FILE)
    );
  }
        
  // ------------------------------------------------------------------------------------------- //

  bool WindowsApi::IsPathError(DWORD errorCode) {
    return (
      (errorCode == ERROR_FILE_NOT_FOUND) ||
      (errorCode == ERROR_PATH_NOT_FOUND) ||
      (errorCode == ERROR_INVALID_DRIVE) ||
      (errorCode == ERROR_BAD_NETPATH) ||
      (errorCode == ERROR_DEV_NOT_EXIST) ||
      (errorCode == ERROR_NETNAME_DELETED) ||
      (errorCode == ERROR_BAD_NET_NAME) ||
      (errorCode == ERROR_INVALID_NAME) ||
      (errorCode == ERROR_BAD_PATHNAME) ||
      (errorCode == ERROR_FILENAME_EXCED_RANGE) ||
      (errorCode == ERROR_DIRECTORY) ||
      (errorCode == ERROR_BAD_DEVICE_PATH) // questionable...
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForSystemError(
    const std::string &errorMessage, DWORD errorCode
  ) {
    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(errorCode));

    if(WindowsApi::IsPermissionError(errorCode)) {
      throw Nuclex::Storage::Errors::PermissionError(
        std::error_code(errorCode, std::system_category()), combinedErrorMessage
      );
    } else if(WindowsApi::IsPathError(errorCode)) {
      throw Nuclex::Storage::Errors::BadPathError(
        std::error_code(errorCode, std::system_category()), combinedErrorMessage
      );
    } else {
      throw Nuclex::Storage::Errors::FileAccessError(
        std::error_code(errorCode, std::system_category()), combinedErrorMessage
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsApi::ThrowExceptionForHResult(
    const std::string &errorMessage, HRESULT resultHandle
  ) {
    using Nuclex::Storage::Helpers::WindowsApi;

    std::string combinedErrorMessage(errorMessage);
    combinedErrorMessage.append(u8" - ");
    combinedErrorMessage.append(WindowsApi::GetErrorMessage(resultHandle));

    throw Nuclex::Storage::Errors::FileAccessError(
      std::error_code(resultHandle, std::system_category()), combinedErrorMessage
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers

#endif // defined(NUCLEX_STORAGE_WIN32)
