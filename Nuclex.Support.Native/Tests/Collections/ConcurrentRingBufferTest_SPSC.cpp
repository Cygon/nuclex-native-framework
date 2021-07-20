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
#include <chrono>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A concurrent single producer, single consumer ring buffer</summary>
  template<typename TItem>
  using TestedBuffer = Nuclex::Support::Collections::ConcurrentRingBuffer<
    TItem,
    Nuclex::Support::Collections::ConcurrentAccessBehavior::SingleProducerSingleConsumer
  >;

  // ------------------------------------------------------------------------------------------- //
#if 0
  /// <summary>Generates the first pseudo-random number following a fixed seed</summary>
  /// <param name="seed">Seed value, same seeds produce same pseudo-random numbers</param>
  /// <returns>The first random number that followed the specified seed</returns>
  /// <remarks>
  ///   In some implementations of the C++ standard library (*cough* MSVC *cough*),
  ///   std::default_random_engine has a substantial setup and/or processing time,
  ///   taking 30+ seconds on a modern CPU to generate 128 KiB of data. Since quality
  ///   of random numbers is not important here, we use this fast "Xor-Shift" generator.
  /// </remarks>
  std::size_t fastRandomNumber(std::size_t seed) {
    seed ^= (seed << 21);
    seed ^= (seed >> 35);
    seed ^= (seed << 4);
    return seed;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      TestedBuffer<int> test(10);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, CanReportCapacity) {
    TestedBuffer<int> test(124);
    EXPECT_EQ(test.GetCapacity(), 124U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, SingleItemsCanBeAppended) {
    TestedBuffer<int> test(10);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, SingleAppendFailsIfBufferFull) {
    TestedBuffer<int> test(3);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
    EXPECT_FALSE(test.TryAppend(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, ItemsCanBeCounted) {
    TestedBuffer<int> test(3);
    EXPECT_EQ(test.Count(), 0U);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_EQ(test.Count(), 1U);
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_EQ(test.Count(), 2U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, ItemsCanBeCountedWhenFragmented) {
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

  TEST(ConcurrentRingBufferTest_SPSC, ItemsStayOrderedWhenFragmented) {
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

  TEST(ConcurrentRingBufferTest_SPSC, BufferCanBeEmpty) {
    TestedBuffer<int> test(5);

    int value;
    EXPECT_FALSE(test.TryTake(value)); // Starts out empty
    EXPECT_TRUE(test.TryAppend(100));
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_FALSE(test.TryTake(value)); // Was emptied again with call above
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_SPSC, SingleItemsCanBeRead) {
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
  TEST(ConcurrentRingBufferTest_SPSC, BenchmarkAddingItems) {
    const std::size_t MaximumThreadCount = 1;
    benchmarkSingleItemAppends<TestedBuffer>(MaximumThreadCount);
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_SPSC, BenchmarkTakingItems) {
    const std::size_t MaximumThreadCount = 1;
    benchmarkSingleItemTakes<TestedBuffer>(MaximumThreadCount);
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_SPSC, BenchmarkMixedItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    const std::size_t MaximumThreadCount = 2; // one producer, one consumer
    benchmarkSingleItemMixed<TestedBuffer>(MaximumThreadCount);
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#ifdef NUCLEX_SUPPORT_COLLECTIONS_UNTESTED_BATCH_OPERATIONS
  TEST(ConcurrentRingBufferTest_SPSC, ItemsCanBeBatchAppended) {
    TestedBuffer<int> test(10);
    int items[] = { 1, 2, 3, 4, 5, 6, 7 };

    EXPECT_EQ(test.TryAppend(items, 7), 7U);
    EXPECT_EQ(test.Count(), 7U); // Ensure consistency, not just correct return
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#ifdef NUCLEX_SUPPORT_COLLECTIONS_UNTESTED_BATCH_OPERATIONS
  TEST(ConcurrentRingBufferTest_SPSC, BatchAppendCanFragmentItems) {
    TestedBuffer<int> test(10);
    int items[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    EXPECT_EQ(test.TryAppend(items, 6), 6U);
    // Expected buffer state: ######----
    EXPECT_EQ(test.Count(), 6U);

    for(std::size_t index = 0; index < 4; ++index) {
      int dummy;
      EXPECT_TRUE(test.TryTake(dummy));
    }
    // Expected buffer state: ----##----
    EXPECT_EQ(test.Count(), 2U);

    EXPECT_EQ(test.TryAppend(items, 6), 6U);
    // Expected buffer state: ##--######
    EXPECT_EQ(test.Count(), 8U);

    for(std::size_t index = 0; index < 4; ++index) {
      int dummy;
      EXPECT_TRUE(test.TryTake(dummy));
    }
    // Expected buffer state: ##------##
    EXPECT_EQ(test.Count(), 4U);

    EXPECT_EQ(test.TryAppend(items, 6), 6U);
    // Expected buffer state: ##########
    EXPECT_EQ(test.Count(), 10U);
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
