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

#include "Nuclex/Support/Collections/RingQueue.h"
#include "BufferTest.h"

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      RingQueue<std::uint8_t> trivialTest;
    );
    EXPECT_NO_THROW(
      RingQueue<TestItem> complexTest;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, NewInstanceContainsNoItems) {
    RingQueue<std::uint8_t> trivialTest;
    EXPECT_EQ(trivialTest.Count(), 0U);

    RingQueue<TestItem> complexTest;
    EXPECT_EQ(complexTest.Count(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, StartsWithNonZeroDefaultCapacity) {
    RingQueue<std::uint8_t> trivialTest;
    EXPECT_GT(trivialTest.GetCapacity(), 0U);

    RingQueue<TestItem> complexTest;
    EXPECT_GT(complexTest.GetCapacity(), 0U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, CanStartWithCustomCapacity) {
    RingQueue<std::uint8_t> trivialTest(512U);
    EXPECT_GE(trivialTest.GetCapacity(), 512U);

    RingQueue<TestItem> complexTest(512U);
    EXPECT_GE(complexTest.GetCapacity(), 512U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, HasCopyConstructor) {
    RingQueue<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    RingQueue<std::uint8_t> copy(test);

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

  TEST(RingQueueTest, HasMoveConstructor) {
    RingQueue<std::uint8_t> test;

    std::uint8_t items[10] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U };
    test.Write(items, 10);

    EXPECT_EQ(test.Count(), 10U);

    RingQueue<std::uint8_t> moved(std::move(test));

    EXPECT_EQ(moved.Count(), 10U);

    std::uint8_t retrieved[10];
    moved.Read(retrieved, 10);

    EXPECT_EQ(moved.Count(), 0U);

    for(std::size_t index = 0; index < 10; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ItemsCanBeAppended) {
    RingQueue<std::uint8_t> test;

    std::uint8_t items[128];
    test.Write(items, 128);

    EXPECT_EQ(test.Count(), 128U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ItemsCanBeAppendedAndDequeued) {
    RingQueue<std::uint8_t> test;

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

  TEST(RingQueueTest, AppendAndDequeueHandleWrapAround) {
    RingQueue<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to 2/3rds
    std::size_t oneThirdCapacity = capacity / 3;
    test.Write(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    // Remove the first 1/3rd, we end up with data in the middle of the ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Read(&retrieved[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity);

    // Now add another 2/3rds to the ring buffer. The write must wrap around.
    test.Write(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 3);

    // Finally, retrieve everything. The read must wrap around.
    test.Read(&retrieved[0], oneThirdCapacity * 3);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index + oneThirdCapacity]);
    }
    for(std::size_t index = 0; index < oneThirdCapacity * 2; ++index) {
      EXPECT_EQ(retrieved[index + oneThirdCapacity], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, WholeBufferCanBeFilledAndEmptied) {
    RingQueue<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to its current capacity
    test.Write(&items[0], capacity);
    EXPECT_EQ(test.Count(), capacity);

    // Remove the first 1/3rd, we end up with data in the middle ofthe ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Read(&retrieved[0], capacity);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < capacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, AppendCanHitBufferEnd) {
    RingQueue<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to 2/3rds
    std::size_t oneThirdCapacity = capacity / 3;
    test.Write(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    // Remove the first 1/3rd, we end up with data in the middle ofthe ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Read(&retrieved[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity);

    // Now add exactly the amount of items it takes to hit the end of the buffer
    std::size_t remainingItemCount = capacity - (oneThirdCapacity * 2);
    test.Write(&items[0], remainingItemCount);
    EXPECT_EQ(test.Count(), oneThirdCapacity + remainingItemCount);

    // If there's a karfluffle or off-by-one problem when hitting the end index,
    // this next call might blow up
    test.Write(&items[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), capacity);

    // Read all of the data from the ring buffer so we can check it
    test.Read(&retrieved[0], capacity);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index + oneThirdCapacity]);
    }
    for(std::size_t index = 0; index < (capacity - oneThirdCapacity * 2); ++index) {
      EXPECT_EQ(retrieved[index + oneThirdCapacity], items[index]);
    }
    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index + (capacity - oneThirdCapacity)], items[index]);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, DequeueCanHitBufferEnd) {
    RingQueue<std::uint8_t> test;

    std::size_t capacity = test.GetCapacity();

    std::vector<std::uint8_t> items(capacity);
    for(std::size_t index = 0; index < capacity; ++index) {
      items[index] = static_cast<std::uint8_t>(index);
    }

    // Fill the ring buffer to 2/3rds
    std::size_t oneThirdCapacity = capacity / 3;
    test.Write(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 2);

    // Remove the first 1/3rd, we end up with data in the middle of the ring
    std::vector<std::uint8_t> retrieved(capacity);
    test.Read(&retrieved[0], oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity);

    // Now add another 2/3rds to the ring buffer. The write must wrap around.
    test.Write(&items[0], oneThirdCapacity * 2);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 3);

    // Finally, retrieve just enough bytes to hit the end.
    test.Read(&retrieved[0], capacity - oneThirdCapacity);
    EXPECT_EQ(test.Count(), oneThirdCapacity * 3 - (capacity - oneThirdCapacity));

    for(std::size_t index = 0; index < oneThirdCapacity; ++index) {
      EXPECT_EQ(retrieved[index], items[index + oneThirdCapacity]);
    }
    for(std::size_t index = 0; index < capacity - oneThirdCapacity * 2; ++index) {
      EXPECT_EQ(retrieved[index + oneThirdCapacity], items[index]);
    }

    // If there's a karfluffle or off-by-one problem when moving the start index,
    // this next call might blow up
    std::size_t remainingByteCount = oneThirdCapacity * 3 - (capacity - oneThirdCapacity);
    test.Read(&retrieved[0], remainingByteCount);
    EXPECT_EQ(test.Count(), 0U);

    for(std::size_t index = 0; index < remainingByteCount; ++index) {
      EXPECT_EQ(
        retrieved[index],
        items[index + capacity - oneThirdCapacity * 2]
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, WritingInvokesCopyConstructor) {
    checkWritingInvokesCopyConstructor<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ShovingInvokesMoveConstructor) {
    //checkShovingInvokesMoveConstructor<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, MoveSemanticsAreUsedWhenCapacityChanges) {
    checkMoveSemanticsAreUsedWhenCapacityChanges<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ReadUsesMoveSemanticsAndCallsDestructor) {
    checkReadUsesMoveSemanticsAndCallsDestructor<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, BufferDestroysLeftOverItemsWhenDestroyed) {
    checkBufferDestroysLeftOverItemsWhenDestroyed<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ExceptionDuringCapacityChangeCausesNoLeaks) {
    checkExceptionDuringCapacityChangeCausesNoLeaks<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ExceptionDuringWriteCausesNoLeaks) {
    checkExceptionDuringWriteCausesNoLeaks<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ExceptionDuringShoveCausesNoLeaks) {
    //checkExceptionDuringShoveCausesNoLeaks<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(RingQueueTest, ExceptionDuringReadCausesNoLeaks) {
    checkExceptionDuringReadCausesNoLeaks<RingQueue>();
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
