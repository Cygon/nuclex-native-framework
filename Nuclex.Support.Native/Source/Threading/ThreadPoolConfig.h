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

#ifndef NUCLEX_SUPPORT_THREADING_THREADPOOLCONFIG_H
#define NUCLEX_SUPPORT_THREADING_THREADPOOLCONFIG_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cmath> // for std::sqrt()

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Configuration options for the thread pool</summary>
  class ThreadPoolConfig {

    /// <summary>Whether the current thread is a thread pool thread</summary>
    public: thread_local static bool IsThreadPoolThread;

    /// <summary>Maximum size of a submitted task to be re-used via the pool</summary>
    /// <remarks>
    ///   <para>
    ///     The thread pool packages all information about a task (method pointer,
    ///     passed arguments, internal bookkeeping) into heap-allocated memory blocks
    ///     that it attempts to reuse in future tasks.
    ///   </para>
    ///   <para>
    ///     This is the size limit, in bytes, beyond which a task's memory block is not
    ///     reused but freed immediately after the task is finished.
    ///   </para>
    ///   <para>
    ///     Otherwise, if the user submits a gigantic task, even just once in a while,
    ///     it would enter the reuse pool and also get re-used for smaller tasks,
    ///     requiring another allocation when another gigantic task is scheduled.
    ///     Eventually, only oversized memory blocks would be circulating around.
    ///   </para>
    /// </remarks>
    public: static const constexpr std::size_t SubmittedTaskReuseLimit = 128;

    /// <summary>Once per how many milliseconds each worker thread wakes up</summary>
    /// <remarks>
    ///   <para>
    ///     Worker threads are immediately woken up through a semaphore if there is work
    ///     to do or if the thread pool is shutting down.
    ///   </para>
    ///   <para>
    ///     If worker threads are idle, however, they will once in a while check if
    ///     they can shut down (until the minimumThreadCount parameter is reached).
    ///     This interval specifies, how often threads will check if they can shut down
    ///     and look for hanging work (the latter should never be the case, but as
    ///     a matter of defensive programming, it is done anyway).
    ///   </para>
    ///   <para>
    ///     Should work be issued at a faster rate than the heart beat interval,
    ///     then this value has no effect. With a value of 50 milliseconds, if you
    ///     generate work (for all threads) at 20 fps, the threads will always
    ///     wake up for work and never due to an idle heartbeat.
    ///   </para>
    ///   <para>
    ///     This value is only used by the Linux implementation of the thread pool
    ///   </para>
    /// </remarks>
    public: static const constexpr std::size_t WorkerHeartBeatMilliseconds = 50;

    /// <summary>Number of heartbeats after which a thread tries shutting down</summary>
    /// <remarks>
    ///   <para>
    ///     If a thread has consecutively woken this number of times due to having
    ///     no work, it will terminate unless the thread pool already is at
    ///     the minimum number of threads specified during construction.
    ///   </para>
    ///   <para>
    ///     This value is only used by the Linux implementation of the thread pool
    ///   </para>
    /// </remarks>
    public: static const constexpr std::size_t IdleShutDownHeartBeats = 10;

    /// <summary>Guesses a good default for the number of threads to keep alive</summary>
    /// <param name="processorCount">Number of processors (CPU cores) in the system</param>
    /// <returns>The default value for the thread pool's minimum thread count</returns>
    /// <remarks>
    ///   <para>
    ///     We want to keep a few threads around in case the thread pool is used for
    ///     one-off tasks. This method tries to guess a reasonable number of threads
    ///     to keep ready for this purpose. To prevent the number from exploding on
    ///     systems we take the square root.
    ///   </para>
    ///   <para>
    ///     If the library's user intends to use the thread pool for massive number
    ///     crunching, a higher minimum thread count can be specified manually.
    ///   </para>
    /// </remarks>
    public: static std::size_t GuessDefaultMinimumThreadCount(
      std::size_t processorCount
    ) {
      std::size_t processorCountSquareRoot = static_cast<std::size_t>(
        std::sqrt(processorCount) + 0.5
      );
      if(processorCountSquareRoot >= 4) {
        return processorCountSquareRoot; // 5 for 22 cores, 6 for 32 cores, 7 for 44 cores, ...
      } else if(processorCountSquareRoot >= 3) {
        return 4; // for fourteen cores or less
      } else {
        return 2; // for six cores or less
      }
    }

    /// <summary>Guesses a good default for the maximum number of threads</summary>
    /// <param name="processorCount">Number of processors (CPU cores) in the system</param>
    /// <returns>The default value for the thread pool's maximum thread count</returns>
    /// <remarks>
    ///   <para>
    ///     It seems to be a good idea to keep this number above the real number of
    ///     CPU cores available. This way, when threads finish, there still enough work
    ///     for all CPU cores that the operating system's thread scheduler can assign
    ///     to cores even if the thread pool is not fully utilized while the user code
    ///     responds to the finished task and generates more work.
    ///   </para>
    ///   <para>
    ///     If a thread pool should always keep a certain number of CPU cores free
    ///     (for example to prioritize communication or UI threads), the user can manually
    ///     specify a different maximum thread count in the constructor.
    ///   </para>
    /// </remarks>
    public: static std::size_t GuessDefaultMaximumThreadCount(
      std::size_t processorCount
    ) {
      return processorCount + GuessDefaultMinimumThreadCount(processorCount);
      //return processorCount * 2; // another option...
    }

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREADPOOLCONFIG_H
