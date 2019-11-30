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

#ifndef NUCLEX_PIXELS_STORAGE_REALFILE_H
#error This file must be included through RealFile.cpp
#endif

#if defined(NUCLEX_PIXELS_WIN32)
#include "Utf8/checked.h"
#endif

// This is nothing more than a ton of wrapper functions because the
// Windows API is so unwieldy and pesters us with UTF-16.

namespace {

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Converts a UTF-8 path into a UTF-16 path</summary>
  /// <param name="utf8String">String containing a UTF-8 path</param>
  /// <param name="utf16Characters">Vector that will receive the UTF-16 path</param>
  void utf8ToUtf16Path(const std::string &utf8String, std::vector<wchar_t> &utf16Characters) {
    utf16Characters.clear();

    if(utf8String.empty()) {
      return;
    }

    // We guess that we need as many UTF-16 characters as we needed UTF-8 characters
    // based on the assumption that most text will only use ascii characters.
    utf16Characters.reserve(utf8String.length() + 4);

    // According to Microsoft, this is how you lift the 260 char MAX_PATH limit.
    const wchar_t prefix[] = L"\\\\?\\";
    utf16Characters.push_back(prefix[0]);
    utf16Characters.push_back(prefix[1]);
    utf16Characters.push_back(prefix[2]);
    utf16Characters.push_back(prefix[3]);

    // Do the conversions. If the vector was too short, it will be grown in factors
    // of 2 usually (depending on the standard library implementation)
    utf8::utf8to16(utf8String.begin(), utf8String.end(), std::back_inserter(utf16Characters));
  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>RAII helper that calls LocalFree() on a wide character string</summary>
  struct BufferAllocationScope {

    /// <summary>Initializes a new string memory releaser</summary>
    /// <param name="buffer">Buffer that will be freed when the instance is destroyed</param>
    public: BufferAllocationScope(LPWSTR buffer) :
      buffer(buffer) {}

    /// <summary>Frees the buffer</summary>
    public: ~BufferAllocationScope() {
      ::LocalFree(reinterpret_cast<HLOCAL>(buffer));
    }

    /// <summary>Buffer that will be freed when the instance is destroyed</summary>
    private: LPWSTR buffer;

  };

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Returns the error message to the most-recent Windows API error</summary>
  /// <param name="errorCode">The error code returned by GetLastError()</param>
  /// <returns>The error message for the most-recently occurred Windows API error</returns>
  std::string getWindowsError(DWORD errorCode) {

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
        0, // Let FormatMessage search neutral, thread locale, user locale and system locale
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
      BufferAllocationScope errorMessageScope(errorMessageBuffer);

      utf8ErrorMessage.reserve(errorMessageLength);
      utf8::utf16to8(
        errorMessageBuffer, errorMessageBuffer + errorMessageLength,
        std::back_inserter(utf8ErrorMessage)
      );
    }

    // Microsoft likes to end an error message with various spaces and newlines,
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

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Throws a FileAccessError exception for a Windows API error</summary>
  /// <param name="errorCode">Error code for which a file access error is thrown</param>
  void throwWindowsFileAccessError(DWORD errorCode) {
    throw Nuclex::Pixels::Errors::FileAccessError(
      std::error_code(errorCode, std::system_category()), getWindowsError(errorCode)
    );
  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Opens a file for reading using the windows API</summary>
  /// <param name="path">Path fo the file that will be opened for reading</parma>
  /// <param name="promiseSequentialAccess">
  ///   Whether you promise to access the file only sequentially
  /// </param>
  /// <returns>The handle of the opened file<returns>
  HANDLE openWindowsFileForReading(const std::string &path, bool promiseSequentialAccess) {

    // Convert the path to UTF-16 because it's what Microsoft unicode APIs crave
    std::vector<wchar_t> utf16Path;
    {
      utf8ToUtf16Path(path, utf16Path);
      utf16Path.push_back(0);
    }

    HANDLE fileHandle = ::CreateFileW(
      &utf16Path[0],
      FILE_READ_DATA, // File exists, we only want its contents, nothing else
      FILE_SHARE_READ, // Others can read at the same time but not change
      nullptr,
      OPEN_EXISTING, // File must already exist
      promiseSequentialAccess ? FILE_FLAG_SEQUENTIAL_SCAN : 0,
      nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) {
      DWORD lastErrorCode = ::GetLastError();
      throwWindowsFileAccessError(lastErrorCode);
    }

    return fileHandle;

  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Opens a file for writing using the windows API</summary>
  /// <param name="path">Path fo the file that will be opened for writing</parma>
  /// <param name="promiseSequentialAccess">
  ///   Whether you promise to access the file only sequentially
  /// </param>
  /// <returns>The handle of the opened file<returns>
  HANDLE openWindowsFileForWriting(const std::string &path, bool promiseSequentialAccess) {

    // Convert the path to UTF-16 because it's what Microsoft unicode APIs crave
    std::vector<wchar_t> utf16Path;
    {
      utf8ToUtf16Path(path, utf16Path);
      utf16Path.push_back(0);
    }

    HANDLE fileHandle = ::CreateFileW(
      &utf16Path[0],
      FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE,
      0, // no sharing for a file that is being created / modified
      nullptr,
      CREATE_ALWAYS, // create a new file, even if one existed already
      FILE_ATTRIBUTE_NORMAL | (promiseSequentialAccess ? FILE_FLAG_SEQUENTIAL_SCAN : 0),
      nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) {
      DWORD lastErrorCode = ::GetLastError();
      throwWindowsFileAccessError(lastErrorCode);
    }

    return fileHandle;

  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)
   
  /// <summary>Determines the size of an open file</summary>
  /// <param name="fileHandle">Handle of the file whose size will be determined</param>
  /// <returns>The size of the file with the specified handle</returns>
  std::uint64_t getWindowsFileSize(HANDLE fileHandle) {
    LARGE_INTEGER fileSize;

    BOOL succeeded = ::GetFileSizeEx(fileHandle, &fileSize);
    if(!succeeded) {
      DWORD lastErrorCode = ::GetLastError();
      throwWindowsFileAccessError(lastErrorCode);
    }

    return static_cast<std::uint64_t>(fileSize.QuadPart);
  }

#endif

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)
   
  /// <summary>Moves the file cursor in a Windows file</summary>
  /// <param name="fileHandle">Handle of the file whose cursor will be moved</param>
  /// <param name="position">New position for the file cursor</param>
  void moveWindowsFileCursor(HANDLE fileHandle, std::uint64_t position) {
    assert(
      (position < static_cast<std::uint64_t>(std::numeric_limits<LONGLONG>::max())) &&
      "Requested position must be within the limitations of the Windows API"
    );

    LARGE_INTEGER fileCursorPosition;
    fileCursorPosition.QuadPart = static_cast<LONGLONG>(position);

    BOOL succeeded = ::SetFilePointerEx(
      fileHandle, fileCursorPosition, nullptr, FILE_BEGIN
    );
    if(!succeeded) {
      DWORD lastErrorCode = ::GetLastError();
      throwWindowsFileAccessError(lastErrorCode);
    }
  }

#endif

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)
   
  /// <summary>Reads data from a file using the Windows API</summary>
  /// <param name="fileHandle">Handle of the file from which data will be read</param>
  /// <param name="fileCursor">
  ///   Position within the file (not checked to move the file cursor but is updated)
  /// </param>
  /// <param name="byteCount">Number of bytes that will be read</param>
  /// <param name="buffer">Buffer into which the data will be written</param>
  void readWindowsFile(
    HANDLE fileHandle,
    std::uint64_t &fileCursor, std::size_t byteCount, std::uint8_t *buffer
  ) {
    assert(
      (byteCount < std::numeric_limits<DWORD>::max()) &&
      "Requested byte count must be within the limitations of the Windows API"
    );

    // Read from the current location in the file
    DWORD readByteCount;
    BOOL succeeded = ::ReadFile(
      fileHandle,
      buffer,
      static_cast<DWORD>(byteCount),
      &readByteCount,
      nullptr
    );
    if(!succeeded) {
      DWORD lastErrorCode = ::GetLastError();
      throwWindowsFileAccessError(lastErrorCode);
    }

    // Read succeeded, so the file cursor has moved
    fileCursor += readByteCount;

    // But the read may still have given us fewer bytes than requested.
    // This is the kind of complex, hidden-trap API design we want to avoid.
    if(readByteCount != byteCount) {
      throwWindowsFileAccessError(ERROR_HANDLE_EOF);
    }
  }

#endif

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)
   
  /// <summary>Writes data into a file using the Windows API</summary>
  /// <param name="fileHandle">Handle of the file from which data will be read</param>
  /// <param name="fileCursor">
  ///   Position within the file (not checked to move the file cursor but is updated)
  /// </param>
  /// <param name="byteCount">Number of bytes that will be read</param>
  /// <param name="buffer">Buffer into which the data will be written</param>
  void writeWindowsFile(
    HANDLE fileHandle,
    std::uint64_t &fileCursor, std::size_t byteCount, const std::uint8_t *buffer
  ) {
    assert(
      (byteCount < std::numeric_limits<DWORD>::max()) &&
      "Requested byte count must be within the limitations of the Windows API"
    );

    // Write to the current location in the file
    DWORD writtenByteCount;
    BOOL succeeded = ::WriteFile(
      fileHandle,
      buffer,
      static_cast<DWORD>(byteCount),
      &writtenByteCount,
      nullptr
    );
    if(!succeeded) {
      DWORD lastErrorCode = ::GetLastError();
      throwWindowsFileAccessError(lastErrorCode);
    }

    // Write succeeded, so the file cursor has moved
    fileCursor += writtenByteCount;

    // But the write may still have written fewer bytes than requested.
    // Which to us means the operation did not yield the expected outcome, aka it failed.
    if(writtenByteCount != byteCount) {
      throwWindowsFileAccessError(ERROR_HANDLE_EOF);
    }
  }

#endif

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace
