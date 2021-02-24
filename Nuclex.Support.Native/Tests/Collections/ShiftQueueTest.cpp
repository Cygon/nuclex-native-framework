#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#include "Nuclex/Support/Collections/ShiftBuffer.h"
#include "BufferTest.h"

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      ShiftBuffer<std::uint8_t> trivialTest;
      ShiftBuffer<TestItem> customTest;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, NewInstanceContainsNoItems) {
    ShiftBuffer<std::uint8_t> trivialTest;
    EXPECT_EQ(trivialTest.Count(), 0U);

    ShiftBuffer<TestItem> customTest;
    EXPECT_EQ(customTest.Count(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, StartsWithNonZeroDefaultCapacity) {
    ShiftBuffer<std::uint8_t> trivialTest;
    EXPECT_GT(trivialTest.GetCapacity(), 0U);

    ShiftBuffer<TestItem> customTest;
    EXPECT_GT(customTest.GetCapacity(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, CanStartWithCustomCapacity) {
    ShiftBuffer<std::uint8_t> trivialTest(512U);
    EXPECT_GE(trivialTest.GetCapacity(), 512U);

    ShiftBuffer<TestItem> customTest(512U);
    EXPECT_GE(customTest.GetCapacity(), 512U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, HasCopyConstructor) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    ShiftBuffer<std::uint8_t> copy(test);

    EXPECT_EQ(copy.Count(), 10U);

    std::uint8_t retrieved[10];
    copy.Read(retrieved, 10);

    EXPECT_EQ(copy.Count(), 0U);
    EXPECT_EQ(test.Count(), 10U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, HasMoveConstructor) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    ShiftBuffer<std::uint8_t> moved(std::move(test));

    EXPECT_EQ(moved.Count(), 10U);

    std::uint8_t retrieved[10];
    moved.Read(retrieved, 10);

    EXPECT_EQ(moved.Count(), 0U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ItemsCanBeAppended) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    test.Write(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ItemsCanBeAppendedWithMoveSemantics) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    test.Shove(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ItemsCanBeReadAndWritten) {
    ShiftBuffer<std::uint8_t> test;

    std::uint8_t items[128];
    for(std::size_t index = 0; index < 128; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }
    test.Write(items, 128);

    EXPECT_EQ(test.Count(), 128U);

    std::uint8_t retrieved[128];
    test.Read(retrieved, 128);

    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < 128; ++index) {
      EXPECT_EQ(retrieved[index], static_cast<std::uint8_t>(index));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, WritingInvokesCopyConstructor) {
    checkWritingInvokesCopyConstructor<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ShovingInvokesMoveConstructor) {
    checkShovingInvokesMoveConstructor<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, MoveSemanticsAreUsedWhenCapacityChanges) {
    checkMoveSemanticsAreUsedWhenCapacityChanges<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ReadUsesMoveSemanticsAndCallsDestructor) {
    checkReadUsesMoveSemanticsAndCallsDestructor<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, BufferDestroysLeftOverItemsWhenDestroyed) {
    checkBufferDestroysLeftOverItemsWhenDestroyed<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ExceptionDuringCapacityChangeCausesNoLeaks) {
    checkExceptionDuringCapacityChangeCausesNoLeaks<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ExceptionDuringWriteCausesNoLeaks) {
    checkExceptionDuringWriteCausesNoLeaks<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ExceptionDuringShoveCausesNoLeaks) {
    checkExceptionDuringShoveCausesNoLeaks<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ShiftBufferTest, ExceptionDuringReadCausesNoLeaks) {
    checkExceptionDuringReadCausesNoLeaks<ShiftBuffer>();
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
