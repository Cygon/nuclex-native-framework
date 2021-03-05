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

#ifndef NUCLEX_SUPPORT_THREADING_THREAD_H
#define NUCLEX_SUPPORT_THREADING_THREAD_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for for std::uintptr_t
#include <chrono> // for std::chrono::microseconds etc.
#include <thread> // for std::thread

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides supporting methods for threads</summary>
  /// <remarks>
  ///   <para>
  ///     The thread affinity methods provided by this class are limited to 64 CPUs and
  ///     do not provide any methods for querying NUMA nodes (i.e. systems where CPUs are
  ///     provided by two or more physical chips). For situations where extreme thread
  ///     utilization is needed (i.e. AI, raytracing, containers shared among large numbers
  ///     of threads), please use pthreads, libnuma or a portable wrapper.
  ///   </para>
  /// </remarks>
  class Thread {

    /// <summary>Lets the calling thread wait for the specified amount of time</summary>
    /// <param name="time">Duration for which the thread will wait</param>
    public: NUCLEX_SUPPORT_API static void Sleep(std::chrono::microseconds time);

    /// <summary>Determines whether the calling thread belongs to the thread pool</summary>
    /// <returns>True if the calling thread belongs to the thread pool</returns>
    public: NUCLEX_SUPPORT_API static bool BelongsToThreadPool();

    // This method is not cleanly implementable on Microsoft platforms
#if defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)
    /// <summary>Returns a unique ID for the calling thread</summary>
    /// <returns>
    ///   A unique ID that no other thread that's running at the same time will have
    /// </returns>
    /// <remarks>
    ///   This is useful for some lock-free synchronization techniques. It is also used
    ///   as the input to the thread affinity setting methods.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uintptr_t GetCurrentThreadId();
#endif //defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)

    /// <summary>Returns a unique ID for the specified thread</summary>
    /// <returns>
    ///   The unique ID for the specified thread which no other thread that's running at
    ///   the same time will have
    /// </returns>
    /// <remarks>
    ///   This is useful for some lock-free synchronization techniques. It is also used
    ///   as the input to the thread affinity setting methods.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uintptr_t GetStdThreadId(std::thread &thread);

    /// <summary>Checks which CPU cores a thread is allowed to run on</summary>
    /// <param name="threadId">Thread whose CPU affinity mask will be retrieved</param>
    /// <returns>A bit mask where each bit corresponds to a CPU core</returns>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on. So unless you change
    ///     a thread's affinity, this will return a mask of all CPU cores available.
    ///   </para>
    ///   <para>
    ///     See <see cref="SetCpuAffinityMask" /> for a short description of why you may
    ///     or may not want to adjust CPU affinity for a thread.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uint64_t GetCpuAffinityMask(std::uintptr_t threadId);

    /// <summary>Checks which CPU cores the calling thread is allowed to run on</summary>
    /// <returns>A bit mask where each bit corresponds to a CPU core</returns>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on. So unless you change
    ///     a thread's affinity, this will return a mask of all CPU cores available.
    ///   </para>
    ///   <para>
    ///     See <see cref="SetCpuAffinityMask" /> for a short description of why you may
    ///     or may not want to adjust CPU affinity for a thread.
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uint64_t GetCpuAffinityMask();

    /// <summary>Selects the CPU cores on which a thread is allowed to run</summary>
    /// <param name="threadId">ID of the thread whose CPU affinity mask will be changed</param>
    /// <param name="affinityMask">Bit mask of CPU cores the thread can run on</param>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on.
    ///   </para>
    ///   <para>
    ///     In most cases, it is a good idea to leave it that way - for low-thread
    ///     operations, the CPU core is often cycled to ensure heat is generated evenly
    ///     over the whole chip, allowing &quot;TurboBoost&quot; (Intel),
    ///     &quot;TurboCore&quot; (AMD) to raise clock frequencies.
    ///   </para>
    ///   <para>
    ///     For highly threaded operations on the other hand it can make sense to assign
    ///     them to fixed CPU cores. For example, to keep a UI or communications thread
    ///     unclogged, or to optimize performance on NUMA systems (actual multi-CPU systems
    ///     have one memory controller per chip, so if multiple chips massage the same
    ///     memory area, expensive synchronization between the memory controllers via
    ///     the system bus needs to happen).
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void SetCpuAffinityMask(
      std::uintptr_t threadId, std::uint64_t affinityMask
    );

    /// <summary>Selects the CPU cores on which the calling thread is allowed to run</summary>
    /// <param name="affinityMask">Bit mask of CPU cores the thread can run on</param>
    /// <remarks>
    ///   <para>
    ///     For any newly created thread, it is left up to the operating system's thread
    ///     scheduler to decide which CPU core a thread runs on.
    ///   </para>
    ///   <para>
    ///     In most cases, it is a good idea to leave it that way - for low-thread
    ///     operations, the CPU core is often cycled to ensure heat is generated evenly
    ///     over the whole chip, allowing &quot;TurboBoost&quot; (Intel),
    ///     &quot;TurboCore&quot; (AMD) to raise clock frequencies.
    ///   </para>
    ///   <para>
    ///     For highly threaded operations on the other hand it can make sense to assign
    ///     them to fixed CPU cores. For example, to keep a UI or communications thread
    ///     unclogged, or to optimize performance on NUMA systems (actual multi-CPU systems
    ///     have one memory controller per chip, so if multiple chips massage the same
    ///     memory area, expensive synchronization between the memory controllers via
    ///     the system bus needs to happen).
    ///   </para>
    /// </remarks>
    public: NUCLEX_SUPPORT_API static void SetCpuAffinityMask(std::uint64_t affinityMask);

    private: Thread(const Thread &) = delete;
    private: Thread&operator =(const Thread &) = delete;

  };


  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREAD_H
