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

#ifndef NUCLEX_SUPPORT_THREADING_THREAD_H
#define NUCLEX_SUPPORT_THREADING_THREAD_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <cstdint> // for for std::uintptr_t
#include <chrono> // for std::chrono::microseconds etc.

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides supporting methods for threads</summary>
  class Thread {

    /// <summary>Lets the calling thread wait for the specified amount of time</summary>
    /// <param name="time">Duration for which the thread will wait</param>
    public: NUCLEX_SUPPORT_API static void Sleep(std::chrono::microseconds time);

    /// <summary>Determines whether the calling thread belongs to the thread pool</summary>
    /// <returns>True if the calling thread belongs to the thread pool</returns>
    public: NUCLEX_SUPPORT_API static bool BelongsToThreadPool();

#if defined(NUCLEX_SUPPORT_WANT_USELESS_THREAD_ID_QUERY)
    /// <summary>Returns a unique ID for the calling thread</summary>
    /// <returns>
    ///   A unique ID that no other thread that's running at the same time will have
    /// </returns>
    /// <remarks>
    ///   This is useful for some lock-free synchronization techniques.
    /// </remarks>
    public: NUCLEX_SUPPORT_API static std::uintptr_t GetCurrentThreadId();
#endif

    private: Thread(const Thread &) = delete;
    private: Thread&operator =(const Thread &) = delete;

  };


  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_THREAD_H
