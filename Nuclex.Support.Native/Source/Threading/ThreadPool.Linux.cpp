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

#include "Nuclex/Support/Threading/ThreadPool.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include "Nuclex/Support/ScopeGuard.h" // for ScopeGuard
#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"

#include "ThreadPoolTaskPool.h" // thread pool settings + task pool
#include "Posix/PosixTimeApi.h" // error handling helpers, time helpers

#include <cassert> // for assert()
#include <atomic> // for std::atomic
#include <thread> // for std::thread

#include <sys/sysinfo.h> // for ::get_nprocs()
#include <semaphore.h> // for ::sem_init(), ::sem_wait(), ::sem_post(), ::sem_destroy()

// There is no OS-provided thread pool on Linux systems
//
// Thus, an entire stand-alone thread pool is implemented as a private implementation
// invisible to the header. Which makes this file quite a bit larger than the Windows
// counterpart which relies on an already existing implementation shipped with the OS.
//

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementation {

    #pragma region struct SubmittedTask

    /// <summary>Wraps a callback that can be schuled on a worker thread</summary>
    public: struct SubmittedTask {

      /// <summary>Size of the payload allocated for this task instance</summary>
      public: std::size_t PayloadSize;
      /// <summary>The task instance living in the payload</summary>
      public: ThreadPool::Task *Task;
      /// <summary>This contains a ThreadPool::Task (actually a derived type)</summary>
      public: std::uint8_t Payload[sizeof(std::intptr_t)];

    };

    #pragma endregion // SubmittedTask

    /// <summary>Creates an instance of the platform dependent data container</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    /// <returns>The new data container instance</returns>
    /// <remarks>
    ///   This will result in a vanilla instance. The trickery you see in the code
    ///   is just to do one big heap allocation for both the data container and
    ///   the std::thread array (which gets put directly after in memory).
    /// </remarks>
    public: static PlatformDependentImplementation *CreateInstance(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );

    /// <summary>Destroys an instance of the platform dependent data container</summary>
    /// <param name="instance">Instance that will be destroyed</param>
    public: static void DestroyInstance(PlatformDependentImplementation *instance);

    /// <summary>Initializes a platform dependent data members of the process</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    protected: PlatformDependentImplementation(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );

    /// <summary>Destroys the resources owned by the platform dependent data container</summary>
    protected: ~PlatformDependentImplementation();

    /// <summary>Adds another thread to the pool</summary>
    /// <returns>True if the thread was added, false if the pool was full</returns>
    public: bool AddThread();

    /// <summary>Method that is executed by the thread pool's worker threads</summary>
    /// <param name="threadIndex">Unique index of the thread</param>
    private: void runThreadWorkLoop(std::size_t threadIndex);

    /// <summary>Fast-forwards through all tasks, destroying them</summary>
    private: void cancelAllTasks();

    /// <summary>Minimum number of threads to always keep running</summary>
    public: std::size_t MinimumThreadCount; 
    /// <summary>Maximum number of threads to create under high load</summary>
    public: std::size_t MaximumThreadCount; 
    /// <summary>Number of threads currently running</summary>
    public: std::atomic<int> ThreadCount;
    /// <summary>Whether the thread pool is in the process of shutting down</summary>
    public: std::atomic<bool> IsShuttingDown;
    /// <summary>Semaphore that allows one thread for each task to pass</summary>
    public: ::sem_t TaskSemaphore;
    /// <summary>Incremented by the last thread exiting when IsShuttingDown is true</summary>
    public: ::sem_t LightsOut;
    /// <summary>Tasks that have been scheduled for execution in the thread pool</summary>
    public: moodycamel::ConcurrentQueue<SubmittedTask *> ScheduledTasks;
    /// <summary>Submitted tasks for re-use</summary>
    public: ThreadPoolTaskPool<
      SubmittedTask, offsetof(SubmittedTask, Payload)
    > SubmittedTaskPool;
    /// <summary>Status of all allocated thread slots</summary>
    /// <remarks>
    ///   -1: killed, 0: unused, 1: under construction, 2: running, 3: shutting down
    /// </remarks>
    public: std::atomic<std::int8_t> *ThreadStatus;
    /// <summary>Running threads, capacity is always ProcessorCount * 2</summary>
    public: std::thread *Threads;

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation *
  ThreadPool::PlatformDependentImplementation::CreateInstance(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) {
    std::size_t requiredByteCount = (
      sizeof(PlatformDependentImplementation) +
      (sizeof(std::atomic<std::int8_t>[2]) * maximumThreadCount / 2) +
      (sizeof(std::thread[2]) * maximumThreadCount / 2)
    );

    // Allocate memory, perform in-place construction and use the extra memory
    // as the address for the std::thread array
    std::unique_ptr<std::uint8_t[]> buffer(new std::uint8_t[requiredByteCount]);

    // Construct the platform-dependent implementation in-place
    PlatformDependentImplementation *instance = (
      new(buffer.get()) PlatformDependentImplementation(minimumThreadCount, maximumThreadCount)
    );

    // Thread status atomics directly follow the main structure
    // CHECK: Should be do some alignment calculations? std::atomic might be sensitive.
    instance->ThreadStatus = reinterpret_cast<std::atomic<std::int8_t> *>(
      buffer.get() +
      sizeof(PlatformDependentImplementation)
    );
    {
      std::uint8_t *address;
      for(std::size_t index = 0; index < maximumThreadCount; ++index) {
        address = reinterpret_cast<std::uint8_t *>(instance->ThreadStatus + index);
        new(address) std::atomic<std::int8_t>(0);
      }
    }

    // Thread instances follow the main structure and thread status atomics
    // CHECK: Alignment calculations needed here?
    instance->Threads = reinterpret_cast<std::thread *>(
      buffer.release() +
      sizeof(PlatformDependentImplementation) +
      (sizeof(std::atomic<std::int8_t>[2]) * maximumThreadCount / 2)
    );

    return instance;
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::DestroyInstance(
    PlatformDependentImplementation *instance
  ) {

    // Destroy all threads. We expect them to be shut down before this method is called.
    std::size_t threadIndex = instance->MaximumThreadCount;
    while(threadIndex > 0) {
      --threadIndex;

      std::int8_t threadStatus = instance->ThreadStatus[threadIndex].load(
        std::memory_order::memory_order_consume // if() below carries dependency
      );
      if(unlikely(threadStatus > 0)) {
        assert((threadStatus < 1) && u8"Thread finished before its destruction");
        // Detaching is a pretty terrible thing to do, but the alternative is to destroy
        // the threads and have them call std::terminate(). So we assert and let
        // each thread crash in a multi-threaded firework of segmentation faults.
        instance->Threads[threadIndex].detach();
        instance->Threads[threadIndex].~thread();
      } else if(likely(threadStatus < 0)) {
        instance->Threads[threadIndex].join(); // Thread is stopped, call returns instantly.
        instance->Threads[threadIndex].~thread();
      }
    }

    // With the threads gone, destroy all thread status atomics
    threadIndex = instance->MaximumThreadCount;
    while(threadIndex > 0) {
      --threadIndex;
      instance->ThreadStatus[threadIndex].~atomic();
    }

    // Before shutting down, the worker threads should have called cancelAllTasks(),
    // destroying all scheduled tasks without invoking their callbacks.
    assert(instance->ScheduledTasks.size_approx() == 0);

    // Leave the rest up to the normal destructor, them reclaim the memory
    instance->~PlatformDependentImplementation();
    delete[] reinterpret_cast<std::uint8_t *>(instance);

  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::PlatformDependentImplementation(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) :
    MinimumThreadCount(minimumThreadCount),
    MaximumThreadCount(maximumThreadCount),
    ThreadCount(0),
    IsShuttingDown(false),
    TaskSemaphore {0},
    LightsOut(),
    ScheduledTasks(),
    SubmittedTaskPool(),
    ThreadStatus(nullptr),
    Threads(nullptr) {

    // Create a semaphore we use to let the required number of worker threads through
    int result = ::sem_init(&this->TaskSemaphore, 0, 0);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not create the 'TaskSemaphore' semaphore", errorNumber
      );
    }

      // Create a semaphore we use to coordinate the shut down of the thread pool
    {
      auto destroySemaphoreScope = ON_SCOPE_EXIT_TRANSACTION {
        int result = ::sem_close(&this->TaskSemaphore);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert(
          (result != -1) && u8"Task semaphore is successfully destroyed during stack unwind"
        );
      };

      result = ::sem_init(&this->LightsOut, 0, 0);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Could not create the 'LightsOut' semaphore", errorNumber
        );
      }

      destroySemaphoreScope.Commit(); // Initialization succeeded, keep the semaphore
    }
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::~PlatformDependentImplementation() {

    // Safety check, if this assertion triggers you'll send all threads into segfaults.
#if !defined(NDEBUG)
    std::size_t remainingThreadCount = this->ThreadCount.load(
      std::memory_order::memory_order_relaxed
    );
    assert(
      (remainingThreadCount == 0) && u8"All threads have terminated before destruction"
    );
#endif

    // Kill the shutdown semaphore
    int result = ::sem_destroy(&this->TaskSemaphore);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != -1) && u8"'LightsOut' semaphore is successfully destroyed");

    // Kill the task signalling semaphore
    result = ::sem_destroy(&this->TaskSemaphore);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != -1) && u8"'TaskSemaphore' semaphore is successfully destroyed");

  }

  // ------------------------------------------------------------------------------------------- //

  bool ThreadPool::PlatformDependentImplementation::AddThread() {

    // Do not add new threads if the thread pool is shutting down. The thread pool
    // will wait for all threads to exit and then start destroying them, so at that
    // point, messing around in the thread array would lead to disaster.
    {
      bool isShuttingDown = this->IsShuttingDown.load(
        std::memory_order::memory_order_relaxed
      );
      if(isShuttingDown) {
        return false;
      }
    }

    // Look for a free spot in the thread array. Then attempt to reserve it using
    // a C-A-S operation on the status list.
    for(std::size_t index = 0; index < this->MaximumThreadCount; ++index) {
      std::int8_t status = this->ThreadStatus[index].load(
        std::memory_order::memory_order_consume // if() below carries dependency
      );
      if(status < 1) { // Is this slot free?
        for(;;) { // compare_exchange_weak() can fail, so prepare to try again
          bool wasReplaced = this->ThreadStatus[index].compare_exchange_weak(
            status, 1,
            std::memory_order::memory_order_release,
            std::memory_order::memory_order_relaxed
          );

          // Success! We just reserved a spot for a new thread.
          // Attempt to construct the new thread in an exception-safe manner.
          if(wasReplaced) {
            auto returnSlotScope = ON_SCOPE_EXIT_TRANSACTION {
              this->ThreadCount.fetch_sub(1, std::memory_order_release);
              this->ThreadStatus[index].store(0, std::memory_order::memory_order_relaxed);
            };

            if(status == -1) {
              this->Threads[index].join();
              this->Threads[index].~thread();
            }
            this->ThreadCount.fetch_add(1, std::memory_order_release);
            new(this->Threads + index) std::thread(
              &PlatformDependentImplementation::runThreadWorkLoop, this, index
            );

            returnSlotScope.Commit();
            return true;
          }
          if(status != 0) {
            break; // Another thread took the slot during our C-A-S attempt :-(
          }
        } // for(C-A-S attempt)
      }
    } // for(each index in thread array)

    // We went through all slots and they were all occupied
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::runThreadWorkLoop(std::size_t threadIndex) {

    // Mark the thread as running
    this->ThreadStatus[threadIndex].store(2, std::memory_order::memory_order_release);
    ON_SCOPE_EXIT { 
      this->ThreadStatus[threadIndex].store(-1, std::memory_order::memory_order_release);
      std::size_t remainingThreadCount = this->ThreadCount.fetch_sub(
        1, std::memory_order_consume // if() below carries dependency
      );
      if(unlikely(remainingThreadCount == 1)) { // 1 because we're getting the previous value
        int result = ::sem_post(&this->LightsOut);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert((result != -1) && u8"Last thread is able to signal 'LightsOut' on exit");
      }
    };

    // Number of heart beats we went through without anything to do
    std::size_t idleHeartBeatCount = 0;

    // Keep looking for work to do
    for(;;) {
      bool isShuttingDown = this->IsShuttingDown.load(std::memory_order::memory_order_consume);
      if(unlikely(isShuttingDown)) {
        cancelAllTasks();
        break;
      }

      // Wait for work to become available. This semaphore is incremented each time
      // a task is scheduled, meaning it will let one thread from the pool come through
      // to process each task. The wait timeout is our heart beat interval.
      struct ::timespec heartBeatTimeout = Posix::PosixTimeApi::GetTimePlus(
        CLOCK_REALTIME, std::chrono::milliseconds(ThreadPoolConfig::WorkerHeartBeatMilliseconds)
      );
      int result = ::sem_timedwait(&this->TaskSemaphore, &heartBeatTimeout);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(errorNumber == EINTR) {
          continue; // false interrupt
        } else if(errorNumber == ETIMEDOUT) {
          ++idleHeartBeatCount;
          if(idleHeartBeatCount > ThreadPoolConfig::IdleShutDownHeartBeats) {
            int oldThreadCount = this->ThreadCount.fetch_sub(1, std::memory_order_release);
            bool canTerminate = (
              (oldThreadCount > 0) &&
              (static_cast<std::size_t>(oldThreadCount) > this->MinimumThreadCount)
            );
            if(canTerminate) {
              break; // Thread was idle for too long and can shut down
            } else {
              this->ThreadCount.fetch_add(1, std::memory_order_release);
              idleHeartBeatCount = ThreadPoolConfig::IdleShutDownHeartBeats;
            }
          }
        } else {
          Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
            u8"Thread pool thread encountered error waiting for semaphore", errorNumber
          );
        }
      }

      // Yay, we've got work to do. Reset the idle heart beat counter!
      idleHeartBeatCount = 0;

      // Execute the task and return the submitted task container to the pool
      {
        SubmittedTask *submittedTask;
        bool wasDequeued = this->ScheduledTasks.try_dequeue(submittedTask);
        if(wasDequeued) {
          ON_SCOPE_EXIT {
            submittedTask->Task->~Task();
            this->SubmittedTaskPool.ReturnTask(submittedTask);
          };

          submittedTask->Task->operator()();
        }
      } // take and execute one submitted task
    } // for(;;)
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::cancelAllTasks() {
    for(;;) {
      SubmittedTask *submittedTask;
      bool wasDequeued = this->ScheduledTasks.try_dequeue(submittedTask);
      if(wasDequeued) {
        submittedTask->Task->~Task();
        this->SubmittedTaskPool.DeleteTask(submittedTask);
      } else {
        break;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMinimumThreadCount() {
    return ThreadPoolConfig::GuessDefaultMinimumThreadCount(
      static_cast<std::size_t>(::get_nprocs())
    );
  }        

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMaximumThreadCount() {
    return ThreadPoolConfig::GuessDefaultMaximumThreadCount(
      static_cast<std::size_t>(::get_nprocs())
    );
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::ThreadPool(
    std::size_t minimumThreadCount /* = GetDefaultMinimumThreadCount() */,
    std::size_t maximumThreadCount /* = GetDefaultMaximumThreadCount() */
  ) :
    implementation(
      PlatformDependentImplementation::CreateInstance(minimumThreadCount, maximumThreadCount)
    ) {

    auto destroyImplementationScope = ON_SCOPE_EXIT_TRANSACTION {
      PlatformDependentImplementation::DestroyInstance(this->implementation);
    };
    for(std::size_t index = 0; index < minimumThreadCount; ++index) {
      this->implementation->AddThread();
    }
    destroyImplementationScope.Commit();
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {
    
    // Set the shutdown flag (this causes the worker threads to shut down)
    this->implementation->IsShuttingDown.store(
      true, std::memory_order::memory_order_release
    );

    // Wake up all the worker threads by incrementing the semaphore enough times
    // (each thread will see the IsShuttingDown signal and not wait on the semaphore again)
    for(std::size_t index = 0; index < this->implementation->MaximumThreadCount; ++index)  {
      int result = ::sem_post(&this->implementation->TaskSemaphore);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == 0) && u8"Semaphore is incremented successfully for shutdown");
    }

    // The threads have been signalled to shut down, given the wake-up signal and
    // now all that remains to do is hope our user didn't schedule some eternal task.
    for(;;) {
      struct ::timespec waitEndTime = Posix::PosixTimeApi::GetTimePlus(
        CLOCK_REALTIME, std::chrono::milliseconds(5000)
      );
      int result = ::sem_clockwait(
        &this->implementation->LightsOut, CLOCK_MONOTONIC, &waitEndTime
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(errorNumber == EINTR) {
          continue; // spurious wake-up
        } else if(errorNumber == ETIMEDOUT) {
          assert((errorNumber != ETIMEDOUT) && u8"Threads shut down within timeout");
          break;
        } else {
          assert((result != -1) && u8"Semaphore can be waited upon for thread shutdown");
        }
      }

      break; // Semaphore was signalled
    }

    // Eliminate the implementation class. This will also join() or detach() the threads
    // in order to facilitate an orderly shutdown.
    PlatformDependentImplementation::DestroyInstance(this->implementation);

  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t *ThreadPool::getOrCreateTaskMemory(std::size_t payload) {
    std::uint8_t *submittedTaskMemory = reinterpret_cast<std::uint8_t *>(
      this->implementation->SubmittedTaskPool.GetNewTask(payload)
    );
    return (
      submittedTaskMemory + offsetof(PlatformDependentImplementation::SubmittedTask, Payload)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::submitTask(std::uint8_t *taskMemory, Task *task) {
    std::uint8_t *submittedTaskMemory = (
      taskMemory - offsetof(PlatformDependentImplementation::SubmittedTask, Payload)
    );
    PlatformDependentImplementation::SubmittedTask *submittedTask = (
      reinterpret_cast<PlatformDependentImplementation::SubmittedTask *>(
        submittedTaskMemory
      )
    );

    submittedTask->Task = task;

    // Task is ready, schedule it for execution by a worker thread
    bool wasEnqueued = this->implementation->ScheduledTasks.enqueue(submittedTask);
    if(unlikely(!wasEnqueued)) {
      submittedTask->Task->~Task();
      this->implementation->SubmittedTaskPool.DeleteTask(submittedTask);
      throw std::runtime_error(u8"Could not schedule task for thread pool execution");
    }
    
    // Wake up a worker thread (or prevent the next one to become available
    // from going to sleep again)
    int result = ::sem_post(&this->implementation->TaskSemaphore);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Could not increment semaphore for waiting tasks", errorNumber
      );
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_LINUX)
