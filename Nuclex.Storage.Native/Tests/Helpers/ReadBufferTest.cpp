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
#define NUCLEX_STORAGE_SOURCE 1

#include "../../Source/Helpers/ReadBuffer.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      ReadBuffer test;
      (void)test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, StartsOutEmpty) {
    ReadBuffer test;

    EXPECT_EQ(0U, test.CountCachedBytes());
    EXPECT_EQ(0U, test.CountAvailableBytes());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, AssignedFixedBufferInfluencesSize) {
    const std::uint8_t data[16] = { 0 };

    ReadBuffer test;

    EXPECT_EQ(0U, test.CountCachedBytes());
    EXPECT_EQ(0U, test.CountAvailableBytes());

    test.UseFixedBuffer(data, 16);

    EXPECT_EQ(0U, test.CountCachedBytes());
    EXPECT_EQ(16U, test.CountAvailableBytes());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, CanCacheFixedBuffer) {
    const std::uint8_t data[10] = { 0 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 10);

    EXPECT_EQ(0U, test.CountCachedBytes());
    EXPECT_EQ(10U, test.CountAvailableBytes());

    test.CacheFixedBufferContents();

    EXPECT_EQ(10U, test.CountCachedBytes());
    EXPECT_EQ(10U, test.CountAvailableBytes());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, CachedDataCanBeAccessed) {
    const std::uint8_t data[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 10);
    test.CacheFixedBufferContents();

    const std::uint8_t *cache = test.GetCachedData();

    for(std::size_t index = 0; index < 10U; ++index) {
      EXPECT_EQ(cache[index], data[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, CachedDataCanBeSkipped) {
    const std::uint8_t data[10] = { 0 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 10);
    test.CacheFixedBufferContents();

    test.SkipCachedBytes(5);
    EXPECT_EQ(5U, test.CountCachedBytes());
    EXPECT_EQ(5U, test.CountAvailableBytes());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, AccessedCachedDataStartsAfterLastRead) {
    const std::uint8_t data[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 10);
    test.CacheFixedBufferContents();
    test.SkipCachedBytes(5);

    EXPECT_EQ(5U, test.CountCachedBytes());
    EXPECT_EQ(5U, test.CountAvailableBytes());

    const std::uint8_t *cache = test.GetCachedData();

    for(std::size_t index = 0; index < 5U; ++index) {
      EXPECT_EQ(cache[index], data[index + 5]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, DataCanBeReadFromCache) {
    const std::uint8_t data[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 10);
    test.CacheFixedBufferContents();

    std::uint8_t copy[10];

    test.Read(copy, 10);

    for(std::size_t index = 0; index < 5U; ++index) {
      EXPECT_EQ(copy[index], data[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, DataCanBeReadFromFixedBuffer) {
    const std::uint8_t data[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 10);

    std::uint8_t copy[10];

    test.Read(copy, 10);

    for(std::size_t index = 0; index < 5U; ++index) {
      EXPECT_EQ(copy[index], data[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ReadBufferTest, DataCanBeReadMixed) {
    const std::uint8_t data[10] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

    ReadBuffer test;
    test.UseFixedBuffer(data, 5);
    test.CacheFixedBufferContents();
    test.UseFixedBuffer(data + 5, 5);

    std::uint8_t copy[10];

    test.Read(copy, 10);

    for(std::size_t index = 0; index < 5U; ++index) {
      EXPECT_EQ(copy[index], data[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers
