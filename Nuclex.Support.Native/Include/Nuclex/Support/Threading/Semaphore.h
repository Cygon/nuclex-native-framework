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

#ifndef NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
#define NUCLEX_SUPPORT_THREADING_SEMAPHORE_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <chrono> // for std::chrono::microseconds

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets only a specific number of threads enter at the same time</summary>
  /// <remarks>
  ///   <para>
  ///     This is a completely vanilla semaphore implementation that either delegates
  ///     to the platform's threading library or uses available threading primitives to
  ///     build the required behavior.
  ///   </para>
  ///   <para>
  ///     Using it grants you automatic resource management, reduced header dependencies
  ///     and guaranteed behavior, including actual relative timeouts on Posix platforms
  ///     where the default implementation would use wall clock (meaning clock adjustment
  ///     sensitive) timeouts.
  ///   </para>
  ///   <para>
  ///     It's at least as fast as your platform's native semaphore, likely much faster.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE Semaphore {

    /// <summary>Initializes a new semaphore with the specified initial count</summary>
    /// <param name="initialCount">
    ///   Initial number of threads that the semaphore will let through
    /// </param>
    public: NUCLEX_SUPPORT_API Semaphore(std::size_t initialCount = 0);

    /// <summary>Frees all resources owned by the semaphore</summary>
    /// <remarks>
    ///   There should not be any threads waiting on the semaphore when it is destroyed.
    ///   The behavior for such threats is undefined, they may hang forever, they
    ///   may receive an exception or the entire process may be terminated.
    /// </remarks>
    public: NUCLEX_SUPPORT_API ~Semaphore();

    /// <summary>Increments the semaphore, letting one more thread through</summary>
    /// <param name="count">Number of times the semaphore will be incremented</param>
    public: NUCLEX_SUPPORT_API void Post(std::size_t count = 1);

    /// <summary>
    ///   Waits until the semaphore has a count above zero, then decrements the count
    /// </summary>
    /// <remarks>
    ///   This caues the calling thread to block if the semaphore didn't already have
    ///   a positive count. If the thread is blocked, it will stay so until another
    ///   thread calls <see cref="Post" /> on the semaphore.
    /// </remarks>
    public: NUCLEX_SUPPORT_API void WaitThenDecrement();

    /// <summary>
    ///   Waits until the semaphore has a count above zero, then decrements the count
    /// </summary>
    /// <param name="patience">How long to wait for the semaphore before giving up</param>
    /// <returns>
    ///   True if the semaphore let the thread through and was decremented,
    ///   false if the timeout elapsed and the semaphore was not decremented.
    /// <returns>
    /// <remarks>
    ///   This causes the calling thread to block if the semaphore didn't already have
    ///   a positive count. If the thread is blocked, it will stay so until another
    ///   thread calls <see cref="Post" /> on the semaphore or until the specified
    ///   patience time has elapsed.
    /// </remarks>
    public: NUCLEX_SUPPORT_API bool WaitForThenDecrement(
      const std::chrono::microseconds &patience
    );

    //public: void WaitUntilThenDecrement(const std::chrono::time_point< &patience);

    /// <summary>Structure to hold platform dependent process and file handles</summary>
    private: struct PlatformDependentImplementationData;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: PlatformDependentImplementationData &getImplementationData();
#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)
    alignas(8) unsigned char implementationDataBuffer[sizeof(std::size_t) * 2];
#else // Posix
    unsigned char implementationDataBuffer[96];
#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
