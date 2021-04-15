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

#include "Nuclex/Support/Threading/Gate.h"
#include "Nuclex/Support/Threading/Thread.h"

#include <gtest/gtest.h>

#include <atomic> // for std::atomic
#include <thread> // for std::thread
#include <stdexcept> // for std::system_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages a thread to test the behavior of the gate</summary>
  class TestThread {

    /// <summary>Initializes a new test thread checking the specified gate</summary>
    /// <param name="gate">Gate that the thread will be checking</param>
    public: TestThread(Nuclex::Support::Threading::Gate &gate) :
      gate(gate),
      thread(),
      gatePassed(false) {}

    /// <summary>Waits for the thread to end an destroys it</summary>
    public: ~TestThread() {
      this->gate.Open();
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

    /// <summary>Checks whether the test thread has passed through the gate</summary>
    public: bool HasPassed() const {
      return this->gatePassed.load(std::memory_order::memory_order_acquire);
    }

    /// <summary>Method that runs in a thread to check the gate function</summary>
    private: void threadMethod() {
      this->gate.Wait();
      this->gatePassed.store(true, std::memory_order::memory_order_release);
    }

    /// <summary>Gate that the test thread will attempt to pass</summary>
    private: Nuclex::Support::Threading::Gate &gate;
    /// <summary>Thread that will attempt to pass the gate</summary>
    private: std::thread thread;
    /// <summary>Set to true as soon as the thread has passed the gate</summary>
    private: std::atomic<bool> gatePassed;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, InstancesCanBeCreated) {
    EXPECT_NO_THROW(
      Gate gate;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, CanBeOpened) {
    Gate gate;
    gate.Open();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, CanBeClosed) {
    Gate gate;
    gate.Open();
    gate.Close();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, DoubleOpenIsOkay) {
    Gate gate;
    gate.Open();
    gate.Open();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, DoubleCloseIsOkay) {
    Gate gate;
    gate.Close(); // technically, this is the double close since it starts closed :)
    gate.Close();
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, StateCanBeSetViaBoolean) {
    Gate gate;
    gate.Set(true);
    gate.Set(false);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, ThreadCanPassOpenGate) {
    Gate gate;
    gate.Open();

    TestThread test(gate);
    test.LaunchThread();
    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, ThreadWaitsBeforeClosedGate) {
    Gate gate;

    TestThread test(gate);
    test.LaunchThread();

    // Give the thread some time to pass. We can't wait for the thread to
    // reach the gate without building a race condition of our own, so we'll just
    // give it ample time to run against the gate.
    Thread::Sleep(std::chrono::microseconds(25000)); // 25 ms

    // Thread should still be waiting in front of the gate
    EXPECT_FALSE(test.HasPassed());

    gate.Open();

    test.JoinThread();
    EXPECT_TRUE(test.HasPassed());
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(GateTest, WaitCanTimeOut) {
    Gate gate;

    bool hasPassed = gate.WaitFor(std::chrono::microseconds(1000));
    EXPECT_FALSE(hasPassed);

    gate.Open();

    hasPassed = gate.WaitFor(std::chrono::microseconds(1000));
    EXPECT_TRUE(hasPassed);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
