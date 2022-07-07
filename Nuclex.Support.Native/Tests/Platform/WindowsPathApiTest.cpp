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

#include "../../Source/Platform/WindowsPathApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, DetectsIfPathIsRelative) {
    EXPECT_TRUE(WindowsPathApi::IsPathRelative(L"Relative\\Path.txt"));
    EXPECT_TRUE(WindowsPathApi::IsPathRelative(L"R:elative\\Path.txt"));
    EXPECT_FALSE(WindowsPathApi::IsPathRelative(L"\\Absolute\\Path"));
    EXPECT_FALSE(WindowsPathApi::IsPathRelative(L"A:\\bsolute\\Path"));
    EXPECT_FALSE(WindowsPathApi::IsPathRelative(L"\\\\UNC\\Path"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanAppendPath) {
    std::wstring testPath = L"C:\\Users";

    WindowsPathApi::AppendPath(testPath, L"Guest");
    EXPECT_EQ(testPath, L"C:\\Users\\Guest");

    testPath.push_back(L'\\');
    WindowsPathApi::AppendPath(testPath, L"Documents");
    EXPECT_EQ(testPath, L"C:\\Users\\Guest\\Documents");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanRemoveFilenameFromPath) {
    std::wstring testPath = L"C:\\ProgramData\\RandomFile.txt";
    WindowsPathApi::RemoveFileFromPath(testPath);
    EXPECT_EQ(testPath, L"C:\\ProgramData\\");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanDetectFilenameExtensionPresence) {
    EXPECT_TRUE(WindowsPathApi::HasExtension(L"C:\\TestFile.txt"));
    EXPECT_FALSE(WindowsPathApi::HasExtension(L"C:\\TestFile"));
    EXPECT_TRUE(WindowsPathApi::HasExtension(L"C:\\Directory.dir\\TestFile.txt"));
    EXPECT_FALSE(WindowsPathApi::HasExtension(L"C:\\Directory.dir\\TestFile"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanCheckIfFileExists) {
    std::wstring explorerPath;
    WindowsPathApi::GetWindowsDirectory(explorerPath);
    WindowsPathApi::AppendPath(explorerPath, L"explorer.exe");
    EXPECT_TRUE(WindowsPathApi::DoesFileExist(explorerPath));

    EXPECT_FALSE(WindowsPathApi::DoesFileExist(L"C:\\This\\Does\\Not\\Exist"));
    EXPECT_FALSE(WindowsPathApi::DoesFileExist(L"C:\\ThisDoesNotExist.txt"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanLocateWindowsDirectory) {
    std::wstring testPath;
    WindowsPathApi::GetWindowsDirectory(testPath);

    EXPECT_GE(testPath.length(), 4); // Shortest possible
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsPathApiTest, CanLocateSystemDirectory) {
    std::wstring testPath;
    WindowsPathApi::GetSystemDirectory(testPath);

    EXPECT_GE(testPath.length(), 6); // Shortest possible
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
