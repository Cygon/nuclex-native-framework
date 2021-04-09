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

#if defined(NUCLEX_SUPPORT_WIN32)

#include "Nuclex/Support/ScopeGuard.h" // for ScopeGuard
#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"

#include "ThreadPoolTaskPool.h" // thread pool settings + task pool
#include "../Helpers/WindowsApi.h" // error handling helpers

#include <cassert> // for assert()
#include <atomic> // for std;:atomic
#include <mutex> // for std;:mutex

#include <VersionHelpers.h> // for ::IsWindowsVistaOrGreater()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of logical processors in the system<summary>
  /// <returns>The number of logical processors available to the system</returns>
  std::size_t countLogicalProcessors() {
    static ::SYSTEM_INFO systemInfo = { 0 };
    if(systemInfo.dwNumberOfProcessors == 0) {
      ::GetSystemInfo(&systemInfo); // There is no failure return...
    }
    return static_cast<std::size_t>(systemInfo.dwNumberOfProcessors);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct ThreadPool::PlatformDependentImplementation {

    #pragma region struct SubmittedTask

    /// <summary>Wraps a callback that can be schuled on a worker thread</summary>
    public: struct SubmittedTask {

      /// <summary>Initializes a new submitted task</summary>
      public: SubmittedTask() : Implementation(nullptr) {}

      /// <summary>
      ///   Destroys the thread pool work item then the task is no longer being recycled
      /// </summary>
      public: ~SubmittedTask() {
        ::CloseThreadpoolWork(this->Work);
      }

      /// <summary>Size of the payload allocated for this task instance</summary>
      public: std::size_t PayloadSize;
      /// <summary>
      ///   The instance of the PlatformDependentimplementation class that owns this task
      /// </summary>
      public: PlatformDependentImplementation *Implementation;
      /// <summary>The thread pool work item, if the new thread pool API is used</summary>
      public: ::TP_WORK *Work;
      // <summary>The task instance living in the payload</summary>
      public: ThreadPool::Task *Task;
      /// <summary>This contains a ThreadPool::Task (actually a derived type)</summary>
      public: std::uint8_t Payload[sizeof(std::intptr_t)];

    };

    #pragma endregion // SubmittedTask

    /// <summary>Initializes a platform dependent data members of the process</summary>
    /// <param name="minimumThreadCount">Minimum number of threads to keep running</param>
    /// <param name="maximumThreadcount">Maximum number of threads to start up</param>
    public: PlatformDependentImplementation(
      std::size_t minimumThreadCount, std::size_t maximumThreadCount
    );
    /// <summary>Shuts down the thread pool and frees all resources it owns</summary>
    public: ~PlatformDependentImplementation();

    /// <summary>Increments the counter for waiting tasks</summary>
    public: void IncrementTaskCount();

    /// <summary>Decrements the counter for waiting tasks</summary>
    public: void DecrementTaskCount();

    /// <summary>Decrements the counter for waiting tasks</summary>
    public: void DecrementTaskCountNoThrow() throw();

    /// <summary>Called by the thread pool to execute a work item</summary>
    /// <param name="parameter">Task the user has queued for execution</param>
    /// <returns>Always 0</returns>
    public: static DWORD WINAPI oldThreadPoolWorkCallback(void *parameter);

    /// <summary>Called by the thread pool to execute a work item</summary>
    /// <param name="instance">
    ///   Lets the callback request additional actions from the thread pool
    /// </param>
    /// <param name="context">Task the user has queued for execution</param>
    /// <param name="workItem">Work item for which this callback was invoked</param>
    public: static void NTAPI newThreadPoolWorkCallback(
      ::TP_CALLBACK_INSTANCE *instance, void *context, ::TP_WORK *workItem
    );

    /// <summary>Whether the thread pool is shutting down</summary>
    public: std::atomic<bool> IsShuttingDown;
    /// <summary>Whether the thread pool should use the Vista-and-later API</summary>
    public: bool UseNewThreadPoolApi;
    /// <summary>Describes this application (WinSDK version etc.) to the thread pool</summary>
    public: ::TP_CALLBACK_ENVIRON NewCallbackEnvironment;
    /// <summary>Thread pool on which tasks get scheduled if new TP api is used</summary>
    public: ::TP_POOL *NewThreadPool;
    /// <summary>Number of tasks that should currently be waiting in the thread pool</summary>
    public: std::atomic<std::size_t> ScheduledTaskCount;
    /// <summary>Signaled by the last thread shutting down</summary>
    public: HANDLE LightsOutEventHandle;
    /// <summary>Submitted tasks for re-use</summary>
    public: ThreadPoolTaskPool<
      SubmittedTask, offsetof(SubmittedTask, Payload)
    > SubmittedTaskPool;

  };

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::PlatformDependentImplementation(
    std::size_t minimumThreadCount, std::size_t maximumThreadCount
  ) :
    IsShuttingDown(false),
    UseNewThreadPoolApi(::IsWindowsVistaOrGreater()),
    NewCallbackEnvironment(),
    NewThreadPool(nullptr),
    ScheduledTaskCount(0),
    LightsOutEventHandle(INVALID_HANDLE_VALUE),
    SubmittedTaskPool() {

    // Create the event we will use to wait for the task queue
    // to flush when the thread pool shuts down again
    this->LightsOutEventHandle = ::CreateEventW(nullptr, TRUE, TRUE, nullptr);
    bool eventCreationFailed = (
      (this->LightsOutEventHandle == 0) ||
      (this->LightsOutEventHandle == INVALID_HANDLE_VALUE)
    );
    if(unlikely(eventCreationFailed)) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not create event for shutdown coordination", lastErrorCode
      );
    }

    // The new thread pool API introduced with Windows Vista allows us to honor
    // the minimum and maximum thread count parameters, so if possible set it up.
    if(this->UseNewThreadPoolApi) {
      auto closeEventScope = ON_SCOPE_EXIT_TRANSACTION{
        BOOL result = ::CloseHandle(this->LightsOutEventHandle);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
        assert((result != FALSE) && u8"Shutdown event is closed on stack unwind");
      };

      ::TpInitializeCallbackEnviron(&this->NewCallbackEnvironment);

      // Create a new thread pool. There is no documentation on how many threads it
      // will create or run by default.
      this->NewThreadPool = ::CreateThreadpool(nullptr);
      if(unlikely(this->NewThreadPool == nullptr)) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not create thread pool (using Vista and later API)", lastErrorCode
        );
      }

      // Apply configuration options
      {
        auto closeThreadPoolScope = ON_SCOPE_EXIT_TRANSACTION{
          ::CloseThreadpool(this->NewThreadPool);
        };

        // Set the minimum and maximum number of threads the thread pool can use.
        // Without doing this, we have no idea how many threads the thread pool would use.
        ::SetThreadpoolThreadMaximum(
          this->NewThreadPool, static_cast<DWORD>(maximumThreadCount)
        );
        // This method can't fail, apparently

        BOOL result = ::SetThreadpoolThreadMinimum(
          this->NewThreadPool, static_cast<DWORD>(minimumThreadCount)
        );
        if(unlikely(result == FALSE)) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not set minimum number of thread pool threads", lastErrorCode
          );
        }

        // Connect the environment structure describing this application with
        // the thread pool. Needed to submit tasks to this pool instead of the default pool
        // (which probably gets created when the first task is submitted to it).
        ::SetThreadpoolCallbackPool(&this->NewCallbackEnvironment, this->NewThreadPool);
        // Another method without an error return

        closeThreadPoolScope.Commit(); // Everything worked out, don't close the thread pool
      }

      closeEventScope.Commit(); // Success, don't close the shutdown event
    } // if new thread pool API used
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::PlatformDependentImplementation::~PlatformDependentImplementation() {

    // Destroy all recyclable task so they're gone before the thread pool
    // itself is shut down (unsure if necessary, but definitely cleaner this
    // way since the tasks keep a pointer to the thread pool)
    this->SubmittedTaskPool.DeleteAllRecyclableTasks();

    // Now the thread pool can be safely shut down
    if(this->UseNewThreadPoolApi) {
      ::CloseThreadpool(this->NewThreadPool);
    }

    // Everything is shut down and we can safely delete this event, too
    BOOL result = ::CloseHandle(this->LightsOutEventHandle);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result != FALSE) && u8"Shutdown event is successfully destroyed");

  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::IncrementTaskCount() {
    std::size_t previousTaskCount = this->ScheduledTaskCount.fetch_add(
      1, std::memory_order::memory_order_consume // if() below carries dependency
    );
    if(previousTaskCount == 0) { // If there were no tasks scheduled before
      BOOL result = ::ResetEvent(this->LightsOutEventHandle);
      if(unlikely(result == FALSE)) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not reset 'LightsOut' event upon first scheduled task", lastErrorCode
        );
      }
    }
  }
  
  // ------------------------------------------------------------------------------------------- //
  
  void ThreadPool::PlatformDependentImplementation::DecrementTaskCount() {
    std::size_t previousTaskCount = this->ScheduledTaskCount.fetch_sub(
      1, std::memory_order::memory_order_consume // if() below carries dependency
    );
    if(previousTaskCount == 1) { // If the last task was just processed
      BOOL result = ::SetEvent(this->LightsOutEventHandle);
      if(unlikely(result == FALSE)) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not set 'LightsOut' event upon last scheduled task exiting", lastErrorCode
        );
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void ThreadPool::PlatformDependentImplementation::DecrementTaskCountNoThrow() throw() {
    std::size_t previousTaskCount = this->ScheduledTaskCount.fetch_sub(
      1, std::memory_order::memory_order_consume // if() below carries dependency
    );
    if(previousTaskCount == 1) { // If the last task was just processed
      BOOL result = ::SetEvent(this->LightsOutEventHandle);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result != FALSE) && u8"'LightsOut' event is successfully signalled");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  DWORD WINAPI ThreadPool::PlatformDependentImplementation::oldThreadPoolWorkCallback(
    void *parameter
  ) {
    SubmittedTask *submittedTask = reinterpret_cast<SubmittedTask *>(parameter);
    PlatformDependentImplementation &implementation = *submittedTask->Implementation;

    // Make sure to always update the task counter and to signal the 'LightsOut' event
    // if the task counter reaches zero (used on shutdown to wait for tasks to flush).
    ON_SCOPE_EXIT {
      implementation.DecrementTaskCountNoThrow();
    };

    // See if the thread pool is shutting down. If so, fast-forward through any scheduled
    // task, destroying it without executing it (this will cancel the owner's std::futures).
    bool isShuttingDown = submittedTask->Implementation->IsShuttingDown.load(
      std::memory_order::memory_order_consume // if() below carries dependency
    );
    if(unlikely(isShuttingDown)) {
      submittedTask->Task->~Task();
      implementation.SubmittedTaskPool.DeleteTask(submittedTask);
    } else {
      ON_SCOPE_EXIT {
        submittedTask->Task->~Task();
        implementation.SubmittedTaskPool.ReturnTask(submittedTask);
      };
      submittedTask->Task->operator()();
    }

    return 0;
  }

  // ------------------------------------------------------------------------------------------- //

  void NTAPI ThreadPool::PlatformDependentImplementation::newThreadPoolWorkCallback(
    ::TP_CALLBACK_INSTANCE *instance, void *context, ::TP_WORK *workItem
  ) {
    (void)instance;
    (void)workItem;
    oldThreadPoolWorkCallback(context);
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMinimumThreadCount() {
    return ThreadPoolConfig::GuessDefaultMinimumThreadCount(
      countLogicalProcessors()
    );
  }        

  // ------------------------------------------------------------------------------------------- //

  std::size_t ThreadPool::GetDefaultMaximumThreadCount() {
    return ThreadPoolConfig::GuessDefaultMaximumThreadCount(
      countLogicalProcessors()
    );
  }

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::ThreadPool(
    std::size_t minimumThreadCount /* = GetDefaultMinimumThreadCount() */,
    std::size_t maximumThreadCount /* = GetDefaultMaximumThreadCount() */
  ) :
    implementation(
      new PlatformDependentImplementation(minimumThreadCount, maximumThreadCount)
    ) {}

  // ------------------------------------------------------------------------------------------- //

  ThreadPool::~ThreadPool() {
    this->implementation->IsShuttingDown.store(
      true, std::memory_order::memory_order_release
    );

    // Wait until all tasks have been flushed from the queue. With the shutdown flag
    // set, the task pool callbacks will skip over all tasks, deleting them without
    // calling and thereby putting their attached std::futures into an error state.
    DWORD result = ::WaitForSingleObject(
      this->implementation->LightsOutEventHandle, 5000
    );
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert(
      (result == WAIT_OBJECT_0) && u8"All tasks flushed before the thread pool is destroyed"
    );

#if !defined(NDEBUG)
    // Additional safety check, there should be no more tasks in the queue
    std::size_t remainingTaskCount = this->implementation->ScheduledTaskCount.load(
      std::memory_order::memory_order_consume // assert() below carries dependency
    );
    assert((remainingTaskCount == 0) && u8"No thread pool tasks remain during shutdown");
#endif

    delete this->implementation;
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t *ThreadPool::getOrCreateTaskMemory(std::size_t payload) {
    PlatformDependentImplementation::SubmittedTask *submittedTask = (
      this->implementation->SubmittedTaskPool.GetNewTask(payload)
    );
    if(unlikely(submittedTask->Implementation == nullptr)) {
      submittedTask->Implementation = this->implementation;
      if(this->implementation->UseNewThreadPoolApi) {
        submittedTask->Work = ::CreateThreadpoolWork(
          &PlatformDependentImplementation::newThreadPoolWorkCallback,
          reinterpret_cast<void *>(submittedTask),
          //nullptr,
          &this->implementation->NewCallbackEnvironment
        );
        if(unlikely(submittedTask->Work == nullptr)) {
          DWORD lastErrorCode = ::GetLastError();
          this->implementation->SubmittedTaskPool.DeleteTask(submittedTask);
          Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not create thread pool work item", lastErrorCode
          );
        }
      }
    }

    std::uint8_t *submittedTaskMemory = reinterpret_cast<std::uint8_t *>(submittedTask);
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

    // Increment task count before executing so we don't risk the task finishing
    // before the increment, dropping the counter lower than 0. If this is
    // the first task being scheduled after the thread pool was idle, reset
    // our 'LightsOut' event so the thread pool destructor knows to wait.
    {
      auto deleteTaskScope = ON_SCOPE_EXIT_TRANSACTION {
        this->implementation->SubmittedTaskPool.DeleteTask(submittedTask);
      };
      this->implementation->IncrementTaskCount();
      deleteTaskScope.Commit();
    }

    // Schedule the task for execution
    if(this->implementation->UseNewThreadPoolApi) {
      ::SubmitThreadpoolWork(submittedTask->Work);
    } else {
      ::QueueUserWorkItem(
        &PlatformDependentImplementation::oldThreadPoolWorkCallback,
        submittedTask,
        WT_EXECUTEDEFAULT
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // defined(NUCLEX_SUPPORT_WIN32)
