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

#include "Nuclex/Support/Threading/Thread.h"

#if defined(NUCLEX_SUPPORT_LINUX)
#include "Posix/PosixProcessApi.h" // for PosixProcessApi
#include <ctime> // for ::clock_gettime() and ::clock_nanosleep()
#include <cstdlib> // for ldiv_t
#include <algorithm> // for std::min()
#elif defined(NUCLEX_SUPPORT_WIN32)
#include "../Helpers/WindowsApi.h" // for ::Sleep(), ::GetCurrentThreadId() and more
#endif

#include "ThreadPoolConfig.h" // for ThreadPoolConfig::IsThreadPoolThread

#include <thread> // for std::thread
#include <cstring> // for std::memcpy()
#include <cassert> // for assert()

// Design: currently this does not take NUMA and >64 CPUs into account
//
// Reasoning:
//
// While there's potential to use many more cores than the average application in 2020 uses,
// use cases with more than 64 cores are usually found in AI and raytracing. I'm making the bet
// here that consumer CPUs will not grow above 64 cores for a decade.
//
// When dealing with more than 64 cores, additional precautions are needed for optimal
// performance, such as being aware of NUMA nodes (how many physical chips there are.).
// On Linux, this requires libnuma. On Windows, this is done via "processor groups".
// Microsoft's API also exposes up to 64 cores without processor group awareness.
//
// Counter arguments:
//
// If more than 64 cores become very common, applications built with Nuclex.Support.Native that
// make use of CPU affinity will all be hogging the lower 64 cores.
//

namespace {

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WIN32)
  /// <summary>Figures out the thread affinity mask for the specified thread</summary>
  /// <param name="windowsThreadHandle">
  ///   Handle of the thread or current thread pseudo handle for the thread to check
  /// </param>
  /// <returns>
  ///   The affinity mask indicating which CPUs the thread can be scheduled on
  /// </returns>
  std::uint64_t getWindowsThreadAffinityMask(HANDLE windowsThreadHandle) {

    // Form an affinity mask field that is not an INVALID_ARGUMENT but also does
    // as little damage as possible
    std::uint64_t allCpusAffinity = 0;
    {
      // Count CPUs in CPU group 0 only so we don't risk an INVALID_ARGUMENT. This of course
      // assumes that the thread is running on group 0.
      DWORD cpuCount = ::GetActiveProcessorCount(0);
      for(std::size_t index = 0; index < cpuCount; ++index) {
        allCpusAffinity |= (std::uint64_t(1) << index);
      }
    }

    // Assign a temporary, unwanted thread affinity. That's the only way to query
    // a thread's CPU affinity on Windows :-(
    DWORD_PTR previousAffinityMask = ::SetThreadAffinityMask(
      windowsThreadHandle, *reinterpret_cast<DWORD_PTR *>(&allCpusAffinity)
    );
    if(previousAffinityMask == 0) {
      DWORD errorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not change thread affinity via ::SetThreadAffinityMask()", errorCode
      );
    }

    // Revert the affinity to what it was before. If this fails, this nice getter method
    // will actually have changed the thread's affinity for good :-(
    DWORD_PTR temporaryAffinityMask = ::SetThreadAffinityMask(
      windowsThreadHandle, previousAffinityMask
    );
    if(temporaryAffinityMask == 0) {
      DWORD errorCode = ::GetLastError();
      Nuclex::Support::Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not revert thread affinity via ::SetThreadAffinityMask()", errorCode
      );
    }

    // What's this? Microsoft abused DWORD_PTR to hold the number of threads, it's a pointer,
    // so it's 32 bits on x86 builds and reinterpreting it as a std::uint64_t would access
    // random stack memory, so we reinterpret as a pointer-sized integer (std::uintptr_t) and
    // then cast to a 64 bit integer...
    return static_cast<std::uint64_t>(
      *reinterpret_cast<std::uintptr_t *>(&previousAffinityMask)
    );
  }
#endif // defined(NUCLEX_SUPPORT_WIN32)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  /// <summary>Queries the CPU affinity mask of the specified thread</summary>
  /// <param name="thread">Thread for which the affinity mask will be queried</param>
  /// <returns>
  ///   The affinity mask indicating which CPUs the thread can be scheduled on
  /// </returns>
  std::uint64_t queryPThreadThreadAffinity(const ::pthread_t &thread) {

    // Query the affinity into pthreads' cpu_set_t
    ::cpu_set_t cpuSet;
    //CPU_ZERO(&cpuSet);
    int errorNumber = ::pthread_getaffinity_np(thread, sizeof(cpuSet), &cpuSet);
    if(errorNumber != 0) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Error querying CPU affinity via pthread_getaffinity_np()", errorNumber
      );
    }

    // Now turn it into a bit mask
    std::uint64_t result = 0;
    {
      std::size_t maxCpuIndex = std::min(64, CPU_SETSIZE);
      for(std::size_t index = 0; index < maxCpuIndex; ++index) {
        if(CPU_ISSET(index, &cpuSet)) {
          result |= (1 << index);
        }
      }
    }

    return result;

  }
#endif // !defined(NUCLEX_SUPPORT_WIN32)
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_WIN32)
  /// <summary>
  ///   Updates the affinity mask of the specified thread, changin which CPUs is may be
  ///   scheduled on
  /// </summary>
  /// <param name="thread">Thread whose affinity mask will be changed</param>
  /// <param name="affinityMask">
  ///   New affinity mask where each bit allows the CPU with the respective index to run
  ///   the thread
  /// </param>
  void changePThreadThreadAffinity(const ::pthread_t &thread, std::uint64_t affinityMask) {

    // Translate the affinity mask into cpu_set_t
    ::cpu_set_t cpuSet;
    {
      CPU_ZERO(&cpuSet);

      std::size_t maxCpuIndex = std::min(64, CPU_SETSIZE);
      for(std::size_t index = 0; index < maxCpuIndex; ++index) {
        if((affinityMask & (1 << index)) != 0) {
          CPU_SET(index, &cpuSet);
        }
      }
    }

    // Apply the affinity setting
    int errorNumber = ::pthread_setaffinity_np(thread, sizeof(cpuSet), &cpuSet);
    if(errorNumber != 0) {
      Nuclex::Support::Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Error changing CPU affinity via pthread_setaffinity_np()", errorNumber
      );
    }

  }
#endif // !defined(NUCLEX_SUPPORT_WIN32)
  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  void Thread::Sleep(std::chrono::microseconds time) {
#if defined(NUCLEX_SUPPORT_WIN32)
    std::int64_t microseconds = time.count();
    if(microseconds > 0) {
      microseconds += std::int64_t(500); // To round to nearest mllisecond
      ::Sleep(static_cast<DWORD>(microseconds / std::int64_t(1000)));
    }
#elif defined(NUCLEX_SUPPORT_LINUX)
    // Calculate the point in time at which sleep should finish
    ::timespec endTime;
    {
      int result = ::clock_gettime(CLOCK_MONOTONIC, &endTime);
      if(result == -1) {
        int errorNumber = errno;
        Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Error retrieving current time via clock_gettime()", errorNumber
        );
      }

      // Calculate the future point in time by adding the requested number of microseconds
      {
        const std::size_t NanosecondsPerMicrosecond = 1000; // 1,000 ns = 1 μs
        const std::size_t MicrosecondsPerSecond = 1000000; // 1,000,000 μs = 1 s
        const std::size_t NanosecondsPerSecond = 1000000000; // 1,000,000,000 ns = 1 s

        // timespec has seconds and nanoseconds, so divide the milliseconds into full seconds
        // and remainder milliseconds to deal with this
        ::ldiv_t divisionResults = ::ldiv(time.count(), MicrosecondsPerSecond);

        // If the summed nanoseconds add up to more than one second, increment the timespec's
        // seconds, otherwise just assign the summed nanoseconds.
        std::size_t nanoseconds = (
          divisionResults.rem * NanosecondsPerMicrosecond + endTime.tv_nsec
        );
        if(nanoseconds >= NanosecondsPerSecond) {
          endTime.tv_sec += divisionResults.quot + 1;
          endTime.tv_nsec = nanoseconds - NanosecondsPerSecond;
        } else {
          endTime.tv_sec += divisionResults.quot;
          endTime.tv_nsec = nanoseconds;
        }
      }
    }

    // Now attempt to sleep until the calculated point in time
    for(;;) {
      int result = ::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &endTime, nullptr);
      if(result == 0) {
        return;
      } else if(result != EINTR) {
        int errorNumber = errno;
        Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Error pausing thread via ::clock_nanosleep()", errorNumber
        );
      }
    }
#else
    std::this_thread::sleep_for(time);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  bool Thread::BelongsToThreadPool() {
    return ThreadPoolConfig::IsThreadPoolThread;
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)
  std::uintptr_t Thread::GetCurrentThreadId() {
#if defined(NUCLEX_SUPPORT_WIN32)
    throw std::logic_error(u8"This method cannot be implementation on Windows");

    thread_local static HANDLE duplicatedThreadHandle = INVALID_HANDLE_VALUE;
    assert(
      (sizeof(std::uintptr_t) >= sizeof(HANDLE)) &&
      u8"Windows thread handle (HANDLE) can be stored inside an std::uintptr_t"
    );

    if(duplicatedThreadHandle == INVALID_HANDLE_VALUE) {
      //HANDLE currentThread = ::GetCurrentThread();
      // Error checking...
      //duplicatedThreadHandle = ::DuplicateHandle(currentThread...);
      // Error checking
    }

    std::uintptr_t result = 0;
    *reinterpret_cast<HANDLE *>(&result) = duplicatedThreadHandle;
    return result;
#else // LINUX and POSIX
    ::pthread_t currentThreadIdentity = ::pthread_self();
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );

    std::uintptr_t result = 0;
    *reinterpret_cast<::pthread_t *>(&result) = currentThreadIdentity;
    return result;
#endif
  }
#endif // defined(MICROSOFTS_API_ISNT_DESIGNED_SO_POORLY)
  // ------------------------------------------------------------------------------------------- //

  std::uintptr_t Thread::GetStdThreadId(std::thread &thread) {
#if defined(NUCLEX_SUPPORT_WIN32)
    assert(
      (sizeof(std::uintptr_t) >= sizeof(HANDLE)) &&
      u8"Windows thread handle (HANDLE) can be stored inside an std::uintptr_t"
    );
    void *nativeHandleAsPointer = thread.native_handle();
    HANDLE nativeHandle = *reinterpret_cast<HANDLE *>(&nativeHandleAsPointer);

    std::uintptr_t result = 0;
    *reinterpret_cast<HANDLE *>(&result) = nativeHandle;
    return result;
#else // LINUX and POSIX
    ::pthread_t threadIdentity = thread.native_handle();
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );

    // This results in efficient code. A reinterpret_cast would break strict aliasing.
    std::uintptr_t result = 0;
    std::memcpy(&result, &threadIdentity, sizeof(threadIdentity));
    return result;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t Thread::GetCpuAffinityMask(std::uintptr_t threadId) {
#if defined(NUCLEX_SUPPORT_WIN32)
    assert(
      (sizeof(std::uintptr_t) >= sizeof(HANDLE)) &&
      u8"Windows thread id (DWORD) can be stored inside an std::uintptr_t"
    );

    HANDLE threadHandle = *reinterpret_cast<HANDLE *>(&threadId);
    return getWindowsThreadAffinityMask(threadHandle);
#else // LINUX and POSIX
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );

    // This results in efficient code. A reinterpret_cast would break strict aliasing.
    ::pthread_t thread;
    std::memcpy(&thread, &threadId, sizeof(thread));
    return queryPThreadThreadAffinity(thread);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t Thread::GetCpuAffinityMask() {
#if defined(NUCLEX_SUPPORT_WIN32)
    HANDLE currentThreadPseudoHandle = ::GetCurrentThread();
    return getWindowsThreadAffinityMask(currentThreadPseudoHandle);
#else // LINUX and POSIX
    ::pthread_t currentThread = ::pthread_self();
    return queryPThreadThreadAffinity(currentThread);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void Thread::SetCpuAffinityMask(std::uintptr_t threadId, std::uint64_t affinityMask) {
#if defined(NUCLEX_SUPPORT_WIN32)
    assert(
      (sizeof(std::uintptr_t) >= sizeof(HANDLE)) &&
      u8"Windows thread id (DWORD) can be stored inside an std::uintptr_t"
    );

    HANDLE threadHandle = *reinterpret_cast<HANDLE *>(&threadId);

    // Update the thread's affinity. The parameter is a DWORD_PTR abused to hold a bit mask
    // for either 32 CPU cores (x86) or 64 CPU cores (amd64).
    DWORD_PTR previousAffinityMask = ::SetThreadAffinityMask(
      threadHandle, *reinterpret_cast<DWORD_PTR *>(&affinityMask)
    );
    if(previousAffinityMask == 0) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not change thread affinity via ::SetThreadAffinityMask()", errorCode
      );
    }
#else // LINUX and POSIX
    assert(
      (sizeof(std::uintptr_t) >= sizeof(::pthread_t)) &&
      u8"PThread thread identifier can be stored inside an std::uintptr_t"
    );

    // This results in efficient code. A reinterpret_cast would break strict aliasing.
    ::pthread_t thread;
    std::memcpy(&thread, &threadId, sizeof(thread));
    changePThreadThreadAffinity(thread, affinityMask);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void Thread::SetCpuAffinityMask(std::uint64_t affinityMask) {
#if defined(NUCLEX_SUPPORT_WIN32)
    HANDLE currentThreadPseudoHandle = ::GetCurrentThread();

    // Assign a temporary, unwanted thread affinity. That's the only way to query
    // a thread's CPU affinity on Windows
    DWORD_PTR previousAffinityMask = ::SetThreadAffinityMask(
      currentThreadPseudoHandle, *reinterpret_cast<DWORD_PTR *>(&affinityMask)
    );
    if(previousAffinityMask == 0) {
      DWORD errorCode = ::GetLastError();
      Helpers::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not change thread affinity via ::SetThreadAffinityMask()", errorCode
      );
    }
#else // LINUX and POSIX
    ::pthread_t currentThread = ::pthread_self();
    changePThreadThreadAffinity(currentThread, affinityMask);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
