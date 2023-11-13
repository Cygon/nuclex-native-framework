#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#include "Nuclex/Support/Threading/Thread.h"

#include <gtest/gtest.h>

#include <thread>
#include <atomic>

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WANT_USELESS_THREAD_ID_QUERY)
  TEST(ThreadTest, CanGetCurrentThreadId) {
    std::uintptr_t threadId = Thread::GetCurrentThreadId();
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, ThreadsCanSleepAccurately) {
    Thread::Sleep(std::chrono::microseconds(25000));
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)
  TEST(ThreadTest, ThreadHasNativeIdentifier) {
    const std::uintptr_t nullUintPtr = reinterpret_cast<std::uintptr_t>(nullptr);

    std::uintptr_t threadId = Thread::GetCurrentThreadId();
    EXPECT_NE(threadId, std::uintptr_t(nullUintPtr));
  }
#endif // defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)
  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, IdentifierOfOtherThreadCanBeQueried) {
    const std::uintptr_t nullUintPtr = reinterpret_cast<std::uintptr_t>(nullptr);

    std::uintptr_t firstThreadId, secondThreadId;
    {
      std::atomic<bool> firstSpinRelease = false;
      std::atomic<bool> secondSpinRelease = false;

      std::thread firstThread(
        [&] { while(firstSpinRelease.load(std::memory_order_consume) == false) {} }
      );
      std::thread secondThread(
        [&] { while(secondSpinRelease.load(std::memory_order_consume) == false) {} }
      );

      firstThreadId = Thread::GetStdThreadId(firstThread);
      secondThreadId = Thread::GetStdThreadId(secondThread);

      firstSpinRelease.store(true, std::memory_order_release);
      secondSpinRelease.store(true, std::memory_order_release);
      firstThread.join();
      secondThread.join();
    }

    EXPECT_NE(firstThreadId, nullUintPtr);
    EXPECT_NE(secondThreadId, nullUintPtr);
    EXPECT_NE(firstThreadId, secondThreadId);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, OwnAffinityCanBeChecked) {

    // Build a mask where the affinity bits for all CPUs are set.
    std::uint64_t allCpusAffinity = 0;
    {
      std::size_t cpuCount = std::thread::hardware_concurrency();
      for(std::size_t index = 0; index < cpuCount; ++index) {
        allCpusAffinity |= (std::uint64_t(1) << index);
      }
    }

    // Query the affinity flags set for the calling thread
    std::uint64_t ownAffinity;
    {
      std::thread otherThread(
        [&] { ownAffinity = Thread::GetCpuAffinityMask(); }
      );
      otherThread.join();
    }

    // If you run this on a system with more than 64 CPUs, the test will fail.
    // Either the exact flags for the present CPU cores or a (-1) for all cores is okay.
    if(ownAffinity == std::uint64_t(-1)) {
      EXPECT_EQ(ownAffinity, std::uint64_t(-1));
    } else {
      EXPECT_EQ(ownAffinity, allCpusAffinity);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, OtherThreadsAffinityCanBeChecked) {

    // Build a mask where the affinity bits for all CPUs are set.
    std::uint64_t allCpusAffinity = 0;
    {
      std::size_t cpuCount = std::thread::hardware_concurrency();
      for(std::size_t index = 0; index < cpuCount; ++index) {
        allCpusAffinity |= (std::uint64_t(1) << index);
      }
    }

    // Query the affinity flags set for a new thread
    std::uint64_t newThreadAffinity = 0;
    {
      std::atomic<bool> spinRelease = false;
      std::thread otherThread(
        [&] { while(spinRelease.load(std::memory_order_consume) == false) {} }
      );

      newThreadAffinity = Thread::GetCpuAffinityMask(Thread::GetStdThreadId(otherThread));

      spinRelease.store(true, std::memory_order_release);
      otherThread.join();
    }

    // If you run this on a system with more than 64 CPUs, the test will fail.
    // Either the exact flags for the present CPU cores or a (-1) for all cores is okay.
    if(newThreadAffinity == std::uint64_t(-1)) {
      EXPECT_EQ(newThreadAffinity, std::uint64_t(-1));
    } else {
      EXPECT_EQ(newThreadAffinity, allCpusAffinity);
    }

  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, OwnAffinityCanBeChanged) {
    std::uint64_t unchangedAffinity, changedAffinity;

    // Use CPU cores 3 and 4
    std::uint64_t testedAffinity = (3 << 2);

    {
      std::thread otherThread(
        [&] {
          unchangedAffinity = Thread::GetCpuAffinityMask();
          Thread::SetCpuAffinityMask(testedAffinity);
          changedAffinity = Thread::GetCpuAffinityMask();
        }
      );
      otherThread.join();
    }

    EXPECT_NE(unchangedAffinity, testedAffinity);
    EXPECT_NE(unchangedAffinity, changedAffinity);
    EXPECT_EQ(changedAffinity, testedAffinity);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadTest, OtherThreadsAffinityCanBeChanged) {
    std::uint64_t unchangedAffinity, changedAffinity;

    // Use CPU cores 3 and 4
    std::uint64_t testedAffinity = (3 << 2);

    {
      std::atomic<bool> spinRelease = false;
      std::thread otherThread(
        [&] { while(spinRelease.load(std::memory_order_consume) == false) {} }
      );

      std::uintptr_t otherThreadId = Thread::GetStdThreadId(otherThread);
      unchangedAffinity = Thread::GetCpuAffinityMask(otherThreadId);
      Thread::SetCpuAffinityMask(otherThreadId, testedAffinity);
      changedAffinity = Thread::GetCpuAffinityMask(otherThreadId);

      spinRelease.store(true, std::memory_order_release);
      otherThread.join();
    }

    EXPECT_NE(unchangedAffinity, testedAffinity);
    EXPECT_NE(unchangedAffinity, changedAffinity);
    EXPECT_EQ(changedAffinity, testedAffinity);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
