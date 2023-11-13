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

#include "Nuclex/Support/TemporaryFileScope.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "Nuclex/Support/ScopeGuard.h" // Closing opened files even if exceptions happen
#include "Nuclex/Support/Text/StringConverter.h" // Conversion between UTF-8 and wide char
#include "Platform/WindowsApi.h" // Minimalist Windows.h and error handling helpers
#include "Platform/WindowsPathApi.h" // Basic path manipulation required to join directories
#include "Platform/WindowsFileApi.h" // Opening files and reading/writing them
#else
#include "Platform/PosixApi.h" // Posix error handling
#include "Platform/PosixPathApi.h" // Basic posix path manipulation for temp directory access
#include "Platform/LinuxFileApi.h" // Opening files and reading/writing them

#include <unistd.h> // for ::write(), ::close(), ::unlink()
#include <cstdlib> // for ::getenv(), ::mkdtemp()
#endif

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Builds the full template string that's passed to ::mkstemp()</summary>
  /// <param name="path">Path vector the template will be stored in</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  void buildTemplateForMksTemp(std::string &path, const std::string &prefix) {
    path.reserve(256); // PATH_MAX would be a bit too bloaty usually...

    // Obtain the system's temporary directory (usually /tmp, can be overridden)
    //   path: "/tmp/"
    {
      Nuclex::Support::Platform::PosixPathApi::GetTemporaryDirectory(path);

      std::string::size_type length = path.size();
      if(path[length -1] != '/') {
        path.push_back('/');
      }
    }

    // Append the user-specified prefix, if any
    //   path: "/tmp/myapp"
    if(!prefix.empty()) {
      path.append(prefix);
    }

    // Append the mandatory placeholder characters
    //   path: "/tmp/myappXXXXXX"
    {
      static const std::string placeholder(u8"XXXXXX", 6);
      path.append(placeholder);
    }
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TemporaryFileScope::TemporaryFileScope(const std::string &namePrefix /* = u8"tmp" */) :
    path(),
    privateImplementationData {0} {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    static_assert(
      (sizeof(this->privateImplementationData) >= sizeof(HANDLE)) &&
      u8"File handlefits in space provided for private implementation data"
    );
    *reinterpret_cast<HANDLE *>(this->privateImplementationData) = INVALID_HANDLE_VALUE;

    std::wstring fullPath = Platform::WindowsPathApi::CreateTemporaryFile(namePrefix);
    HANDLE fileHandle = ::CreateFileW(
      fullPath.c_str(),
      GENERIC_READ | GENERIC_WRITE, // desired access
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
      nullptr,
      CREATE_ALWAYS, // creation disposition
      FILE_ATTRIBUTE_NORMAL,
      nullptr
    );
    if(unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
      DWORD errorCode = ::GetLastError();

      // Something went wrong, kill the temporary file again before throwing the exception
      BOOL result = ::DeleteFileW(fullPath.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary file is successfully deleted in error handler");

      std::string errorMessage(u8"Could not open temporary file '");
      errorMessage.append(Text::StringConverter::Utf8FromWide(fullPath));
      errorMessage.append(u8"' for writing");

      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    // If we don't close the file, it cannot be accessed unless other code opens
    // it with FILE_SHARE_READ | FILE_SHARE_WRITE (the latter is the problem)
    ::CloseHandle(fileHandle);

    this->path = Text::StringConverter::Utf8FromWide(fullPath);
#else
    static_assert(
      (sizeof(this->privateImplementationData) >= sizeof(int)) &&
      u8"File descriptor fits in space provided for private implementation data"
    );

    // Build the path template including the system's temporary directory
    std::string pathTemplate;
    buildTemplateForMksTemp(pathTemplate, namePrefix);

    // Select and open a unique temporary filename
    int fileDescriptor = ::mkstemp(pathTemplate.data());
    if(unlikely(fileDescriptor == -1)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not create temporary file '");
      errorMessage.append(pathTemplate);
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    // Store the file handle in the private implementation data block and
    // remember the full path for when the user queries it later
    *reinterpret_cast<int *>(this->privateImplementationData) = fileDescriptor;
    this->path.assign(pathTemplate);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryFileScope::~TemporaryFileScope() {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    if(likely(!this->path.empty())) {
      std::wstring utf16Path = Text::StringConverter::WideFromUtf8(this->path);
      BOOL result = ::DeleteFileW(utf16Path.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary file is successfully deleted after use");
    }
#else
    int fileDescriptor = *reinterpret_cast<int *>(this->privateImplementationData);

    // Close the file so we don't leak handles
    if(likely(fileDescriptor != 0)) {
      int result = ::close(fileDescriptor);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Temporary file is closed successfully");
    }

    // Delete the file. Even if the close failed, on Linux systems we
    // can delete the file and it will be removed from the file index
    // (and the data will disppear as soon as the last process closes it).
    if(likely(!this->path.empty())) {
      int result = ::unlink(this->path.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != -1) && u8"Temporary file is deleted successfully");
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryFileScope::SetFileContents(
    const std::uint8_t *contents, std::size_t byteCount
  ) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    ::HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForWriting(this->path);
    ON_SCOPE_EXIT {
      BOOL result = ::CloseHandle(fileHandle);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"File handle is closed successfully");
    };

    Platform::WindowsFileApi::Seek(fileHandle, 0, FILE_BEGIN);
    Platform::WindowsFileApi::Write(fileHandle, contents, byteCount);
    Platform::WindowsFileApi::SetLengthToFileCursor(fileHandle);
    Platform::WindowsFileApi::FlushFileBuffers(fileHandle);
#else
    int fileDescriptor = *reinterpret_cast<int *>(this->privateImplementationData);
    assert((fileDescriptor != 0) && u8"File is opened and accessible");

    Platform::LinuxFileApi::Seek(fileDescriptor, ::off_t(0), SEEK_SET);
    Platform::LinuxFileApi::Write(fileDescriptor, contents, byteCount);
    Platform::LinuxFileApi::SetLength(fileDescriptor, byteCount);
    Platform::LinuxFileApi::Flush(fileDescriptor);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
