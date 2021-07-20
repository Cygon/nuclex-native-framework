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

#include "Nuclex/Support/TemporaryDirectoryScope.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)
#include "Platform/WindowsApi.h" // for WindowsApi
#include "Platform/WindowsPathApi.h" // for WindowsPathApi
#include "Platform/WindowsFileApi.h" // for WindowsFileApi
#include "Nuclex/Support/Text//StringConverter.h"
#else
#include "Platform/PosixApi.h" // for PosixApi
#include "Platform/PosixPathApi.h" // for PosixPathApi
#include "Platform/LinuxFileApi.h" // for LinuxFileApi

#include <ftw.h> // for struct ::ftw
#include <sys/stat.h> // for struct ::stat
#include <unistd.h> // for ::write(), ::close(), ::unlink()
#include <cstdlib> // for ::getenv(), ::mkdtemp()
#endif

#include "Nuclex/Support/ScopeGuard.h"

#include <vector> // for std::vector
#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Builds the full template string that's passed to ::mkdtemp()</summary>
  /// <param name="path">Path vector the template will be stored in</param>
  /// <param name="prefix">Prefix for the temporary filename, can be empty</param>
  void buildTemplateForMkdTemp(std::string &path, const std::string &prefix) {
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
      path.insert(path.end(), prefix.begin(), prefix.end());
    }

    // Append the mandatory placeholder characters
    //   path: "/tmp/myappXXXXXX"
    {
      static const std::string placeholder(u8"XXXXXX", 6);
      path.insert(path.end(), placeholder.begin(), placeholder.end());
    }
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Callback for the file-tree-walk function that deletes files</summary>
  /// <param name="path">Path of the current file or directory</param>
  /// <param name="nodeStat">Stat structure of the current file or directory</param>
  /// <param name="typeflag">Type of the current node (FTW_F or FTW_D)</param>
  /// <param name="ftwinfo">Folder depth an other info provided by nftw()</param>
  /// <returns>0 on success, a standard Posix error code in case of failure</returns>
  int removeFileOrDirectoryCallback(
    const char *path, const struct ::stat *nodeStat, int typeflag, struct ::FTW *ftwinfo
  ) {
    (void)nodeStat;
    (void)typeflag;
    (void)ftwinfo;

    return ::remove(path);
  }
#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  /// <summary>Recursively deletes the specified directory and all its contents</summary>
  /// <param name="path">Absolute path of the directory that will be deleted</param>
  /// <remarks>
  ///   The path must not be terminated with a path separator.
  /// </remarks>
  void deleteDirectoryWithContents(const std::wstring &path) {
    static const std::wstring allFilesMask(L"\\*");

    ::WIN32_FIND_DATAW findData;

    // First, delete the contents of the directory, recursively for subdirectories
    std::wstring searchMask = path + allFilesMask;
    HANDLE searchHandle = ::FindFirstFileExW(
      searchMask.c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, nullptr, 0
    );
    if(searchHandle == INVALID_HANDLE_VALUE) {
      DWORD lastError = ::GetLastError();
      if(lastError != ERROR_FILE_NOT_FOUND) { // or ERROR_NO_MORE_FILES, ERROR_NOT_FOUND?
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not start directory enumeration", lastError
        );
      }
    }

    // Did this directory have any contents? If so, delete them first
    if(searchHandle != INVALID_HANDLE_VALUE) {
      ON_SCOPE_EXIT {
        BOOL result = ::FindClose(searchHandle);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert((result != FALSE) && u8"Search handle is closed successfully");
      };
      for(;;) {

        // Do not process the obligatory '.' and '..' directories
        if(findData.cFileName[0] != '.') {
          bool isDirectory = (
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
          );

          // Subdirectories need to be handled by deleting their contents first
          std::wstring filePath = path + L'\\' + findData.cFileName;
          if(isDirectory) {
            deleteDirectoryWithContents(filePath);
          } else {
            BOOL result = ::DeleteFileW(filePath.c_str());
            if(result == FALSE) {
              DWORD lastError = ::GetLastError();
              Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
                u8"Could not delete temporary file", lastError
              );
            }
          }
        }

        // Advance to the next file in the directory
        BOOL result = ::FindNextFileW(searchHandle, &findData);
        if(result == FALSE) {
          DWORD lastError = ::GetLastError();
          if(lastError != ERROR_NO_MORE_FILES) {
            Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
              u8"Error during directory enumeration", lastError
            );
          }
          break; // All directory contents enumerated and deleted
        }

      } // for
    }

    // The directory is empty, we can now safely remove it
    BOOL result = ::RemoveDirectoryW(path.c_str());
    if(result == FALSE) {
      DWORD lastError = ::GetLastError();
      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not remove nested temporary directory", lastError
      );
    }
  }
#endif // defined(NUCLEX_SUPPORT_WINDOWS)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::TemporaryDirectoryScope(
    const std::string &namePrefix /* = u8"tmp" */
  ) : path() {
#if defined(NUCLEX_SUPPORT_WINDOWS)

    // Ask Windows to create a unique temporary file for us
    std::wstring filePath = Platform::WindowsPathApi::CreateTemporaryFile(namePrefix);
    auto temporaryFileDeleter = ON_SCOPE_EXIT_TRANSACTION {
      BOOL result = ::DeleteFileW(filePath.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary file is deleted successfull in error handler");
    };

    // Leave the file in place and add a '.dir' to the end for the temporary directory
    std::wstring directoryPath;
    directoryPath.reserve(filePath.length() + 5);
    directoryPath.assign(filePath);
    static const std::wstring suffix(L".dir", 4);
    directoryPath.append(suffix);

    // Create the temporary directory
    BOOL result = ::CreateDirectoryW(directoryPath.c_str(), nullptr);
    if(unlikely(result == FALSE)) {
      DWORD errorCode = ::GetLastError();

      std::string errorMessage(u8"Could not create directory '");
      errorMessage.append(Text::StringConverter::Utf8FromWide(directoryPath));
      errorMessage.append(u8"'");

      Platform::WindowsApi::ThrowExceptionForSystemError(errorMessage, errorCode);
    }

    // Everything worked out, remember the path and disarm the scope guard
    temporaryFileDeleter.Commit();
    this->path.assign(Text::StringConverter::Utf8FromWide(directoryPath));

#else

    // Build a template string in the system's temp directory to call ::mkdtemp()
    std::string pathTemplate;
    buildTemplateForMkdTemp(pathTemplate, namePrefix);

    // Select and open a unique temporary directory name
    const char *path = ::mkdtemp(pathTemplate.data());
    if(unlikely(path == nullptr)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not create temporary directory '");
      errorMessage.append(pathTemplate);
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }

    // Store the full path to the temporary directory we just created
    assert((path == pathTemplate.c_str()) && u8"Original path buffer is modified");
    this->path.assign(pathTemplate);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::~TemporaryDirectoryScope() {
#if defined(NUCLEX_SUPPORT_WINDOWS)
    std::wstring utf16Path = Text::StringConverter::WideFromUtf8(this->path);
    deleteDirectoryWithContents(utf16Path);

    std::wstring::size_type length = utf16Path.length();
    if(length > 4) {
      utf16Path.resize(length - 4);
      BOOL result = ::DeleteFileW(utf16Path.c_str());
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"Temporary directory scope file is deleted successfully");
    }
#else
    int result = ::nftw(this->path.c_str(), removeFileOrDirectoryCallback, 64, FTW_DEPTH | FTW_PHYS);
    if(unlikely(result != 0)) {
      int errorNumber = errno;

      std::string errorMessage(u8"Could not erase temporary directory contents in '");
      errorMessage.append(this->path);
      errorMessage.append(u8"'");

      Platform::PosixApi::ThrowExceptionForSystemError(errorMessage, errorNumber);
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::string TemporaryDirectoryScope::GetPath(const std::string &filename) const {
    std::string fullPath = this->path;
    {
      if(fullPath.length() > 0) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
        char lastCharacter = fullPath[fullPath.length() - 1];
        if((lastCharacter != '\\') && (lastCharacter != '/')) {
          fullPath.push_back('\\');
        }
#else
        if(fullPath[fullPath.length() - 1] != '/') {
          fullPath.push_back('/');
        }
#endif
      }
      fullPath.append(filename);
    }

    return fullPath;
  }

  // ------------------------------------------------------------------------------------------- //

  std::string TemporaryDirectoryScope::PlaceFile(
    const std::string &name, const std::uint8_t *contents, std::size_t byteCount
  ) {
    std::string fullPath = this->path;
    {
      if(fullPath.length() > 0) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
        char lastCharacter = fullPath[fullPath.length() - 1];
        if((lastCharacter != '\\') && (lastCharacter != '/')) {
          fullPath.push_back('\\');
        }
#else
        if(fullPath[fullPath.length() - 1] != '/') {
          fullPath.push_back('/');
        }
#endif
      }
      fullPath.append(name);
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    {
      HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForWriting(fullPath);
      ON_SCOPE_EXIT {
        Platform::WindowsFileApi::CloseFile(fileHandle);
      };

      Platform::WindowsFileApi::Write(fileHandle, contents, byteCount);
      Platform::WindowsFileApi::FlushFileBuffers(fileHandle);
    }
#else
    {
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForWriting(fullPath);
      ON_SCOPE_EXIT {
        Platform::LinuxFileApi::Close(fileDescriptor);
      };

      Platform::LinuxFileApi::Write(fileDescriptor, contents, byteCount);
      Platform::LinuxFileApi::Flush(fileDescriptor);
    }
#endif

    return fullPath;
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryDirectoryScope::ReadFile(
    const std::string &name, std::vector<std::uint8_t> &contents
  ) const {
    std::string fullPath = this->path;
    {
      if(fullPath.length() > 0) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
        char lastCharacter = fullPath[fullPath.length() - 1];
        if((lastCharacter != '\\') && (lastCharacter != '/')) {
          fullPath.push_back('\\');
        }
#else
        if(fullPath[fullPath.length() - 1] != '/') {
          fullPath.push_back('/');
        }
#endif
      }
      fullPath.append(name);
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    {
      HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForReading(fullPath);
      ON_SCOPE_EXIT { Platform::WindowsFileApi::CloseFile(fileHandle); };

      contents.resize(4096);
      for(std::size_t offset = 0;; offset += 4096) {
        std::size_t readByteCount = Platform::WindowsFileApi::Read(
          fileHandle, contents.data() + offset, 4096
        );
        if(readByteCount < 4096) {
          contents.resize(contents.size() - 4096 + readByteCount);
          break;
        } else {
          contents.resize(contents.size() + readByteCount);
        }
      }
    }
#else
    {
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(fullPath);
      ON_SCOPE_EXIT { Platform::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;; offset += 4096) {
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
          fileDescriptor, contents.data() + offset, 4096
        );
        if(readByteCount < 4096) {
          contents.resize(contents.size() - 4096 + readByteCount);
          break;
        } else {
          contents.resize(contents.size() + readByteCount);
        }
      }
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryDirectoryScope::ReadFile(const std::string &name, std::string &contents) const {
    std::string fullPath = this->path;
    {
      if(fullPath.length() > 0) {
#if defined(NUCLEX_SUPPORT_WINDOWS)
        char lastCharacter = fullPath[fullPath.length() - 1];
        if((lastCharacter != '\\') && (lastCharacter != '/')) {
          fullPath.push_back('\\');
        }
#else
        if(fullPath[fullPath.length() - 1] != '/') {
          fullPath.push_back('/');
        }
#endif
      }
      fullPath.append(name);
    }

#if defined(NUCLEX_SUPPORT_WINDOWS)
    {
      HANDLE fileHandle = Platform::WindowsFileApi::OpenFileForReading(fullPath);
      ON_SCOPE_EXIT { Platform::WindowsFileApi::CloseFile(fileHandle); };

      contents.resize(4096);
      for(std::size_t offset = 0;; offset += 4096) {
        std::uint8_t *data = reinterpret_cast<std::uint8_t *>(contents.data());
        std::size_t readByteCount = Platform::WindowsFileApi::Read(
          fileHandle, data + offset, 4096
        );
        if(readByteCount < 4096) {
          contents.resize(contents.size() - 4096 + readByteCount);
          break;
        } else {
          contents.resize(contents.size() + readByteCount);
        }
      }
    }
#else
    {
      int fileDescriptor = Platform::LinuxFileApi::OpenFileForReading(fullPath);
      ON_SCOPE_EXIT { Platform::LinuxFileApi::Close(fileDescriptor); };

      contents.resize(4096);
      for(std::size_t offset = 0;; offset += 4096) {
        std::uint8_t *data = reinterpret_cast<std::uint8_t *>(contents.data());
        std::size_t readByteCount = Platform::LinuxFileApi::Read(
          fileDescriptor, data + offset, 4096
        );
        if(readByteCount < 4096) {
          contents.resize(contents.size() - 4096 + readByteCount);
          break;
        } else {
          contents.resize(contents.size() + readByteCount);
        }
      }
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
