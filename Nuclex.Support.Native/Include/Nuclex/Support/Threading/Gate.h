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

#ifndef NUCLEX_SUPPORT_THREADING_GATE_H
#define NUCLEX_SUPPORT_THREADING_GATE_H

#include "Nuclex/Support/Config.h"

#include <cstddef> // for std::size_t
#include <chrono> // for std::chrono::microseconds

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Lets threads through only if opened</summary>
  /// <remarks>
  ///   <para>
  ///     This is one of the simplest thread synchronization primitives. It will
  ///     simply block all threads while it is closed and let all threads through
  ///     while it is open.
  ///   </para>
  ///   <para>
  ///     It can be used in place of a reverse counting semaphore to wait for multiple
  ///     threads to complete their work or to launch multiple threads if you
  ///     intentionally want to construct a high-contention situation.
  ///   </para>
  ///   <para>
  ///     To Windows and .NET developers, it is known as a &quot;ManualResetEvent&quot;
  ///   </para>
  /// </remarks>
  class Gate {

    /// <summary>Initializes a new gate in an open or closed state</summary>
    /// <param name="initiallyOpen">Whether the gate is initially open</param>
    public: NUCLEX_SUPPORT_API Gate(bool initiallyOpen = false);

    /// <summary>Frees all resources owned by the gate</summary>
    /// <remarks>
    ///   There should not be any threads waiting on the gate when it is destroyed.
    ///   The behavior for such threats is undefined, they may hang forever, they
    ///   may receive an exception or the entire process may be terminated.
    /// </remarks>
    public: NUCLEX_SUPPORT_API ~Gate();

    /// <summary>Opens the gate, letting any currently and future threads through</summary>
    public: NUCLEX_SUPPORT_API void Open();

    /// <summary>Closes the gate, making any future threads wait in front of it</summary>
    public: NUCLEX_SUPPORT_API void Close();

    /// <summary>Sets the state of the gate to opened or closed</summary>
    /// <param name-"opened">Whether the gate will be opened (true) or closed (false)</param>
    public: NUCLEX_SUPPORT_API void Set(bool opened);

    /// <summary>
    ///   Waits for the gate to open. Returns immediately if it already is open.
    /// </summary>
    public: NUCLEX_SUPPORT_API void Wait() const;

    /// <summary>
    ///   Waits for the gate to open. Returns immediately if it already is open.
    /// </summary>
    /// <param name="patience">
    ///   How long to wait for the gate to open before giving up
    /// </param>
    /// <returns>True if the gate was opened, false if the patience time has elapsed</returns>
    public: NUCLEX_SUPPORT_API bool WaitFor(const std::chrono::microseconds &patience) const;

    //public: void WaitUntil(const std::chrono::time_point< &patience);

    /// <summary>Structure to hold platform dependent process and file handles</summary>
    private: struct PlatformDependentImplementationData;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: const PlatformDependentImplementationData &getImplementationData() const;
    /// <summary>Accesses the platform dependent implementation data container</summary>
    /// <returns>A reference to the platform dependent implementation data</returns>
    private: PlatformDependentImplementationData &getImplementationData();
    private: union alignas(8) {
      /// <summary>Platform dependent process and file handles used for the process</summary>
      PlatformDependentImplementationData *implementationData;
      /// <summary>Used to hold the platform dependent implementation data if it fits</summary>
      /// <remarks>
      ///   Small performance / memory fragmentation improvement.
      ///   This avoids a micro-allocation for the implenmentation data structure in most cases.
      /// </remarks>
#if defined(NUCLEX_SUPPORT_LINUX)
      unsigned char implementationDataBuffer[4];
#elif defined(NUCLEX_SUPPORT_WIN32)
      unsigned char implementationDataBuffer[sizeof(std::size_t)]; // matches HANDLE size
#else // Posix
      unsigned char implementationDataBuffer[96];
#endif
    };

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading

#endif // NUCLEX_SUPPORT_THREADING_SEMAPHORE_H
