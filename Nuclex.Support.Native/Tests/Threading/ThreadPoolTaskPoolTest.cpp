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

#include "../Source/Threading/ThreadPoolTaskPool.h"

#include <memory> // for std::unique_ptr
#include <mutex> // for std::mutex

#include <gtest/gtest.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Mock task used to test the task pool</summary>
  struct TestTask {

    /// <summary>Number of times a task constructor has been called</summary>
    public: static std::size_t ConstructorCallCount;
    /// <summary>Number of times a task destructor has been called</summary>
    public: static std::size_t DestructorCallCount;

    /// <summary>Initializes a new test task</summary>
    public: TestTask() {
      ++ConstructorCallCount;
    }

    /// <summary>Destroys a test task</summary>
    public: ~TestTask() {
      ++DestructorCallCount;
    }

    /// <summary>Size of the payload carried by the task</summary>
    public: std::size_t PayloadSize;
    /// <summary>Example content, never used, never accessed</summary>
    public: float Unused;
    /// <summary>Placeholder for the variable payload appended to the task</summary>
    public: std::uint8_t Payload[sizeof(std::uintptr_t)];

  };

  // ------------------------------------------------------------------------------------------- //

  std::size_t TestTask::ConstructorCallCount = 0;

  // ------------------------------------------------------------------------------------------- //

  std::size_t TestTask::DestructorCallCount = 0;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Used to avoid unit tests from interfering with each other in case they're run in parallel
  /// </summary>
  std::mutex CallCountMutex;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A pool of mock tasks</summary>
  typedef Nuclex::Support::Threading::ThreadPoolTaskPool<
    TestTask, offsetof(TestTask, Payload)
  > TestTaskPool;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, HasDefaultConstructor) {
    EXPECT_NO_THROW(
      TestTaskPool taskPool;
    );
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, TaskConstructorAndDestructorAreCalled) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *myTask = taskPool.GetNewTask(32);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.DeleteTask(myTask);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, TasksCanBeRecycled) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *originalTask = taskPool.GetNewTask(32);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(originalTask);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      TestTask *anotherTask = taskPool.GetNewTask(16);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      EXPECT_EQ(anotherTask, originalTask);

      taskPool.DeleteTask(anotherTask);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, RecycledTaskIsOnlyHandedOutWhenLargeEnough) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *originalTask = taskPool.GetNewTask(16);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(originalTask);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      TestTask *anotherTask = taskPool.GetNewTask(32);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 2);
      //EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      // CHECK: This test failed spuriously once
      // There isn't even any threading or tricky stuff involved here,
      // except that the dequeue is done from the highly complex moodycamel queue...
      EXPECT_NE(anotherTask, originalTask);

      taskPool.DeleteTask(anotherTask);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, PoolDestructionKillsRecycledTasks) {
    std::lock_guard callCountScope(CallCountMutex);

    std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
    std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

    {
      TestTaskPool taskPool;

      TestTask *myTask = taskPool.GetNewTask(32);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(myTask);
      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);
    }

    EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
    EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ThreadPoolTaskPoolTest, HugeTasksAreNotRecycled) {
    TestTaskPool taskPool;

    {
      std::lock_guard callCountScope(CallCountMutex);

      std::size_t previousConstructorCallCount = TestTask::ConstructorCallCount;
      std::size_t previousDestructorCallCount = TestTask::DestructorCallCount;

      TestTask *originalTask = taskPool.GetNewTask(1024);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount);

      taskPool.ReturnTask(originalTask);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 1);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);

      TestTask *anotherTask = taskPool.GetNewTask(16);

      EXPECT_EQ(TestTask::ConstructorCallCount, previousConstructorCallCount + 2);
      EXPECT_EQ(TestTask::DestructorCallCount, previousDestructorCallCount + 1);

      EXPECT_NE(anotherTask, originalTask);

      taskPool.DeleteTask(anotherTask);
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
