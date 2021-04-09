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

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOL_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOL_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <future> // for std::packaged_task
#include <functional> // for std::bind

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Distributes tasks to several threads</summary>
  /// <remarks>
  ///   <para>
  ///     On some platforms (the Microsoft ones), creating a new threads is a heavy operation
  ///     that makes it unsuitable for micro tasks, like parallelizing a mere loop.
  ///   </para>
  ///   <para>
  ///     With the thread pool, a bunch of threads are created up front and simply wait for
  ///     a task. This allows tasks of fine granularity to be split into multiple threads
  ///     without having the setup time exceed the gains.
  ///   </para>
  ///   <para>
  ///     Do not use the thread pool for general purpose tasks or waiting on mutexes. It would
  ///     immediately prevent the thread pool from performing work for 1 or more CPU cores due
  ///     to the threads being stuck on the wait.
  ///   </para>
  ///   <para>
  ///     Only use the thread pool if you have real number crunching that can be parallelized
  ///     to as many CPU cores as the system can provide. Performing a single task in
  ///     the background or doing something time consuming (like disk accesses) should always
  ///     be done with std::async or std::thread.
  ///   </para>
  ///   <para>
  ///     Ideally, your tasks would be split into packages that can be done in a millisecond
  ///     or less, allowing even 
  ///   </para>
  /// </remarks>
  class ThreadPool {

    #pragma region class Task

    /// <summary>Base class for tasks that get executed by the thread pool</summary>
    /// <remarks>
    ///   Only used internally and does some creative memory acrobatics. Don't expose!
    /// </remarks>
    private: class Task {

      /// <summary>Terminates the task. If the task was not executed, cancels it</summary>
      public: virtual ~Task() = default;
      /// <summary>Executes the task. Is called on the thread pool thread</summary>
      public: virtual void operator()() = 0;

    };

    #pragma endregion // class Task

    /// <summary>Determines a good base number of threads to keep active</summary>
    /// <returns>The default minimum number of threads for new thread pools</returns>
    public: NUCLEX_SUPPORT_API static std::size_t GetDefaultMinimumThreadCount();

    /// <summary>Determines a good maximum number of threads for a thread pool</summary>
    /// <returns>The default maximum number of threads for new thread pools</returns>
    public: NUCLEX_SUPPORT_API static std::size_t GetDefaultMaximumThreadCount();

    /// <summary>Initializes a new thread pool</summary>
    /// <param name="minimumThreadCount">
    ///   Number of threads that will be created up-front and always stay active
    /// </param>
    /// <param name="maximumThreadCount">
    ///   Highest number of threads to which the thread pool can grow under load
    /// </param>
    public: NUCLEX_SUPPORT_API ThreadPool(
      std::size_t minimumThreadCount = GetDefaultMinimumThreadCount(),
      std::size_t maximumThreadCount = GetDefaultMaximumThreadCount()
    );

    /// <summary>Stops all threads and frees all resources used</summary>
    public: NUCLEX_SUPPORT_API ~ThreadPool();

    /// <summary>Schedules a task to be executed on a worker thread</summary>
    /// <typeparam name="TMethod">
    ///   Type of the method that will be run on a worker thread
    /// </typeparam>
    /// <typeparam name="TArguments">
    ///   Type of the arguments that will be passed to the method when it is called
    /// </typeparam>
    /// <param name="method">Method that will be called from a worker thread</param>
    /// <param name="arguments">Argument values that will be passed to the method</param>
    /// <returns>
    ///   An std::future instance that will provide the result returned by the method
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This method is your main interface to schedule work on threads of the thread
    ///     pool. Despite the slightly template-heavy signature, it is lean and convenient
    ///     to use. Here's an example:
    ///   </para>
    ///   <example>
    ///     <code>
    ///       int test(int a, int b) {
    ///         Thread::Sleep(milliseconds(10));
    ///         return (a * b) - (a + b);
    ///       }
    ///
    ///       int main() {
    ///         ThreadPool myThreadPool;
    ///
    ///         std::future<int> futureResult = myThreadPool.AddTask<&test>(12, 34);
    ///         int result = futureResult.get(); // waits until result is available
    ///       }
    ///     </code>
    ///   </example>
    ///   <para>
    ///     The returned std::future behaves in every way like an std::future used with
    ///     std::async(). You can ignore it (if your task has no return value), wait
    ///     for a result with std::future::wait() or check its status.
    ///   </para>
    ///   <para>
    ///     Don't be shy about ignoring the returned std::future, the task will still
    ///     run and all std::future handling is inside this header, so the compiler has
    ///     every opportunity to optimize it away.
    ///   </para>
    ///   <para>
    ///     If the thread pool is destroyed before starting on a task, the task will be
    ///     canceled. If you did take hold of the std::future instance, that means it
    ///     will throw an std::future_error of type broken_promise in std::future::get().
    ///   </para>
    /// </remarks>
    public: template<typename TMethod, typename... TArguments>
    std::future<typename std::invoke_result<TMethod, TArguments...>::type>
    Schedule(TMethod &&method, TArguments &&... arguments) {
      typedef typename std::invoke_result<TMethod, TArguments...>::type ResultType;
      typedef std::packaged_task<ResultType()> TaskType;

      #pragma region struct PackagedTask

      /// <summary>Custom packaged task that carries the method and parameters</summary>
      struct PackagedTask : public Task {

        /// <summary>Initializes the packaged task</summary>
        /// <param name="method">Method that should be called back by the thread pool</param>
        /// <param name="arguments">Arguments to save until the invocation</param>
        public: PackagedTask(TMethod &&method, TArguments &&... arguments) :
          Task(),
          Callback(
            std::bind(std::forward<TMethod>(method), std::forward<TArguments>(arguments)...)
          ) {}

        /// <summary>Terminates the task. If the task was not executed, cancels it</summary>
        public: ~PackagedTask() override = default;

        /// <summary>Executes the task. Is called on the thread pool thread</summary>
        public: void operator()() override {
          this->Callback();
        }

        /// <summary>Stored method pointer and arguments that will be called back</summary>
        public: TaskType Callback;

      };

      #pragma endregion // struct PackagedTask

      // Construct a new task with a callback to the caller-specified method and
      // saved arguments that can subsequently be scheduled on the thread pool.
      std::uint8_t *taskMemory = getOrCreateTaskMemory(sizeof(PackagedTask));
      PackagedTask *packagedTask = new(taskMemory) PackagedTask(
        std::forward<TMethod>(method), std::forward<TArguments>(arguments)...
      );

      // Grab the result before scheduling the task. If the stars are aligned and
      // the thread pool is churning, it may otherwise happen that the task is
      // completed and destroyed between submitTask() and the call to get_future()
      std::future<ResultType> result = packagedTask->Callback.get_future();

      // Schedule for execution. The task will either be executed (default) or
      // destroyed if the thread pool shuts down, both outcomes will result in
      // the future completing with either a result or in an error state.
      submitTask(taskMemory, packagedTask);

      return result;
    }

    /// <summary>
    ///   Creates (or fetches from the pool) a task with the specified payload size
    /// </summary>
    /// <param name="payload">Size of the task instance</param>
    /// <returns>A new or reused task with at least the requested payload size</returns>
    private: NUCLEX_SUPPORT_API std::uint8_t *getOrCreateTaskMemory(std::size_t payload);

    /// <summary>
    ///   Submits a task (created via getOrCreateTaskMemory()) to the thread pool
    /// </summary>
    /// <param name="taskMemory">Memory block returned by getOrCreateTaskMemory</param>
    /// <param name="task">Task that will be submitted</param>
    private: NUCLEX_SUPPORT_API void submitTask(std::uint8_t *taskMemory, Task *task);

    /// <summary>Structure to hold platform dependent thread and sync objects</summary>
    private: struct PlatformDependentImplementation;
    /// <summary>Platform dependent process and file handles used for the process</summary>
    private: PlatformDependentImplementation *implementation;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOL_H
