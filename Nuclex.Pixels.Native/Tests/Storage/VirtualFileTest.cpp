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

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileAccessError.h"
#include <gtest/gtest.h>

#include "TemporaryDirectoryScope.h"

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  TEST(VirtualFileTest, CanWriteToRealFile) {
    TemporaryDirectoryScope temporaryDirectory;

    std::string expectedFileContents = u8"Hello World!";

    // Write something into a file using the VirtualFile's convenience methods
    std::string testPath = temporaryDirectory.GetPath(u8"write-test.tmp");
    {
      std::unique_ptr<VirtualFile> writableFile = VirtualFile::OpenRealFileForWriting(testPath);
      writableFile->WriteAt(
        0, expectedFileContents.length(),
        reinterpret_cast<const std::uint8_t *>( expectedFileContents.c_str())
      );
    }

    // Now read the file back using an alternative file system implementation
    std::string actualFileContents = temporaryDirectory.ReadFullFile(
      u8"write-test.tmp"
    );

    EXPECT_EQ(actualFileContents, expectedFileContents);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VirtualFileTest, CanReadFromRealFile) {
    TemporaryDirectoryScope temporaryDirectory;

    std::string actualFileContents = u8"Hello World!";

    temporaryDirectory.WriteFullFile(u8"read-test.tmp", actualFileContents);

    // Write something into a file using the VirtualFile's convenience methods
    std::string testPath = temporaryDirectory.GetPath(u8"read-test.tmp");
    std::vector<char> buffer;
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(testPath);
      buffer.resize(file->GetSize());
      file->ReadAt(0, buffer.size(), reinterpret_cast<std::uint8_t *>(&buffer[0]));
    }

    std::string claimedFileContents = std::string(&buffer[0], buffer.size());

    EXPECT_EQ(claimedFileContents, actualFileContents);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VirtualFileTest, AccessingNonExistingFileThrowsError) {
    TemporaryDirectoryScope temporaryDirectory;
    
    EXPECT_THROW(
      VirtualFile::OpenRealFileForReading(u8"does-not-exist.tmp"),
      Errors::FileAccessError
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VirtualFileTest, ReadingOutOfBoundsThrowsError) {
    TemporaryDirectoryScope temporaryDirectory;

    temporaryDirectory.WriteFullFile(u8"read-test.tmp", "0123456789");

    std::string testPath = temporaryDirectory.GetPath(u8"read-test.tmp");
    std::uint8_t buffer[8];
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(testPath);
      file->ReadAt(0, 8, buffer);

      EXPECT_THROW(
        file->ReadAt(8, 3, buffer),
        Errors::FileAccessError
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VirtualFileTest, WritingWithGapBeyondEndOfFileThrowsError) {
    TemporaryDirectoryScope temporaryDirectory;

    temporaryDirectory.WriteFullFile(u8"read-test.tmp", "0123456789");

    std::string testPath = temporaryDirectory.GetPath(u8"read-test.tmp");
    std::uint8_t buffer[8] = { '0', '1', '2', '3', '4', '5', '6', '7' };
    {
      std::unique_ptr<VirtualFile> file = VirtualFile::OpenRealFileForWriting(testPath);
      file->WriteAt(0, 8, buffer);
      file->WriteAt(8, 8, buffer);

      EXPECT_THROW(
        file->WriteAt(17, 8, buffer),
        Errors::FileAccessError
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(VirtualFileTest, FilePartsCanBeReread) {
    TemporaryDirectoryScope temporaryDirectory;

    temporaryDirectory.WriteFullFile(u8"read-test.tmp", "0123456789");

    std::string testPath = temporaryDirectory.GetPath(u8"read-test.tmp");
    std::uint8_t buffer[8] = { '0', '1', '2', '3', '4', '5', '6', '7' };
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(testPath);
      file->ReadAt(0, 6, buffer);
      file->ReadAt(6, 4, buffer);
      file->ReadAt(2, 6, buffer);

      EXPECT_EQ(buffer[0], '2');
      EXPECT_EQ(buffer[1], '3');
      EXPECT_EQ(buffer[2], '4');
      EXPECT_EQ(buffer[3], '5');
      EXPECT_EQ(buffer[4], '6');
      EXPECT_EQ(buffer[5], '7');
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
