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

#include "../../Source/Platform/LinuxFileApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/TemporaryFileScope.h"
#include "Nuclex/Support/ScopeGuard.h"

#include <unistd.h> // for ::access()
#include <sys/stat.h> // for ::stat()

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanOpenFileForReading) {
    TemporaryFileScope tempFile;
    tempFile.SetFileContents(u8"Hello World");

    int fileDescriptor = LinuxFileApi::OpenFileForReading(tempFile.GetPath().c_str());
    ASSERT_NE(fileDescriptor, -1);

    ::close(fileDescriptor);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanOpenFileForWriting) {
    TemporaryFileScope tempFile;

    {
      int fileDescriptor = LinuxFileApi::OpenFileForWriting(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      std::uint8_t data[] = { 123 };
      ::ssize_t writtenByteCount = ::write(fileDescriptor, data, 1);
      ASSERT_EQ(writtenByteCount, 1);
    }

    struct ::stat fileStatus;
    int errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(errorNumber, 0);
    EXPECT_EQ(fileStatus.st_size, 1);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanReadFromFile) {
    TemporaryFileScope tempFile;
    tempFile.SetFileContents(u8"Hello World");

    {
      int fileDescriptor = LinuxFileApi::OpenFileForReading(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      std::uint8_t buffer[11];
      std::size_t readByteCount = LinuxFileApi::Read(fileDescriptor, buffer, 11);
      ASSERT_EQ(readByteCount, 11U);

      EXPECT_EQ(buffer[0], u8'H');
      EXPECT_EQ(buffer[1], u8'e');
      EXPECT_EQ(buffer[2], u8'l');
      EXPECT_EQ(buffer[3], u8'l');
      EXPECT_EQ(buffer[4], u8'o');
      EXPECT_EQ(buffer[5], u8' ');
      EXPECT_EQ(buffer[6], u8'W');
      EXPECT_EQ(buffer[7], u8'o');
      EXPECT_EQ(buffer[8], u8'r');
      EXPECT_EQ(buffer[9], u8'l');
      EXPECT_EQ(buffer[10], u8'd');
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, CanWriteToFile) {
    TemporaryFileScope tempFile;

    {
      int fileDescriptor = LinuxFileApi::OpenFileForWriting(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      std::uint8_t data[] = { 1, 2, 3, 4, 5 };
      LinuxFileApi::Write(fileDescriptor, data, 5);
    }

    struct ::stat fileStatus;
    int errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
    ASSERT_EQ(errorNumber, 0);
    EXPECT_EQ(fileStatus.st_size, 5);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(LinuxFileApiTest, FileCanBeTruncated) {
    TemporaryFileScope tempFile;
    tempFile.SetFileContents(u8"Hello World");

    {
      int fileDescriptor = LinuxFileApi::OpenFileForWriting(tempFile.GetPath().c_str());
      ASSERT_NE(fileDescriptor, -1);
      ON_SCOPE_EXIT {
        ::close(fileDescriptor);
      };

      struct ::stat fileStatus;
      int errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
      ASSERT_EQ(errorNumber, 0);
      EXPECT_EQ(fileStatus.st_size, 11);

      LinuxFileApi::SetLength(fileDescriptor, 5);

      errorNumber = ::stat(tempFile.GetPath().c_str(), &fileStatus);
      ASSERT_EQ(errorNumber, 0);
      EXPECT_EQ(fileStatus.st_size, 5);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
