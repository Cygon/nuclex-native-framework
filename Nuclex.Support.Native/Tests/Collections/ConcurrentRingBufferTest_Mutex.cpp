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

#include <mutex>
#include <vector>

#include "ConcurrentBufferTest.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Shared ring buffer that uses a mutex for synchronization</summary>
  /// <remarks>
  ///   This <em>should</em> be slower than our atomics-using concurrent ring buffer,
  ///   but we want to at least have for performance comparison.
  /// </remarks>
  template<typename TItem>
  class MutexRingBuffer {

    /// <summary>Initializes a new mutex-based shared ring buffer</summary>
    /// <param name="capacity">Number of items the ring buffer can hold</param>
    public: MutexRingBuffer(std::size_t capacity) :
      items(capacity),
      firstItemIndex(0),
      itemCount(0),
      bufferMutex() {}

    /// <summary>Tries to append the specified element to the ring buffer</summary>
    /// <param name="element">Element that will be appended to the ring buffer</param>
    /// <returns>True if the element was appended, false if there was no space left</returns>
    public: bool TryAppend(const TItem &element) {
      std::scoped_lock lockScope(this->bufferMutex);
      if(this->itemCount < this->items.size()) {
        std::size_t index = (this->firstItemIndex + this->itemCount) % this->items.size();
        this->items[index] = element;
        ++this->itemCount;
        return true;
      } else {
        return false;
      }
    }

    /// <summary>Tries to remove an element from the queue</summary>
    /// <param name="element">Element into which the queue's element will be placed</param>
    /// <returns>True if an item was available and return, false otherwise</returns>
    public: bool TryTake(TItem &element) {
      std::scoped_lock lockScope(this->bufferMutex);
      if(this->itemCount >= 1) {
        element = this->items[this->firstItemIndex];
        this->firstItemIndex = (this->firstItemIndex + 1) % this->items.size();
        --this->itemCount;
        return true;
      } else {
        return false;
      }
    }

    /// <summary>Returns the number of items currently stored in the buffer</summary>
    /// <returns>Number of items in the buffer</returns>
    public: std::size_t Count() const {
      std::scoped_lock lockScope(this->bufferMutex);
      return this->itemCount;
    }

    /// <summary>Returns the total number of items that the buffer can hold</summary>
    /// <returns>The capacity of the buffer</returns>
    public: std::size_t GetCapacity() const { return this->items.size(); }

    /// <summary>Vector used to hold the items of the ring buffer</summary>
    private: std::vector<TItem> items;
    /// <summary>Index of the first item in the ring buffer</summary>
    private: std::size_t firstItemIndex;
    /// <summary>Number of items currently stored in the ring buffer</summary>
    private: std::size_t itemCount;
    /// <summary>Mutex used to synchronize threads accessing the ring buffer</summary>
    private: mutable std::mutex bufferMutex;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      MutexRingBuffer<int> test(10);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, CanReportCapacity) {
    MutexRingBuffer<int> test(124);
    EXPECT_EQ(test.GetCapacity(), 124U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, SingleItemsCanBeAppended) {
    MutexRingBuffer<int> test(10);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, SingleAppendFailsIfBufferFull) {
    MutexRingBuffer<int> test(3);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
    EXPECT_FALSE(test.TryAppend(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, ItemsCanBeCounted) {
    MutexRingBuffer<int> test(3);
    EXPECT_EQ(test.Count(), 0U);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_EQ(test.Count(), 1U);
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_EQ(test.Count(), 2U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, ItemsCanBeCountedWhenFragmented) {
    MutexRingBuffer<int> test(10);

    for(std::size_t index = 0; index < 8; ++index) {
      EXPECT_TRUE(test.TryAppend(12345));
    }
    // Expected buffer state: ########--
    EXPECT_EQ(test.Count(), 8U);

    for(std::size_t index = 0; index < 6; ++index) {
      int dummy;
      EXPECT_TRUE(test.TryTake(dummy));
    }
    // Expected buffer state: ------##--
    EXPECT_EQ(test.Count(), 2U);

    for(std::size_t index = 0; index < 4; ++index) {
      EXPECT_TRUE(test.TryAppend(12345));
    }
    // Expected buffer state: ##----####
    EXPECT_EQ(test.Count(), 6U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, ItemsStayOrderedWhenFragmented) {
    MutexRingBuffer<int> test(10);

    for(std::size_t index = 0; index < 8; ++index) {
      EXPECT_TRUE(test.TryAppend(static_cast<int>(index)));
    }
    // Expected buffer state: ########--

    for(std::size_t index = 0; index < 6; ++index) {
      int value;
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, static_cast<int>(index));
    }
    // Expected buffer state: ------##--

    for(std::size_t index = 0; index < 4; ++index) {
      EXPECT_TRUE(test.TryAppend(static_cast<int>(index + 10)));
    }
    // Expected buffer state: ##----####

    {
      int value;
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, 6);
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, 7);
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, 10);
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, 11);
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, 12);
      EXPECT_TRUE(test.TryTake(value));
      EXPECT_EQ(value, 13);
      EXPECT_FALSE(test.TryTake(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, BufferCanBeEmpty) {
    MutexRingBuffer<int> test(5);

    int value;
    EXPECT_FALSE(test.TryTake(value)); // Starts out empty
    EXPECT_TRUE(test.TryAppend(100));
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_FALSE(test.TryTake(value)); // Was emptied again with call above
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_Mutex, SingleItemsCanBeRead) {
    MutexRingBuffer<int> test(5);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));

    int value;
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_EQ(value, 123);
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_EQ(value, 456);
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_EQ(value, 789);
    EXPECT_FALSE(test.TryTake(value));
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_Mutex, BenchmarkAddingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemAppends<MutexRingBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_Mutex, BenchmarkTakingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemTakes<MutexRingBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_Mutex, BenchmarkMixedItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemMixed<MutexRingBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
