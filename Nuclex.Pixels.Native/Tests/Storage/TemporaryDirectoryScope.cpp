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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "TemporaryDirectoryScope.h"
#include "../Source//Storage/Utf8/checked.h"

#include <vector>

#if defined(NUCLEX_PIXELS_LINUX)
#include <ftw.h>
#include <unistd.h>
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Appends the user's/system's preferred temp directory to a path</summary>
  /// <param name="path">Path vector the temp directory will be appended to</param>
  void appendTempDirectory(std::vector<char> &path) {

    // Obtain the most likely system temp directory
    const char *tempDirectory = ::getenv(u8"TMPDIR");
    if(tempDirectory == nullptr) {
      tempDirectory = ::getenv(u8"TMP");
      if(tempDirectory == nullptr) {
        tempDirectory = ::getenv(u8"TEMP");
        if(tempDirectory == nullptr) {
          // This is safe (part of the file system standard and Linux standard base),
          // but we wanted to honor any possible user preferences first.
          tempDirectory = u8"/tmp";
        }
      }
    }

    // Append the temporary directory to the path vector
    while(*tempDirectory != 0) {
      path.push_back(*tempDirectory);
      ++tempDirectory;
    }

  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Creates a unique temporary directory and returns its path</summary>
  /// <returns>The path to the unique temporary directory</returns>
  std::string createTemporaryDirectory() {
    std::vector<char> temporaryDirectoryPath;
    temporaryDirectoryPath.reserve(16 + 32);

    // First, look up the system's temporary directory
    appendTempDirectory(temporaryDirectoryPath);

    // Append a path component separator if appendTempDirectory() didn't provide one
    // and the temp path isn't empty (i.e. user wants temp files loaded into CWD ?_?)
    if(temporaryDirectoryPath.size() > 0) {
      if(temporaryDirectoryPath[temporaryDirectoryPath.size() - 1] != '/') {
        temporaryDirectoryPath.push_back('/');
      }
    }

    // Then append our directory name template to it
    const char directoryNameTemplate[] = u8"nuclex-pixels-unittest-XXXXXX";
    {
      const char *iterator = directoryNameTemplate;
      while(*iterator != 0) {
        temporaryDirectoryPath.push_back(*iterator);
        ++iterator;
      }
    }

    // Zero-terminate the string for the C function call
    temporaryDirectoryPath.push_back(0);

    // Let mkdtemp() sort out a unique directory name for us (and create it!)
    const char *directoryName = ::mkdtemp(&temporaryDirectoryPath[0]);
    if(directoryName == nullptr) {
      throw std::runtime_error("mkdtemp() failed.");
    }

    return std::string(&temporaryDirectoryPath[0], temporaryDirectoryPath.size() - 1);
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Creates a unique temporary file and returns its path</summary>
  /// <returns>The absolute path of a new, unique temporary file</returns>
  std::wstring createTemporaryFile() {
    std::vector<wchar_t> tempPathBuffer;
    {
      tempPathBuffer.resize(MAX_PATH);
      DWORD tempPathLength = ::GetTempPathW(MAX_PATH, &tempPathBuffer[0]);
      if(tempPathLength == 0) {
        throw std::runtime_error("GetTempPathW() failed");
      }
      tempPathBuffer.resize(tempPathLength);
    }

    // Zero-terminate the string for the Windows API function
    tempPathBuffer.push_back(0);

    std::vector<wchar_t> tempFilePathBuffer;
    {
      tempFilePathBuffer.resize(MAX_PATH);

      UINT tempFilePathIndex = ::GetTempFileNameW(
        &tempPathBuffer[0],
        L"np-",
        0,
        &tempFilePathBuffer[0]
      );
      if(tempFilePathIndex == 0) {
        throw std::runtime_error("GetTempFileNameW() failed");
      }
    }

    return std::wstring(&tempFilePathBuffer[0]);
  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Creates a temporary directory for a unique temporary file placeholder</summary>
  /// <param name="temporaryFilePath">File that Windows created as a placeholder</param>
  /// <returns>The absolute path of a new temporary directory</returns>
  std::wstring createTemporaryDirectory(const std::wstring &temporaryFilePath) {
    std::wstring tempDirectoryPath = temporaryFilePath;
    tempDirectoryPath.append(L".dir");

    BOOL succeeded = ::CreateDirectoryW(tempDirectoryPath.c_str(), nullptr);
    if(!succeeded) {
      throw std::runtime_error("CreateDirectoryW() failed");
    }

    return tempDirectoryPath;
  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Callback for the file-tree-walk function that deletes files</summary>
  /// <param name="path">Path of the current file or directory</param>
  /// <param name="nodeStat">Stat structure of the current file or directory</param>
  /// <param name="typeflag">Type of the current node (FTW_F or FTW_D)</param>
  /// <param name="ftwinfo">Folder depth an other info provided by nftw()</param>
  /// <returns>0 on success, a standard Posix error code in case of failure</returns>
  int removeFileOrDirectoryCallback(
    const char *path, const struct stat *nodeStat, int typeflag, struct FTW *ftwinfo
  ) {
    (void)nodeStat;
    (void)typeflag;
    (void)ftwinfo;

    return ::remove(path);
  }

  /// <summary>Recursively deletes the specified directory and all its contents</summary>
  /// <param name="path">Absolute path of the directory that will be deleted</param>
  void deleteDirectoryWithContents(const std::string &path) {
    int result = ::nftw(path.c_str(), removeFileOrDirectoryCallback, 64, FTW_DEPTH | FTW_PHYS);
    if(result != 0) {
      throw std::runtime_error("nftw() or remove() failed");
    }
  }

#endif // !defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>Automatically closes a search handle upon destruction</summary>
  class SearchHandleScope {

    /// <summary>Initializes a new search handle closer</summary>
    /// <param name="searchHandle">Search handle that will be closed on destruction</param>
    public: SearchHandleScope(HANDLE searchHandle) :
      searchHandle(searchHandle) {}

    /// <summary>Closes the search handle</summary>
    public: ~SearchHandleScope() {
      ::FindClose(this->searchHandle);
    }

    /// <summary>Search handle that will be closed when the instance is destroyed</summary>
    private: HANDLE searchHandle;

  };

  /// <summary>Recursively deletes the specified directory and all its contents</summary>
  /// <param name="path">Absolute path of the directory that will be deleted</param>
  /// <remarks>
  ///   The path must not be terminated with a path separator.
  /// </remarks>
  void deleteDirectoryWithContents(const std::wstring &path) {
    static const std::wstring allFilesMask(L"\\*");

    WIN32_FIND_DATAW findData;

    // First, delete the contents of the directory, recursively for subdirectories
    std::wstring searchMask = path + allFilesMask;
    HANDLE searchHandle = ::FindFirstFileExW(
      searchMask.c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, nullptr, 0
    );
    if(searchHandle == INVALID_HANDLE_VALUE) {
      DWORD lastError = ::GetLastError();
      if(lastError != ERROR_FILE_NOT_FOUND) { // or ERROR_NO_MORE_FILES, ERROR_NOT_FOUND?
        throw std::runtime_error("Could not start directory enumeration");
      }
    }

    // Did this directory have any contents? If so, delete them first
    if(searchHandle != INVALID_HANDLE_VALUE) {
      SearchHandleScope scope(searchHandle);
      for(;;) {

        // Do not process the obligatory '.' and '..' directories
        if(findData.cFileName[0] != '.') {
          bool isDirectory =
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0);

          // Subdirectories need to be handled by deleting their contents first
          std::wstring filePath = path + L'\\' + findData.cFileName;
          if(isDirectory) {
            deleteDirectoryWithContents(filePath);
          } else {
            BOOL result = ::DeleteFileW(filePath.c_str());
            if(result == FALSE) {
              throw std::runtime_error("Could not delete file");
            }
          }
        }

        // Advance to the next file in the directory
        BOOL result = ::FindNextFileW(searchHandle, &findData);
        if(result == FALSE) {
          DWORD lastError = ::GetLastError();
          if(lastError != ERROR_NO_MORE_FILES) {
            throw std::runtime_error("Error enumerating directory");
          }
          break; // All directory contents enumerated and deleted
        }

      } // for
    }

    // The directory is empty, we can now safely remove it
    BOOL result = ::RemoveDirectoryW(path.c_str());
    if(result == FALSE) {
      throw std::runtime_error("Could not remove directory");
    }
  }

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if defined(NUCLEX_PIXELS_WIN32)

  /// <summary>RAII helper that closes a file handle upon destruction</summary>
  class FileHandleScope {

    /// <summary>Initializes a new file closer for the specified file handle</summary>
    /// <param name="fileHandle">Handle of the file that will be closed</param>
    public: FileHandleScope(HANDLE fileHandle) :
      fileHandle(fileHandle) {}

    /// <summary>Closes the file</summary>
    public: ~FileHandleScope() {
      ::CloseHandle(this->fileHandle);
    }

    /// <summary>Handle of the file that will be clsoed</summary>
    private: HANDLE fileHandle;

  };

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

#if !defined(NUCLEX_PIXELS_WIN32)

  /// <summary>RAII helper that closes a file pointer upon destruction</summary>
  class FilePointerScope {

    /// <summary>Initializes a new file closer for the specified file pointer</summary>
    /// <param name="filePointer">Pointer of the file that will be closed</param>
    public: FilePointerScope(FILE *filePointer) :
      filePointer(filePointer) {}

    /// <summary>Closes the file</summary>
    public: ~FilePointerScope() {
      ::fclose(this->filePointer);
    }

    /// <summary>Pointer for the file that will be clsoed</summary>
    private: FILE *filePointer;

  };

#endif // defined(NUCLEX_PIXELS_WIN32)

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::TemporaryDirectoryScope() {
#if defined(NUCLEX_PIXELS_WIN32)
    this->tempFilePath = createTemporaryFile();
    this->tempDirectoryPath = createTemporaryDirectory(this->tempFilePath);

    std::vector<char> utf8Characters;
    utf8Characters.reserve(this->tempDirectoryPath.length());
    utf8::utf16to8(
      this->tempDirectoryPath.begin(), this->tempDirectoryPath.end(),
      std::back_inserter(utf8Characters)
    );

    this->path.assign(&utf8Characters[0], utf8Characters.size());
#else
    this->path = createTemporaryDirectory();
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  TemporaryDirectoryScope::~TemporaryDirectoryScope() {
#if defined(NUCLEX_PIXELS_WIN32)
    deleteDirectoryWithContents(this->tempDirectoryPath);
    ::DeleteFileW(this->tempFilePath.c_str());
#else
    deleteDirectoryWithContents(this->path);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::string TemporaryDirectoryScope::GetPath(const std::string &filename) const {
    std::string::size_type pathLength = this->path.length();
#if defined(NUCLEX_PIXELS_WIN32)
    if(this->path[pathLength - 1] == '\\') {
      return this->path + filename;
    } else {
      return this->path + '\\' + filename;
    }
#else
    if(this->path[pathLength - 1] == '/') {
      return this->path + filename;
    } else {
      return this->path + '/' + filename;
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::string TemporaryDirectoryScope::ReadFullFile(const std::string &filename) const {
    std::string filePath = GetPath(filename);

#if defined(NUCLEX_PIXELS_WIN32)
    std::wstring utf16FilePath;
    {
      std::vector<wchar_t> utf16Characters;
      utf16Characters.reserve(utf16FilePath.length());
      utf8::utf8to16(
        filePath.begin(), filePath.end(), std::back_inserter(utf16Characters)
      );
      utf16FilePath.assign(&utf16Characters[0], utf16Characters.size());
    }

    HANDLE fileHandle = ::CreateFileW(
      utf16FilePath.c_str(),
      FILE_READ_DATA, FILE_SHARE_READ, nullptr,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("CreateFileW() failed");
    }
    {
      FileHandleScope fileScope(fileHandle);
      DWORD fileSize = ::GetFileSize(fileHandle, nullptr);
      if(fileSize == INVALID_FILE_SIZE) {
        throw std::runtime_error("GetFileSize() failed");
      }
      std::vector<char> fileContents;
      fileContents.resize(fileSize);

      BOOL succeeded = ::ReadFile(
        fileHandle, &fileContents[0], fileSize, nullptr, nullptr
      );
      if(!succeeded) {
        throw std::runtime_error("ReadFileEx() failed");
      }

      return std::string(&fileContents[0], fileContents.size());
    }
#else
    FILE *filePointer = ::fopen(filePath.c_str(), "rb");
    if(filePointer == nullptr) {
      throw std::runtime_error("fopen() failed");
    }
    {
      FilePointerScope fileScope(filePointer);
      struct stat fileStatus;
      int result = ::stat(filePath.c_str(), &fileStatus);
      if(result != 0) {
        throw std::runtime_error("stat() failed");
      }

      std::vector<char> fileContents;
      fileContents.resize(fileStatus.st_size);
      std::size_t readByteCount = ::fread(
        &fileContents[0], 1, fileStatus.st_size, filePointer
      );
      if(readByteCount != fileContents.size()) {
        throw std::runtime_error("fread() failed");
      }
      
      return std::string(&fileContents[0], fileContents.size());
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void TemporaryDirectoryScope::WriteFullFile(
    const std::string &filename, const std::string &contents
  ) const {
    std::string filePath = GetPath(filename);
#if defined(NUCLEX_PIXELS_WIN32)
    std::wstring utf16FilePath;
    {
      std::vector<wchar_t> utf16Characters;
      utf16Characters.reserve(utf16FilePath.length());
      utf8::utf8to16(
        filePath.begin(), filePath.end(), std::back_inserter(utf16Characters)
      );
      utf16FilePath.assign(&utf16Characters[0], utf16Characters.size());
    }

    HANDLE fileHandle = ::CreateFileW(
      utf16FilePath.c_str(),
      FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE, 0, nullptr,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr
    );
    if(fileHandle == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("CreateFileW() failed");
    }
    {
      FileHandleScope fileScope(fileHandle);

      BOOL succeeded = ::WriteFile(
        fileHandle, contents.c_str(), static_cast<DWORD>(contents.length()), nullptr, nullptr
      );
      if(!succeeded) {
        throw std::runtime_error("WriteFileEx() failed");
      }
    }
#else
    FILE *filePointer = ::fopen(filePath.c_str(), "wb");
    if(filePointer == nullptr) {
      throw std::runtime_error("fopen() failed");
    }
    {
      FilePointerScope fileScope(filePointer);
      std::size_t writtenByteCount = ::fwrite(
        contents.c_str(), 1, contents.size(), filePointer
      );
      if(writtenByteCount != contents.size()) {
        throw std::runtime_error("fwrite() failed");
      }
    }
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
