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

#include "../Source/Threading/Windows/WindowsFileApi.h"

#if defined(NUCLEX_SUPPORT_WIN32)

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Threading { namespace Windows {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, DetectsIfPathIsRelative) {
    EXPECT_TRUE(WindowsFileApi::IsPathRelative(L"Relative\\Path.txt"));
    EXPECT_TRUE(WindowsFileApi::IsPathRelative(L"R:elative\\Path.txt"));
    EXPECT_FALSE(WindowsFileApi::IsPathRelative(L"\\Absolute\\Path"));
    EXPECT_FALSE(WindowsFileApi::IsPathRelative(L"A:\\bsolute\\Path"));
    EXPECT_FALSE(WindowsFileApi::IsPathRelative(L"\\\\UNC\\Path"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, CanAppendPath) {
    std::wstring testPath = L"C:\\Users";

    WindowsFileApi::AppendPath(testPath, L"Guest");
    EXPECT_EQ(testPath, L"C:\\Users\\Guest");

    testPath.push_back(L'\\');
    WindowsFileApi::AppendPath(testPath, L"Documents");
    EXPECT_EQ(testPath, L"C:\\Users\\Guest\\Documents");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, CanRemoveFilenameFromPath) {
    std::wstring testPath = L"C:\\ProgramData\\RandomFile.txt";
    WindowsFileApi::RemoveFileFromPath(testPath);
    EXPECT_EQ(testPath, L"C:\\ProgramData\\");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, CanDetectFilenameExtensionPresence) {
    EXPECT_TRUE(WindowsFileApi::HasExtension(L"C:\\TestFile.txt"));
    EXPECT_FALSE(WindowsFileApi::HasExtension(L"C:\\TestFile"));
    EXPECT_TRUE(WindowsFileApi::HasExtension(L"C:\\Directory.dir\\TestFile.txt"));
    EXPECT_FALSE(WindowsFileApi::HasExtension(L"C:\\Directory.dir\\TestFile"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, CanCheckIfFileExists) {
    std::wstring explorerPath;
    WindowsFileApi::GetWindowsDirectory(explorerPath);
    WindowsFileApi::AppendPath(explorerPath, L"explorer.exe");
    EXPECT_TRUE(WindowsFileApi::DoesFileExist(explorerPath));

    EXPECT_FALSE(WindowsFileApi::DoesFileExist(L"C:\\This\\Does\\Not\\Exist"));
    EXPECT_FALSE(WindowsFileApi::DoesFileExist(L"C:\\ThisDoesNotExist.txt"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, CanLocateWindowsDirectory) {
    std::wstring testPath;
    WindowsFileApi::GetWindowsDirectory(testPath);

    EXPECT_GE(testPath.length(), 4); // Shortest possible
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsFileApiTest, CanLocateSystemDirectory) {
    std::wstring testPath;
    WindowsFileApi::GetSystemDirectory(testPath);

    EXPECT_GE(testPath.length(), 6); // Shortest possible
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Windows

#endif // defined(NUCLEX_SUPPORT_WIN32)
