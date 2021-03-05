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

#include "../../Source/Helpers/WriteBuffer.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Directly exposes the write buffer's methods for unit tests</summary>
  class TestableWriteBuffer : public Nuclex::Storage::Helpers::WriteBuffer {
    public: using WriteBuffer::PutSingleByte;
    public: using WriteBuffer::Write;
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      WriteBuffer test;
      (void)test;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, TargetlessBufferHasNoRemainingBytes) {
    WriteBuffer test;
    EXPECT_EQ(test.CountRemainingBytes(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, NewBufferHasNoOverflowBytes) {
    WriteBuffer test;
    EXPECT_FALSE(test.HasOverflowBytes());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, TargetingNewBufferWithoutOverflowAddsNoData) {
    WriteBuffer test;

    std::uint8_t fixedBuffer[16];
    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 16), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, IsWritableWithoutFixedBuffer) {
    TestableWriteBuffer test;
    EXPECT_FALSE(test.HasOverflowBytes());

    const std::uint8_t data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    test.Write(data, 8);
    EXPECT_TRUE(test.HasOverflowBytes());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, FixedBufferIsFilledBeforeOverflow) {
    TestableWriteBuffer test;

    std::uint8_t fixedBuffer[8];
    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 8), 0U);
    EXPECT_EQ(test.CountRemainingBytes(), 8U);

    const std::uint8_t data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    test.Write(data, 8);

    EXPECT_EQ(test.CountRemainingBytes(), 0U);
    EXPECT_FALSE(test.HasOverflowBytes());

    for(std::size_t index = 0; index < 8; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 1));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, WriteCanBeSplitBetweenFixedBufferAndOverflow) {
    TestableWriteBuffer test;

    std::uint8_t fixedBuffer[8];
    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 8), 0U);
    EXPECT_EQ(test.CountRemainingBytes(), 8U);

    const std::uint8_t data[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    test.Write(data, 12);

    EXPECT_EQ(test.CountRemainingBytes(), 0U);
    EXPECT_TRUE(test.HasOverflowBytes());

    for(std::size_t index = 0; index < 8; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 1));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, OverflowIsWrittenToNewFixedBufferImmediately) {
    TestableWriteBuffer test;

    std::uint8_t fixedBuffer[8];
    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 8), 0U);
    EXPECT_EQ(test.CountRemainingBytes(), 8U);

    const std::uint8_t data[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    test.Write(data, 12);

    EXPECT_EQ(test.CountRemainingBytes(), 0U);
    EXPECT_TRUE(test.HasOverflowBytes());

    for(std::size_t index = 0; index < 8; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 1));
    }

    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 8), 4U);
    EXPECT_EQ(test.CountRemainingBytes(), 4U);
    EXPECT_FALSE(test.HasOverflowBytes());

    for(std::size_t index = 0; index < 4; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 9));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WriteBufferTest, OverflowCanCoverMultipleBuffers) {
    TestableWriteBuffer test;

    const std::uint8_t data[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    test.Write(data, 12);
    EXPECT_TRUE(test.HasOverflowBytes());

    std::uint8_t fixedBuffer[5];

    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 5), 5U);
    EXPECT_EQ(test.CountRemainingBytes(), 0U);
    for(std::size_t index = 0; index < 5; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 1));
    }

    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 5), 5U);
    EXPECT_EQ(test.CountRemainingBytes(), 0U);
    for(std::size_t index = 0; index < 5; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 6));
    }

    EXPECT_EQ(test.UseFixedBuffer(fixedBuffer, 5), 2U);
    EXPECT_EQ(test.CountRemainingBytes(), 3U);
    for(std::size_t index = 0; index < 2; ++index) {
      EXPECT_EQ(fixedBuffer[index], static_cast<std::uint8_t>(index + 11));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers
