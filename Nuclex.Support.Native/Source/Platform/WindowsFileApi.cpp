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

#include "WindowsFileApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <stdexcept> // for std::invalid_argument

#include "Nuclex/Support/Text/UnicodeHelper.h" // for UTF-16 <-> UTF-8 conversion

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Throws an exception of the code point is invalid</summary>
  /// <param name="codePoint">Unicode code point that will be checked</param>
  /// <remarks>
  ///   This does a generic code point check, but since within this file the code point
  ///   must be coming from an UTF-8 encoded string, we do complain about invalid UTF-8.
  /// </remarks>
  void requireValidCodePoint(char32_t codePoint) {
    if(!Nuclex::Support::Text::UnicodeHelper::IsValidCodePoint(codePoint)) {
      throw std::invalid_argument(u8"Illegal UTF-8 character(s) encountered");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts a UTF-8 path into a UTF-16 path</summary>
  /// <param name="utf8Path">String containing a UTF-8 path</param>
  /// <returns>The UTF-16 path with magic prefix to eliminate the path length limit</returns>
  std::wstring utf16FromUtf8Path(const std::string &utf8Path) {
    if(utf8Path.empty()) {
      return std::wstring();
    }

    // Valid inputs
    //   - file.txt                      -> \\?\file.txt
    //   - D:/dir/file.txt               -> \\?\D:\dir\file.txt
    //   - \\Server\share\file.txt       -> \\?\UNC\Server\share\file.txt
    //   - \\?\D:\file.txt               -> \\?\D:\file.txt
    //   - \\?\UNC\Server\share\file.txt -> (keep)
/*
    std::string::size_type length = utf8Path.length();
    if(length >= 3) {
      bool
      if(utf8Path[0] == '\\')
    }
*/
    // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    std::wstring utf16Path;
    utf16Path.reserve(utf8Path.length() + 4);

    // According to Microsoft, this is how you lift the 260 char MAX_PATH limit.
    // Also skips the internal call to GetFullPathName() every API method does internally,
    // so paths have to be normalized.
    // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file
    const wchar_t prefix[] = L"\\\\?\\";
    utf16Path.push_back(prefix[0]);
    utf16Path.push_back(prefix[1]);
    utf16Path.push_back(prefix[2]);
    utf16Path.push_back(prefix[3]);

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    {
      using Nuclex::Support::Text::UnicodeHelper;

      const UnicodeHelper::char8_t *current = reinterpret_cast<const UnicodeHelper::char8_t *>(
        utf8Path.c_str()
      );
      const UnicodeHelper::char8_t *end = current + utf8Path.length();

      utf16Path.resize(utf8Path.length() + 4);

      char16_t *write = reinterpret_cast<char16_t *>(utf16Path.data()) + 4;
      while(current < end) {
        char32_t codePoint = UnicodeHelper::ReadCodePoint(current, end);
        requireValidCodePoint(codePoint);
        UnicodeHelper::WriteCodePoint(codePoint, write);
      }

      utf16Path.resize(write - reinterpret_cast<char16_t *>(utf16Path.data()));
    }

    return utf16Path;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileApi::OpenFileForReading(const std::string &path) {
    std::wstring utf16Path = utf16FromUtf8Path(path);

    HANDLE fileHandle = ::CreateFileW(
      utf16Path.c_str(),
      GENERIC_READ, // desired access
      FILE_SHARE_READ, // share mode,
      nullptr,
      OPEN_EXISTING, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
      DWORD errorCode = ::GetLastError();

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for reading");

      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  HANDLE WindowsFileApi::OpenFileForWriting(const std::string &path) {
    std::wstring utf16Path = utf16FromUtf8Path(path);

    HANDLE fileHandle = ::CreateFileW(
      utf16Path.c_str(),
      GENERIC_READ | GENERIC_WRITE, // desired access
      0, // share mode
      nullptr,
      OPEN_ALWAYS, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
      DWORD errorCode = ::GetLastError();

      std::string errorMessage(u8"Could not open file '");
      errorMessage.append(path);
      errorMessage.append(u8"' for writing");

      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return fileHandle;
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileApi::Seek(HANDLE fileHandle, std::ptrdiff_t offset, DWORD anchor) {
    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = offset;
    LARGE_INTEGER newFilePointer;

    BOOL result = ::SetFilePointerEx(
      fileHandle, distanceToMove, &newFilePointer, anchor
    );
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not move file cursor");
      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(newFilePointer.QuadPart);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileApi::Read(HANDLE fileHandle, void *buffer, std::size_t count) {
    DWORD desiredCount = static_cast<DWORD>(count);
    DWORD actualCount = 0;

    BOOL result = ::ReadFile(fileHandle, buffer, desiredCount, &actualCount, nullptr);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not read data from file");
      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(actualCount);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t WindowsFileApi::Write(
    HANDLE fileHandle, const void *buffer, std::size_t count
  ) {
    DWORD desiredCount = static_cast<DWORD>(count);
    DWORD actualCount = 0;

    BOOL result = ::WriteFile(fileHandle, buffer, desiredCount, &actualCount, nullptr);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not write data from file");
      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    return static_cast<std::size_t>(actualCount);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::SetLengthToFileCursor(HANDLE fileHandle) {
    BOOL result = ::SetEndOfFile(fileHandle);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not truncate/pad file to file cursor position");
      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::FlushFileBuffers(HANDLE fileHandle) {
    BOOL result = ::FlushFileBuffers(fileHandle);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not flush file buffers");
      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::CloseFile(HANDLE fileHandle, bool throwOnError /* = true */) {
    BOOL result = ::CloseHandle(fileHandle);
    if(throwOnError && (result == FALSE)) {
      DWORD errorCode = ::GetLastError();
      std::string errorMessage(u8"Could not close file handle");
      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
