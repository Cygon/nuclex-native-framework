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

#include "Nuclex/Support/Collections/ConcurrentQueue.h"
#include "BufferTest.h"
#include "ConcurrentBufferTest.h"

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A concurrent multi producer, multi consumer ring buffer</summary>
  template<typename TItem>
  using TestedQueue  = Nuclex::Support::Collections::ConcurrentQueue<
    TItem,
    Nuclex::Support::Collections::ConcurrentAccessBehavior::MultipleProducersMultipleConsumers
  >;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Collections {

  // ------------------------------------------------------------------------------------------- //

  TEST(ConcurrentQueueTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      ConcurrentQueue<int> test;
    );
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentQueueTest, BenchmarkAddingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemAppends<TestedQueue>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentQueueTest, BenchmarkTakingItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemTakes<TestedQueue>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  TEST(ConcurrentQueueTest, BenchmarkMixedItems) {
  //TEST(ConcurrentRingBufferTest_SPSC, DISABLED_Benchmark) {
    benchmarkSingleItemMixed<TestedQueue>();
  }
#endif // defined(NUCLEX_SUPPORT_ENABLE_BENCHMARKS)
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Collections
