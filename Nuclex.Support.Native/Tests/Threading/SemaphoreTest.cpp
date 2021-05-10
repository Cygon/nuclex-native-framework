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

#include "Nuclex/Support/Threading/Semaphore.h"
#include "Nuclex/Support/Threading/Thread.h"

#include <gtest/gtest.h>

#include <atomic> // for std::atomic
#include <thread> // for std::thread
#include <stdexcept> // for std::system_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a thread to test the behavior of the semaphore</summary>
  class TestThread {

    /// <summary>Initializes a new test thread checking the specified semaphore</summary>
    /// <param name="semaphore">Semaphore that the thread will be checking</param>
    public: TestThread(Nuclex::Support::Threading::Semaphore &semaphore) :
      semaphore(semaphore),
      thread(),
      semaphorePassed(false) {}

    /// <summary>Waits for the thread to end and destroys it</summary>
    public: ~TestThread() {
      this->semaphore.Post(64);
      if(this->thread.joinable()) {
        this->thread.join();
      }
    }

    /// <summary>Launches the test thread</summary>
    public: void LaunchThread() {
      if(this->thread.joinable()) {
        this->thread.join();
      }

      std::thread newThread(&TestThread::threadMethod, this);
      this->thread.swap(newThread);
    }

    /// <summary>Waits for the test thread to terminate</summary>
    public: void JoinThread() {
      this->thread.join();
    }

    /// <summary>Checks whether the test thread has passed through the semaphore</summary>
    public: bool HasPassed() const {
      return this->semaphorePassed.load(std::memory_order_acquire);
    }

    /// <summary>Method that runs in a thread to check the semaphore function</summary>
    private: void threadMethod() {
      this->semaphore.WaitThenDecrement();
      this->semaphorePassed.store(true, std::memory_order_release);
    }

    /// <summary>Semaphore that the test thread will attempt to pass</summary>
    private: Nuclex::Support::Threading::Semaphore &semaphore;
    /// <summary>Thread that will attempt to pass the gate</summary>
    private: std::thread thread;
    /// <summary>Set to true as soon as the thread has passed the semaphore</summary>
    private: std::atomic<bool> semaphorePassed;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Semaphore semaphore;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, CanBeIncremented) {
    Semaphore semaphore;
    semaphore.Post();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, ThreadCanPassIncrementedSemaphore) {
    Semaphore semaphore;
    semaphore.Post();

    TestThread test(semaphore);
    test.LaunchThread();
    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, ThreadWaitsBeforeZeroedSemaphore) {
    Semaphore semaphore;

    TestThread test(semaphore);
    test.LaunchThread();

    // Give the thread some time to pass. We can't wait for the thread to
    // reach the semaphore without building a race condition of our own,
    // so we'll just give it ample time to hit the semaphore.
    Thread::Sleep(std::chrono::microseconds(25000)); // 25 ms

    // Thread should still be waiting in front of the semaphore
    EXPECT_FALSE(test.HasPassed());

    semaphore.Post();

    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(SemaphoreTest, WaitCanTimeOut) {
    Semaphore semaphore;

    bool hasPassed = semaphore.WaitForThenDecrement(
      std::chrono::microseconds(1000)
    );
    EXPECT_FALSE(hasPassed);

    semaphore.Post();

    hasPassed = semaphore.WaitForThenDecrement(
      std::chrono::microseconds(1000)
    );
    EXPECT_TRUE(hasPassed);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
