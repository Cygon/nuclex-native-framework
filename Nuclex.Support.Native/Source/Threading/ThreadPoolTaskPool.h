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

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOLTASKPOOL_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOLTASKPOOL_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Collections/MoodyCamel/concurrentqueue.h"

#include "ThreadPoolConfig.h"

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages reusable tasks for the thread pool</summary>
  /// <typeparam name="TSubmittedTask">Store all informations about a submitted task</typeparam>
  /// <typeparam name="PayloadOffset">Offset at which the variable payload begins</typeparam>
  template<typename TSubmittedTask, std::size_t PayloadOffset>
  class ThreadPoolTaskPool {

    #pragma region struct SubmittedTaskTemplate

#if defined(NUCLEX_SUPPORT_ENABLE_TASK_POOL_VERIFICATION)
    /// <summary>Template for the contents of the submitted task structure</summary>
    /// <remarks>
    ///   Apart from requiring the PayloadSize to be of type std::size_t and
    ///   located at the start of the structure, everything else is up to the owner.
    /// </remarks>
    private: struct SubmittedTaskTemplate {

      /// <summary>Size of the variable payload at the end of the structure</summary>
      public: std::size_t PayloadSize;
      /// <summary>Example element, a pointer to the task instance</summary>
      public: void *Task;
      /// <summary>Placeholder for the variable payload attached to the task</summary>
      public: std::uint8_t Payload[sizeof(std::uintptr_t)];

    };
#endif // defined(NUCLEX_SUPPORT_ENABLE_TASK_POOL_VERIFICATION)

    #pragma endregion // struct SubmittedTaskTemplate

    public: ThreadPoolTaskPool() {
#if defined(NUCLEX_SUPPORT_ENABLE_TASK_POOL_VERIFICATION)
      // This will both check that an attribute 'PayloadSize' is present in the submitted
      // task structure and that it is at the beginning of the structure. If this assertion
      // triggers, your submitted task type is not compatible with the task pool.
      static_assert(
        offsetof(TSubmittedTask, PayloadSize) == offsetof(SubmittedTaskTemplate, PayloadSize)
      );
#endif // defined(NUCLEX_SUPPORT_ENABLE_TASK_POOL_VERIFICATION)
    }

    /// <summary>Destroys all remaining tasks</summary>
    public: ~ThreadPoolTaskPool() {
      DeleteAllRecyclableTasks();
    }

    /// <summary>Destroys all tasks currently waiting to be recycled</summary>
    public: void DeleteAllRecyclableTasks() {
      TSubmittedTask *submittedTask;
      while(this->returnedTasks.try_dequeue(submittedTask)) {
        DeleteTask(submittedTask);
      }
    }

    /// <summary>Creates a new task with the specified payload size</summary>
    /// <param name="payloadSize">Size of the payload the new task must carry</param>
    /// <returns>A new blob containing the 
    public: TSubmittedTask *GetNewTask(std::size_t payloadSize) {
      std::size_t totalRequiredMemory = (PayloadOffset + payloadSize);

      // Try to obtain a returned task with adequate payload size that can
      // be re-used instead of allocating a new one
      if(likely(totalRequiredMemory < ThreadPoolConfig::SubmittedTaskReuseLimit)) {
        TSubmittedTask *submittedTask;
        for(std::size_t attempt = 0; attempt < 3; ++attempt) {
          if(this->returnedTasks.try_dequeue(submittedTask)) {
            if(submittedTask->PayloadSize >= payloadSize) {
              return submittedTask;
            } else {
              DeleteTask(submittedTask);
            }
          } else {
            break; // No more submitted tasks in queue
          }
        } 
      }

      // We found no task that we could re-use, so create a new one
      {
        std::uint8_t *taskMemory = new std::uint8_t[totalRequiredMemory];
        TSubmittedTask *submittedTask = new(taskMemory) TSubmittedTask();
        submittedTask->PayloadSize = payloadSize;
        return submittedTask;
      }
    }

    /// <summary>Checks if a task can be returned to the pool</summary>
    /// <param name="task">Task that will be checked</param>
    /// <returns>True if the task is suitable to be returned to the pool</returns>
    public: static bool IsReturnable(TSubmittedTask *task) {
      std::size_t totalSize = task->PayloadSize + PayloadOffset;
      return (totalSize < ThreadPoolConfig::SubmittedTaskReuseLimit);
    }

    /// <summary>Returns a task to the task pool, allowing for it to be re-used</summary>
    /// <param name="submittedTask">Task that will be returned for re-use</param>
    public: void ReturnTask(TSubmittedTask *submittedTask) {
      //assert(
      //  IsReturnable(submittedTask) && u8"Task is small enough to be returned to the pool"
      //);
      if(IsReturnable(submittedTask)) {
        this->returnedTasks.enqueue(submittedTask);
      } else {
        DeleteTask(submittedTask);
      }
    }

    /// <summary>Frees the memory used by a task</summary>
    /// <param name="submittedTask">Task that will be destroyed</param>
    public: static void DeleteTask(TSubmittedTask *submittedTask) {
      submittedTask->~TSubmittedTask();
      delete[] reinterpret_cast<std::uint8_t *>(submittedTask);
    }

    /// <summary>Tasks that have been given back and wait for their reuse</summary>
    private: moodycamel::ConcurrentQueue<TSubmittedTask *> returnedTasks;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOLTASKPOOL_H
