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
#define NUCLEX_STORAGE_SOURCE 1

#include "Nuclex/Storage/MemoryBlob.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  TEST(MemoryBlobTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      MemoryBlob test;
      (void)test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemoryBlobTest, NewBlobIsEmpty) {
    MemoryBlob test;
    EXPECT_EQ(0, test.GetSize());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemoryBlobTest, GrowsWhenWrittenToEnd) {
    MemoryBlob test;
    test.WriteAt(0, u8"Hello", 5);

    EXPECT_EQ(5, test.GetSize());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemoryBlobTest, CanBeOverwritten) {
    MemoryBlob test;
    test.WriteAt(0, u8"Hello World", 11);
    EXPECT_EQ(11, test.GetSize());

    test.WriteAt(4, u8" Awaits", 7);
    EXPECT_EQ(11, test.GetSize());

    char message[11];
    test.ReadAt(0, message, 11);

    const char *expected = u8"Hell Awaits";
    for(std::size_t index = 0; index < 11; ++index) {
      EXPECT_EQ(message[index], expected[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(MemoryBlobTest, CanBeOverwrittenAndExtended) {
    MemoryBlob test;
    test.WriteAt(0, u8"Hello World", 11);
    EXPECT_EQ(11, test.GetSize());

    test.WriteAt(6, u8"Universe", 8);
    EXPECT_EQ(14, test.GetSize());

    char message[14];
    test.ReadAt(0, message, 14);

    const char *expected = u8"Hello Universe";
    for(std::size_t index = 0; index < 14; ++index) {
      EXPECT_EQ(message[index], expected[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage
