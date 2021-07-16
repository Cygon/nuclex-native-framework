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

#ifndef NUCLEX_SUPPORT_THREADING_LATCH_H
#define NUCLEX_SUPPORT_THREADING_LATCH_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <chrono> // for std::chrono::microseconds

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Blocks threads unless its counter has reached zero</summary>
  /// <remarks>
  ///   <para>
  ///     This is sometimes also called a reverse-counting semaphore. It will only
  ///     let threads through if the counter is zero at the time of the Wait() call.
  ///   </para>
  ///   <para>
  ///     This behavior is useful if you need to wait for a series of tasks to finish or
  ///     a set of resources to complete a process before shutting down.
  ///   </para>
  /// </remarks>
  class Latch {

    /// <summary>Initializes a new countdown latch with the specified initial count</summary>
    /// <param name="initialCount">
    ///   Initial number of decrements needed for the countdown latch to become open
    /// </param>
    public: NUCLEX_SUPPORT_API Latch(std::size_t initialCount = 0);

    /// <summary>Frees all resources owned by the countdown latch</summary>
    /// <remarks>
    ///   There should not be any threads waiting on the countdown latch when it is
    ///   destroyed. The behavior for such threats is undefined, they may hang forever,
    //    they may receive an exception or the entire process may be terminated.
    /// </remarks>
    public: NUCLEX_SUPPORT_API ~Latch();

    /// <summary>Increments the latch, blocking any threads from going through</summary>
    /// <param name="count">Number of times the latch will be incremented</param>
    public: NUCLEX_SUPPORT_API void Post(std::size_t count = 1);

    /// <summary>Decrements the latch counter</summary>
    /// <param name="count">Number of times the latch will be decremented</param>
    public: NUCLEX_SUPPORT_API void CountDown(std::size_t count = 1);

    /// <summary>Waits until the latch's count has reached zero</summary>
    /// <remarks>
    ///   This caues the calling thread to block if the latch's count hasn't already
    ///   reached zero. If the thread is blocked, it will stay so until another
    ///   thread calls <see cref="Decrement" /> on the latch.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void Wait() const;

    /// <summary>Waits until the latch's count has reached zero or a timeout occurs</summary>
    /// <param name="patience">How long to wait for the latch before giving up</param>
    /// <returns>
    ///   True if the latch counter reached zero and let the thread through,
    ///   false if the timeout elapsed and the latch counter was still greater than zero.
    /// <returns>
    /// <remarks>
    ///   This causes the calling thread to block if the latch counter didn't already
    ///   reach zero. If the thread is blocked, it will stay so until another
    ///   thread calls <see cref="Decrement" /> on the latch or until the specified
    ///   patience time has elapsed.
    /// </remarks>
    public: NUCLEX_SUPPORT_API bool WaitFor(
      const std::chrono::microseconds &patience
    ) const;

    //public: void WaitUntil(const std::chrono::time_point< &patience);

    /// <summary>Structure to hold platform dependent process and file handles</summary>
    private: struct PlatformDependentImplementationData;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: const PlatformDependentImplementationData &getImplementationData() const;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: PlatformDependentImplementationData &getImplementationData();
#if defined(NUCLEX_SUPPORT_LINUX)
    alignas(8) unsigned char implementationDataBuffer[16];
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    unsigned char implementationDataBuffer[sizeof(std::size_t) * 12]; // matches HANDLE size
#else // Posix
    unsigned char implementationDataBuffer[96];
#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_LATCH_H
