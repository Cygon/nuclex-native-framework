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

#include "../Source/Threading/Posix/PosixFileApi.h"

#if !defined(NUCLEX_SUPPORT_WIN32)

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Threading { namespace Posix {

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixFileApiTest, DetectsIfPathIsRelative) {
    EXPECT_TRUE(PosixFileApi::IsPathRelative(u8"relative/path"));
    EXPECT_TRUE(PosixFileApi::IsPathRelative(u8"~file"));
    EXPECT_FALSE(PosixFileApi::IsPathRelative(u8"/absolute/path"));
    EXPECT_FALSE(PosixFileApi::IsPathRelative(u8"~/file"));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixFileApiTest, CanAppendPath) {
    std::string testPath = u8"/home";

    PosixFileApi::AppendPath(testPath, u8"nobody");
    EXPECT_EQ(testPath, u8"/home/nobody");

    testPath.push_back('/');
    PosixFileApi::AppendPath(testPath, u8".bashrc");
    EXPECT_EQ(testPath, u8"/home/nobody/.bashrc");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixFileApiTest, CanRemoveFilenameFromPath) {
    std::string testPath = u8"/home/nobody/random-file";
    PosixFileApi::RemoveFileFromPath(testPath);
    EXPECT_EQ(testPath, u8"/home/nobody/");
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixFileApiTest, CanCheckIfFileExists) {
    EXPECT_TRUE(PosixFileApi::DoesFileExist(u8"/proc/version"));

    EXPECT_FALSE(PosixFileApi::DoesFileExist(u8"/testing/this/does/not/exist"));
    EXPECT_FALSE(PosixFileApi::DoesFileExist(u8"/testing-this-does-not-exist"));
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Support::Threading::Posix

#endif // !defined(NUCLEX_SUPPORT_WIN32)
