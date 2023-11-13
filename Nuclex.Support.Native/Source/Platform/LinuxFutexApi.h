#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_PLATFORM_LINUXFUTEXAPI_H
#define NUCLEX_SUPPORT_PLATFORM_LINUXFUTEXAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_LINUX)

#include <string> // std::string
#include <cstdint> // std::uint8_t and std::size_t

#include <ctime> // for ::timespec

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the Linux futex synchronization API</summary>
  class LinuxFutexApi {

    // ----------------------------------------------------------------------------------------- //
    // These are all for "private" futexes. Which means we hint to the Linux kernel
    // that the futex is private to the calling process (i.e. not in shared memory)
    // and certain assumptions and optimizations for that special case can be made.
    // ----------------------------------------------------------------------------------------- //

    #pragma region enum WaitResult

    /// <summary>Reasons for why <see cref="WaitOnAddress" /> has returned</summary>
    public: enum WaitResult {

      /// <summary>The wait was cancelled because the timeout was reached</summary>>
      TimedOut = -1,
      /// <summary>The wait was interrupted for some reason</summary>
      Interrupted = 0,
      /// <summary>Either the monitored value changed or we woke spuriously</summary>
      ValueChanged = 1

      // We can distinguish between ValueChanged and ManualWake, but we don't need it

    };

    #pragma endregion // enum WaitResult

    /// <summary>Waits for a private futex variable to change its value</summary>
    /// <param name="futexWord">Futex word that will be watched for changed</param>
    /// <param name="comparisonValue">
    ///   Value the futex word is expected to have, the method will return immediately
    ///   when the watched futex word has a different value.
    /// </param>
    /// <returns>
    ///   The reason why the wait method has returned. This method will never report back
    ///   <see cref="WaitResult::TimedOut" /> as a reason because it does not time out.
    /// </returns>
    public: static WaitResult PrivateFutexWait(
      const volatile std::uint32_t &futexWord,
      std::uint32_t comparisonValue
    );

    /// <summary>Waits for a private futex variable to change its value</summary>
    /// <param name="futexWord">Futex word that will be watched for changed</param>
    /// <param name="comparisonValue">
    ///   Value the futex word is expected to have, the method will return immediately
    ///   when the watched futex word has a different value.
    /// </param>
    /// <param name="patience">
    ///   Maximum amount of time to wait before returning even when the value doesn't change
    /// </param>
    /// <returns>The reason why the wait method has returned</returns>
    public: static WaitResult PrivateFutexWait(
      const volatile std::uint32_t &futexWord,
      std::uint32_t comparisonValue,
      const ::timespec &patience
    );

    /// <summary>Wakes a single thread waiting for a futex word to change</summary>
    /// <param name="futexWord">Futex word that is being watched by threads</param>
    public: static void PrivateFutexWakeSingle(
      const volatile std::uint32_t &futexWord
    );

    /// <summary>Wakes all threads waiting for a futex word to change</summary>
    /// <param name="futexWord">Futex word that is being watched by threads</param>
    public: static void PrivateFutexWakeAll(
      const volatile std::uint32_t &futexWord
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_LINUX)

#endif // NUCLEX_SUPPORT_PLATFORM_LINUXFUTEXAPI_H
