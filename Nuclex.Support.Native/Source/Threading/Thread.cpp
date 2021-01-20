#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#if defined(NUCLEX_SUPPORT_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NO_MINMAX
#include <Windows.h> // for ::Sleep() and ::GetCurrentThreadId()
#elif defined(NUCLEX_SUPPORT_LINUX)
#include <ctime> // for ::clock_gettime() and ::clock_nanosleep()
#include <cstdlib> // for ldiv_t
#include "../Helpers/PosixApi.h"
#endif

#include <thread> // for std::thread

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  void Thread::Sleep(std::chrono::microseconds time) {
#if defined(NUCLEX_SUPPORT_WIN32)
    std::int64_t milliseconds = time.count();
    if(milliseconds > 0) {
      milliseconds += std::int64_t(500);
      milliseconds /= std::int64_t(1000);
      ::Sleep(static_cast<DWORD>(milliseconds));
    }
#elif false && defined(NUCLEX_SUPPORT_LINUX)
    const static long int MicrosecondsPerSecond = 1000000L;
    const static long int NanosecondsPerMicrosecond = 1000L;

    //std::int64_t microseconds = time.count();
    //if(microseconds < MicrosecondsPerSecond) {

    // Is the delay under 1 second? Use a simple relative sleep
    ::timespec endTime;

    // Get current time
    int result = ::clock_gettime(CLOCK_MONOTONIC, &endTime);
    if(result != 0) {
      //int errorNumber = errno;
      Helpers::PosixApi::ThrowExceptionForSystemError(
        u8"Error retrieving current time via clock_gettime()", result
      );
    }

    // Calculate sleep end time
    {
      ldiv_t result = ::ldiv(static_cast<long int>(time.count()), MicrosecondsPerSecond);
      endTime.tv_sec += result.quot;
      endTime.tv_nsec += result.rem * NanosecondsPerMicrosecond;
      if(endTime.tv_nsec >= NanosecondsPerMicrosecond * MicrosecondsPerSecond) {
        endTime.tv_nsec -= NanosecondsPerMicrosecond * MicrosecondsPerSecond;
        endTime.tv_sec -= 1U;
      }
    }

    // Now attempt to sleep until the calculated point in time
    for(;;) {
      int result = ::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &endTime, nullptr);
      if(result == 0) {
        return;
      } else if(result != EINTR) {
        //int errorNumber = errno;
        Helpers::PosixApi::ThrowExceptionForSystemError(
          u8"Error pausing thread via ::clock_nanosleep()", result
        );
      }
    }
#else
    std::this_thread::sleep_for(time);
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  bool Thread::BelongsToThreadPool() {
    return false; // Since we haven't implemented a thread pool yet, this is easy ;-)
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WANT_USELESS_THREAD_ID_QUERY)
  std::uintptr_t Thread::GetCurrentThreadId() {
#if defined(NUCLEX_SUPPORT_WIN32)
    return static_cast<std::uintptr_t>(::GetCurrentThreadId());
#else
    return static_cast<std::uintptr_t>(std::thread::get_id());
//#elif defined(NUCLEX_SUPPORT_LINUX)
#endif
  }
#endif
  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
