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

#include "../Source/Threading/Windows/WindowsProcessApi.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include "../Source/Threading/Windows/WindowsFileApi.h"

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Threading { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, ExecutableIsResolvedInWindowsDirectory) {

    // Normal executable name
    {
      std::wstring path;
      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad.exe");

      EXPECT_GT(path.length(), 16); // shortest possible valid path
      EXPECT_TRUE(WindowsFileApi::DoesFileExist(path));
    }

    // Executable name with .exe omitted
    {
      std::wstring path;

      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad");

      EXPECT_GT(path.length(), 16); // shortest possible valid path
      EXPECT_TRUE(WindowsFileApi::DoesFileExist(path));
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, CustomExtensionisRespected) {

    // Normal executable name
    {
      std::wstring path;
      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad.exe");

      EXPECT_GT(path.length(), 16); // shortest possible valid path
      EXPECT_TRUE(WindowsFileApi::DoesFileExist(path));
    }

    // Executable name with .exe omitted
    {
      std::wstring path;

      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"notepad.x");

      EXPECT_EQ(path, L"notepad.x");
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, ExecutableIsResolvedInSystemDirectory) {

    // Normal executable name
    {
      std::wstring path;
      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"ping.exe");

      EXPECT_GT(path.length(), 13); // shortest possible valid path
      EXPECT_TRUE(WindowsFileApi::DoesFileExist(path));
    }

    // Executable name with .exe omitted
    {
      std::wstring path;

      WindowsProcessApi::GetAbsoluteExecutablePath(path, L"ping");

      EXPECT_GT(path.length(), 13); // shortest possible valid path
      EXPECT_TRUE(WindowsFileApi::DoesFileExist(path));
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, ExecutableIsResolvedInOwnDirectory) {
    std::wstring path;
    WindowsProcessApi::GetAbsoluteExecutablePath(path, L"Nuclex.Support.Native.Tests.exe");

    EXPECT_GT(path.length(), 35); // shortest possible valid path
    EXPECT_TRUE(WindowsFileApi::DoesFileExist(path));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsProcessApiTest, RelativeWorkingDirectoryStartsInOwnDirectory) {
    std::wstring path;
    WindowsProcessApi::GetAbsoluteExecutablePath(path, L"Nuclex.Support.Native.Tests.exe");

    std::wstring directory;
    WindowsProcessApi::GetAbsoluteWorkingDirectory(directory, L".");

    // The directory may end with a \\. since we specified '.' as the target.
    // This isn't required, so we accept both variants. In case the dot is returned,
    // remove it so the path can be compared against the executable path.
    if(directory.length() >= 2) {
      if(directory[directory.length() - 1] == L'.') {
        if(directory[directory.length() - 2] == L'\\') {
          directory.resize(directory.length() - 2);
        } else {
          directory.resize(directory.length() - 1);
        }
      }
    }

    EXPECT_GT(directory.length(), 4); // shortest possible valid path
    EXPECT_NE(path.find(directory), std::wstring::npos);
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)