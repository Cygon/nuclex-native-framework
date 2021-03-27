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

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/Text/StringConverter.h"
//#include <Shlwapi.h> // for ::PahtRemoveFileSpecW(), ::PathIsRelativeW(), PathAppendW()

namespace Nuclex { namespace Support { namespace Threading { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  bool WindowsFileApi::IsPathRelative(const std::wstring &path) {
    std::wstring::size_type length = path.length();
    if(length == 0) {
      return true;
    }

    if(length >= 3) {
      if((path[1] == L':') && (path[2] == L'\\')) {
        return false;
      }
    }

    return (path[0] != L'\\');
  }

  // ------------------------------------------------------------------------------------------- //
  
  void WindowsFileApi::AppendPath(std::wstring &path, const std::wstring &extra) {
    std::wstring::size_type length = path.length();
    if(length == 0) {
      path.assign(extra);
    } else {
      if(path[length - 1] != L'\\') {
        path.push_back(L'\\');
      }
      path.append(extra);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::RemoveFileFromPath(std::wstring &path) {
    std::wstring::size_type lastBackslashIndex = path.find_last_of(L'\\');
    if(lastBackslashIndex != std::wstring::npos) {
      path.resize(lastBackslashIndex + 1); // Keep the slash on
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool WindowsFileApi::HasExtension(const std::wstring &path) {
    std::wstring::size_type lastBackslashIndex = std::wstring::npos;
    std::wstring::size_type lastDotIndex = std::wstring::npos;
    {
      std::wstring::size_type length = path.length();
      for(std::wstring::size_type index = 0; index < length; ++index) {
        if(path[index] == L'.') {
          lastDotIndex = index;
        }
        if(path[index] == L'\\') {
          lastBackslashIndex = index;
        }
      } 
    }

    // No dots at all -> there is no filename extension anywhere
    if(lastDotIndex == std::wstring::npos) {
      return false;
    }

    // Dot present but no backlsashes -> the dot belongs to a filename extension
    if(lastBackslashIndex == std::wstring::npos) {
      return true;
    }

    // An extension is present if the last dot character belongs to
    // the last path component (the filename)!
    return (lastDotIndex > lastBackslashIndex);
  }

  // ------------------------------------------------------------------------------------------- //

  bool WindowsFileApi::DoesFileExist(const std::wstring &path) {
    DWORD attributes = ::GetFileAttributesW(path.c_str());
    if(attributes == INVALID_FILE_ATTRIBUTES) {
      DWORD lastErrorCode = ::GetLastError();
      if(lastErrorCode == ERROR_FILE_NOT_FOUND) {
        return false;
      }
      if(lastErrorCode == ERROR_PATH_NOT_FOUND) {
        return false;
      }

      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not check process exit code", lastErrorCode
      );
    }

    return (
      ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
      ((attributes & FILE_ATTRIBUTE_DEVICE) == 0)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::GetSystemDirectory(std::wstring &target) {
    target.resize(MAX_PATH);

    UINT result = ::GetSystemDirectoryW(target.data(), MAX_PATH);
    if(result == 0) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not get Windows system directory", errorCode
      );
    }

    target.resize(result);
  }

  // ------------------------------------------------------------------------------------------- //

  void WindowsFileApi::GetWindowsDirectory(std::wstring &target) {
    target.resize(MAX_PATH);

    UINT result = ::GetWindowsDirectoryW(target.data(), MAX_PATH);
    if(result == 0) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not get Windows directory", errorCode
      );
    }

    target.resize(result);
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
  BOOL WindowsFileApi::PathRemoveFileSpecW(LPWSTR pszPath) {
    if(pszPath == nullptr) {
      //::SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    // Go through the string, keeping track of the most recent backslash we found
    LPWSTR lastSlashAddress = nullptr;
    {
      LPWSTR pszCurrentCharacter = pszPath;
      while(*pszCurrentCharacter != 0) {
        if(*pszCurrentCharacter == L'\\') {
          lastSlashAddress = pszCurrentCharacter;
        }
        pszCurrentCharacter = ::CharNextW(pszCurrentCharacter);
      }
    }
    if(lastSlashAddress == nullptr) {
      return FALSE; // Path without backslashes...
    }

    std::wstring::size_type lastSlashIndex = lastSlashAddress - pszPath;
    if(lastSlashIndex < 3) {
      return FALSE; // Weirdo path ('a\') or a UNC path ('\\svr') without a filename
    }
    if(lastSlashIndex == 2) {
      bool isDriveLetter = (
        (pszPath[1] == L':') &&
        (pszPath[2] == L'\\')
      );
      if(isDriveLetter) {
        return FALSE; // It's a drive letter without a filename
      }
    }
    if(lastSlashIndex == 3) {
      bool isExtendedPath = (
        (pszPath[0] == L'\\') &&
        (pszPath[1] == L'\\') &&
        (pszPath[2] == L'?') &&
        (pszPath[3] == L'\\')
      );
      if(isExtendedPath) {
        return FALSE; // It's an extended path without a filename
      }
    }

    // We found a backslash that was not part of a UNC path, extended path or drive letter,
    // so write a 0 byte to end the string here. If the backslash was the final character,
    // it's removed, too. This is consistent and desired (ending a path with a backslash
    // indicates it is a directory, the next call will to up one directory, just as it would
    // happen if a filename was in the path).
    *lastSlashAddress = L'\0';
    return TRUE;
  }
#endif // defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
  BOOL WindowsFileApi::PathIsRelativeW(LPCWSTR pszPath) {
    if(pszPath == nullptr) {
      //::SetLastError(ERROR_INVALID_PARAMETER);
      return TRUE;
    }

    // Empty path -> relative.
    if(pszPath[0] == 0) {
      return TRUE;
    }

    // Path begins with backslash -> absolute (but bullshit since drive depends on CWD)
    if(pszPath[0] == L'\\') {
      return FALSE;
    }

    // Path begins with <letter>:\ -> absolute.
    if(pszPath[1] == L':') {
      return (pszPath[2] != L'\\');
    }

    // Path begins with no UNC prefix, extended prefix or drive letter -> relative.
    return TRUE;
  }
#endif // defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
  BOOL WindowsFileApi::PathAppendW(LPWSTR pszPath, LPCWSTR pszMore) {
    if((pszPath == nullptr) || (pszMore == nullptr)) {
      ::SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    // Skip initial dots (we don't need to deal with UTF-16 in this instance because a
    // '.' fits into one UTF-16 codepoint and the '.' are all at the beginning).
    //
    // We are we doing this? Dunno. It's the behavior of Microsoft's method. Clueless.
    LPWSTR pszEnd;
    {
      std::wstring::size_type skipCount = 0;
      while(pszPath[skipCount] == L'.') {
        ++skipCount;
      }

      pszEnd = pszPath;
      if(skipCount > 0) {
        while(pszEnd[skipCount] != 0) {
          pszEnd[0] = pszEnd[skipCount];
          ++pszEnd;
        }
      } else {
        while(*pszEnd != 0) {
          ++pszEnd;
        }
      }
    }

    // Calculate the length of the buffer so far
    // - pszPath is still points to the first character
    // - pszEnd points to the end of the path (either \0 or, if shifted, garbage)
    // If we need to bail, setting pszPath to \0 leaves the string sort of unmodified.
    // (except the removal of the ..)
    std::wstring::size_type pathLength = pszEnd - pszPath;
    pszPath = pszEnd;

    // If there is a previous character, go back by one character and check if
    // it is a backslash. We only append a backslash if there is a previous character and
    // it is not a backslash (if there's no previous character, the original path is empty)
    if(pathLength >= 1) {
      LPWSTR pszLast = ::CharPrevW(pszPath, pszEnd);
      if(*pszLast != L'\\') {
        if(pathLength >= 259) {
          *pszPath = 0; // This may write above 260 chars, but we know the string was longer
          ::SetLastError(ERROR_BUFFER_OVERFLOW);
          return FALSE;
        }
        *pszEnd = L'\\';
        ++pszEnd;
        ++pathLength;
      }
    }

    // Finally, append pszMore to pszPath
    while(pathLength < 260) {
      if(*pszMore == 0) {
        *pszEnd = 0;
        return TRUE; // We reached the end of pszMore before running out of buffer, success!
      }

      *pszEnd++ = *pszMore++;
      ++pathLength;
    }

    // Failure, buffer end reached and pszMore still had more characters to append.
    *pszPath = 0; 
    ::SetLastError(ERROR_BUFFER_OVERFLOW);
    return FALSE;
  }
#endif // defined(NUCLEX_SUPPORT_EMULATE_SHLWAPI)
  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)

#if 0
std::wstring WindowsProcessApi::combinePaths(std::wstring &path, const std::wstring &extra) {
  std::wstring result;
  result.resize(std::max(std::size_t(MAX_PATH), path.length() + extra.length() + 1));

  LPWSTR combined = ::PathCombineW(result.data(), path.c_str(), extra.c_str());
  if(combined == nullptr) {
    DWORD errorCode = ::GetLastError();
    Helpers::WindowsApi::ThrowExceptionForSystemError(u8"Could not combine paths", errorCode);
  }

  int pathLength = ::lstrlenW(result.c_str());
  result.resize(static_cast<std::size_t>(pathLength));

  return result;
}
#endif