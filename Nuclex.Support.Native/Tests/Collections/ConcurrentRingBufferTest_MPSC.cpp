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

#include "Nuclex/Support/Collections/ConcurrentRingBuffer.h"
#include "BufferTest.h"
#include "ConcurrentBufferTest.h"
#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A concurrent multi producer, single consumer ring buffer</summary>
  template<typename TItem>
  using TestedBuffer = Nuclex::Support::Collections::ConcurrentRingBuffer<
    TItem,
    Nuclex::Support::Collections::ConcurrentAccessBehavior::MultipleProducersSingleConsumer
  >;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      TestedBuffer<int> test(10);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, CanReportCapacity) {
    TestedBuffer<int> test(124);
    EXPECT_EQ(test.GetCapacity(), 124U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, SingleItemsCanBeAppended) {
    TestedBuffer<int> test(10);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, SingleAppendFailsIfBufferFull) {
    TestedBuffer<int> test(3);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
    EXPECT_FALSE(test.TryAppend(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, ItemsCanBeCounted) {
    TestedBuffer<int> test(3);
    EXPECT_EQ(test.Count(), 0U);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_EQ(test.Count(), 1U);
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_EQ(test.Count(), 2U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, ItemsCanBeCountedWhenFragmented) {
    TestedBuffer<int> test(10);

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

  TEST(ConcurrentRingBufferTest_MPSC, ItemsStayOrderedWhenFragmented) {
    TestedBuffer<int> test(10);

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

  TEST(ConcurrentRingBufferTest_MPSC, BufferCanBeEmpty) {
    TestedBuffer<int> test(5);

    int value;
    EXPECT_FALSE(test.TryTake(value)); // Starts out empty
    EXPECT_TRUE(test.TryAppend(100));
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_FALSE(test.TryTake(value)); // Was emptied again with call above
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPSC, SingleItemsCanBeRead) {
    TestedBuffer<int> test(5);
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
  TEST(ConcurrentRingBufferTest_MPSC, BenchmarkAddingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemAppends<TestedBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_MPSC, BenchmarkTakingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    const std::size_t MaximumThreadCount = 1;
    benchmarkSingleItemTakes<TestedBuffer>(MaximumThreadCount);
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_MPSC, BenchmarkMixedItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    const std::size_t MaximumThreadCount = 3; // two producers, one consumer
    benchmarkSingleItemMixed<TestedBuffer>(MaximumThreadCount);
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
