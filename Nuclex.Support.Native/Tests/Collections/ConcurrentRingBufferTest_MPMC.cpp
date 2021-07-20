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

#if defined(_MSC_VER) && (_MSC_VER >= 1920)
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message ( \
  __FILE__ "(" STRING(__LINE__) "): " \
  "warning: disabled Nuclex MPMC Queue unit tests on VS2019 due to a compiler bug" \
)
#define NUCLEX_SUPPORT_BROKEN_COMPILER_USED 1
#endif

#if !defined(NUCLEX_SUPPORT_BROKEN_COMPILER_USED)

#include "Nuclex/Support/Collections/ConcurrentRingBuffer.h"
#include "BufferTest.h"
#include "ConcurrentBufferTest.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A concurrent multi producer, multi consumer ring buffer</summary>
  template<typename TItem>
  using TestedBuffer = Nuclex::Support::Collections::ConcurrentRingBuffer<
    TItem,
    Nuclex::Support::Collections::ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
  >;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      TestedBuffer<int> test(10);
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, CanReportCapacity) {
    TestedBuffer<int> test(124);
    EXPECT_EQ(test.GetCapacity(), 124U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, SingleItemsCanBeAppended) {
    TestedBuffer<int> test(10);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, SingleAppendFailsIfBufferFull) {
    TestedBuffer<int> test(3);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_TRUE(test.TryAppend(789));
    EXPECT_FALSE(test.TryAppend(0));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, ItemsCanBeCounted) {
    TestedBuffer<int> test(3);
    EXPECT_EQ(test.Count(), 0U);
    EXPECT_TRUE(test.TryAppend(123));
    EXPECT_EQ(test.Count(), 1U);
    EXPECT_TRUE(test.TryAppend(456));
    EXPECT_EQ(test.Count(), 2U);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, ItemsCanBeCountedWhenFragmented) {
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

  TEST(ConcurrentRingBufferTest_MPMC, ItemsStayOrderedWhenFragmented) {
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

  TEST(ConcurrentRingBufferTest_MPMC, BufferCanBeEmpty) {
    TestedBuffer<int> test(5);

    int value;
    EXPECT_FALSE(test.TryTake(value)); // Starts out empty
    EXPECT_TRUE(test.TryAppend(100));
    EXPECT_TRUE(test.TryTake(value));
    EXPECT_FALSE(test.TryTake(value)); // Was emptied again with call above
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentRingBufferTest_MPMC, SingleItemsCanBeRead) {
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
  TEST(ConcurrentRingBufferTest_MPMC, BenchmarkAddingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemAppends<TestedBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_MPMC, BenchmarkTakingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemTakes<TestedBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentRingBufferTest_MPMC, BenchmarkMixedItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemMixed<TestedBuffer>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections

#endif // !defined(NUCLEX_SUPPORT_BROKEN_COMPILER_USED)
