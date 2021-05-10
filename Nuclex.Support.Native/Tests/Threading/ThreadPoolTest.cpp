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

#include "Nuclex/Support/Threading/ThreadPool.h" // for ThreadPool
#include "Nuclex/Support/Threading/Thread.h" // for Thread
#include "Nuclex/Support/Threading/Gate.h" // for Gate

#include <memory> // for std::unique_ptr

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Performs a simple calculation, used to test the thread pool</summary>
  /// <param name="a">First input for the useless formula</param>
  /// <param name="b">Second input for the useless formula</param>
  /// <returns>The result of the formula ab - a - b</returns>
  int testMethod(int a, int b) {
    return a * b - (a + b);
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Method that is simply slow to execute</summary>
  void slowMethod() {
    Nuclex::Support::Threading::Thread::Sleep(std::chrono::milliseconds(100));
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Method that fails with an exception</summary>
  int failingMethod() {
    throw std::underflow_error(u8"Hur dur, I'm an underflow error");
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      ThreadPool testPool;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTest, CanScheduleTasks) {
    ThreadPool testPool;

    // Schedule a task to run on a thread pool thread
    std::future<int> future = testPool.Schedule(&testMethod, 12, 34);

    // The future should immediately be valid and usable to chain calls and wait upon
    EXPECT_TRUE(future.valid());

    // Wait for the task to execute on the thread pool, filling the future
    int result = future.get();
    EXPECT_EQ(result, 362);

    // The thread pool is cleanly shut down as it goes out of scope
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTest, ThreadPoolShutdownCancelsTasks) {
    std::unique_ptr<ThreadPool> testPool = std::make_unique<ThreadPool>(1, 1);

    // Add a slow task and our detector task. This thread pool only has
    // one thread, so the slow task will block the worker thread for 100 ms.
    testPool->Schedule(&slowMethod);
    std::future<int> canceledFuture = testPool->Schedule(&testMethod, 12, 34);

    EXPECT_TRUE(canceledFuture.valid());

    // Now we destroy the thread pool. All outstanding tasks will be destroyed,
    // canceling their returned std::futures without proving a result.
    testPool.reset();

    // An attempt to obtain the result from the canceled future should now
    // result in an std::future_error with the broken_promise error code.
    EXPECT_THROW(
      {
        int result = canceledFuture.get();
        (void)result;
      },
      std::future_error
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTest, ExceptionInCallbackPropagatesToStdFuture) {
    ThreadPool testPool;

    // Schedule a task to run on a thread pool thread
    std::future<int> failedFuture = testPool.Schedule(&failingMethod);

    EXPECT_THROW(
      {
        int result = failedFuture.get();
        (void)result;
      },
      std::underflow_error
    );

    // The thread pool is cleanly shut down as it goes out of scope
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTest, StressTestCompletes) {
    for(std::size_t repetition = 0; repetition < 10; ++repetition) {
      std::unique_ptr<ThreadPool> testPool = std::make_unique<ThreadPool>(
        std::thread::hardware_concurrency() / 2, std::thread::hardware_concurrency() / 2
      );

      // Schedule 1000 tasks in two batches with a small break inbetween.
      // This will let some of the (or all of them) complete,
      // letting the thread pool recycle finished tasks for re-use.
      {
        for(std::size_t task = 0; task < 500; ++task) {
          testPool->Schedule(&testMethod, 12, 34);
        }
        Nuclex::Support::Threading::Thread::Sleep(std::chrono::milliseconds(1));
        for(std::size_t task = 0; task < 500; ++task) {
          testPool->Schedule(&testMethod, 34, 12);
        }
      }

      // Schedule one final task, then let the thread pool execute for a bit
      std::future<int> finalTaskFuture = testPool->Schedule(&testMethod, 10, 10);
      std::future_status status = finalTaskFuture.wait_for(std::chrono::milliseconds(1));
      (void)status; // This is up to the core count + performance, we don't check it

      // Destroy the thread pool while it is still working. This will cancel
      // all still ongoing tasks (the returned futures will throw std::future_error)
      testPool.reset();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTest, HelperCanIdentifyThreadPoolThreads) {
    bool threadPoolThreadAnswer;

    // Check whether a thread pool thread gets the right answer
    {
      Gate finishedGate;

      ThreadPool testPool(1, 1);
      testPool.Schedule(
        [&threadPoolThreadAnswer, &finishedGate] {
          threadPoolThreadAnswer = Thread::BelongsToThreadPool();
          finishedGate.Open();
        }
      );

      finishedGate.Wait();
    }

    // Check whether an explicit thread gets the right answer
    bool explicitThreadAnswer;
    {
      std::thread explicitThread(
        [&explicitThreadAnswer] {
          explicitThreadAnswer = Thread::BelongsToThreadPool();
        }
      );
      explicitThread.join();
    }

    EXPECT_TRUE(threadPoolThreadAnswer);
    EXPECT_FALSE(explicitThreadAnswer);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
