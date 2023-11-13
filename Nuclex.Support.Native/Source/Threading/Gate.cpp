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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Threading/Gate.h"

#if defined(NUCLEX_SUPPORT_LINUX) // Directly use futex via kernel syscalls
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include "../Platform/LinuxFutexApi.h" // for LinuxFutexApi::PrivateFutexWait() and more
#elif defined(NUCLEX_SUPPORT_WINDOWS) // Use standard win32 threading primitives
#include "../Platform/WindowsApi.h" // for ::CreateEventW(), ::CloseHandle() and more
#include "../Platform/WindowsSyncApi.h" // for ::WaitOnAddress(), ::WakeByAddressAll()
#else // Posix: use a pthreads conditional variable to emulate a gate
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include <ctime> // for ::clock_gettime()
#endif

#include <atomic> // for std::atomic

#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS)
  // Just some safety checks to make sure pthread_condattr_setclock() is available.
  // https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
  //
  // You shouldn't encounter any Linux system where the Posix implementation isn't set
  // to Posix 2008-09 or something newer by default. If you do, you can set _POSIX_C_SOURCE
  // in your build system or remove the Gate implementation from your library.
  #if defined(_POSIX_C_SOURCE)
    #if (_POSIX_C_SOURCE < 200112L)
      #error Your C runtime library needs to at least implement Posix 2001-12
    #endif
    //#if !defined(__USE_XOPEN2K)
  #endif
#endif

#include <cassert> // for assert()

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Gate::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the gate</summary>
    /// <param name="initiallyOpen">Whether the gate is initially open</param>
    public: PlatformDependentImplementationData(bool initiallyOpen);

    /// <summary>Frees all resources owned by the gate</summary>
    public: ~PlatformDependentImplementationData();

#if defined(NUCLEX_SUPPORT_LINUX)
    /// <summary>Stores the current state of the futex</summary>
    public: volatile std::uint32_t FutexWord;
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    /// <summary>Stores the current state of the wait varable</summary>
    public: volatile std::uint32_t WaitWord;
#else // Posix
    /// <summary>Whether the gate is currently open</summary>
    public: std::atomic<bool> IsOpen;
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;
#endif

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  Gate::PlatformDependentImplementationData::PlatformDependentImplementationData(
    bool initiallyOpen
  ) :
    FutexWord(initiallyOpen ? 1 : 0) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  Gate::PlatformDependentImplementationData::PlatformDependentImplementationData(
    bool initiallyOpen
  ) :
    WaitWord(initiallyOpen ? 1 : 0) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Gate::PlatformDependentImplementationData::PlatformDependentImplementationData(
    bool initiallyOpen
  ) :
    IsOpen(initiallyOpen),
    Condition() {

    // Attribute necessary to use CLOCK_MONOTONIC for condition variable timeouts
    ::pthread_condattr_t *monotonicClockAttribute = (
      Platform::PosixTimeApi::GetMonotonicClockAttribute()
    );

    // Create a new pthread conditional variable
    int result = ::pthread_cond_init(&this->Condition, monotonicClockAttribute);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread conditional variable", result
      );
    }

    result = ::pthread_mutex_init(&this->Mutex, nullptr);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX) || defined(NUCLEX_SUPPORT_WINDOWS)
  Gate::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    // Nothing to do. If threads are waiting, they're now waiting on dead memory.
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Gate::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::pthread_mutex_destroy(&this->Mutex);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread mutex is detroyed successfully");

    result = ::pthread_cond_destroy(&this->Condition);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable is detroyed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Gate::Gate(bool initiallyOpen) :
    implementationData() {

    // If this assert hits, the buffer size assumed by the header was too small.
    // There will be a buffer overflow in the line after and the application will
    // likely crash or at least malfunction.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Gate fits in buffer"
    );
    new(this->implementationDataBuffer) PlatformDependentImplementationData(initiallyOpen);
  }

  // ------------------------------------------------------------------------------------------- //

  Gate::~Gate() {
    getImplementationData().~PlatformDependentImplementationData();
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Gate::Open() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Simply set the atomic variable to 1 to indicate the gate is open
    __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE);

    // Futex Wake (Linux 2.6.0+)
    // https://man7.org/linux/man-pages/man2/futex.2.html
    //
    // This will signal other threads sitting in the Gate::Wait() method to
    // re-check the gate's status and resume running
    Platform::LinuxFutexApi::PrivateFutexWakeAll(impl.FutexWord);
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Gate::Open() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Simply set the atomic variable to 1 to indicate the gate is open
    impl.WaitWord = 1; // std::atomic_store(...);
    std::atomic_thread_fence(std::memory_order::memory_order_release);

    // WakeByAddressAll() (Windows 8+)
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddressall
    //
    // This will signal other threads sitting in the Latch::Wait() method to re-check
    // the gate's state and resume running
    //
    Platform::WindowsSyncApi::WakeByAddressAll(impl.WaitWord);
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Gate::Open() {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    impl.IsOpen.store(true, std::memory_order_relaxed);
    result = ::pthread_cond_signal(&impl.Condition);
    if(unlikely(result != 0)) {
      int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
      assert((unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler");
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not signal pthread conditional variable", result
      );
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Gate::Close() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // This is a GCC intrinsic, also supported by clang. If it doesn't work,
    // you can also just assign since the variable is volatile.
    __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE);
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Gate::Close() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Simply set the atomic variable to 1 to indicate the gate is open
    impl.WaitWord = 0; // std::atomic_store(...);
    std::atomic_thread_fence(std::memory_order::memory_order_release);
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Gate::Close() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // We don't need memory_order_release, but the caller is likely to expect a fence
    impl.IsOpen.store(false, std::memory_order_release);
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  void Gate::Set(bool opened) {
    if(opened) {
      Open();
    } else {
      Close();
    }
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Gate::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Do a single check for whether the gate is currently open. This is not
    // a race condition because the futex syscall will do the check again atomically,
    // but checking once in userspace may allow us to avoid the syscall().
    std::uint32_t safeFutexWord = __atomic_load_n(&impl.FutexWord, __ATOMIC_CONSUME);
    if(safeFutexWord != 0) {
      return; // Gate was open
    }

    // Be ready to check multiple times in case of EINTR
    for(;;) {

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::LinuxFutexApi::WaitResult result = Platform::LinuxFutexApi::PrivateFutexWait(
        impl.FutexWord,
        0 // wait while futex word is 0 (== gate closed)
      );
      if(likely(result != Platform::LinuxFutexApi::Interrupted)) {
        safeFutexWord = __atomic_load_n(&impl.FutexWord, __ATOMIC_CONSUME);
        if(likely(safeFutexWord != 0)) {
          return; // Gate now open
        }
      }

    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Gate::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Do a single check for whether the gate is currently open. This is not
    // a race condition because WaitOnAddress() call will do the check again atomically,
    // but checking once in userspace may allow us to avoid the kernel mode call.
    std::uint32_t safeWaitValue = impl.WaitWord; // std::atomic_load<std::uint32_t>(...);
    if(safeWaitValue != 0) {
      return; // Gate was open
    }

    // Be ready to check multiple times in case of spurious wakeups
    for(;;) {

      // WaitOnAddress (Windows 8+)
      // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
      //
      // This sends the thread to sleep for as long as the wait value has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::WindowsSyncApi::WaitResult result = Platform::WindowsSyncApi::WaitOnAddress(
        static_cast<const volatile std::uint32_t &>(impl.WaitWord),
        static_cast<std::uint32_t>(0) // wait while wait variable is 0 (== gate closed)
      );
      if(likely(result == Platform::WindowsSyncApi::WaitResult::ValueChanged)) {
        safeWaitValue = impl.WaitWord; // std::atomic_load(...);
        if(likely(safeWaitValue != 0)) {
          return; // Value was not 0, so gate is now open
        }
      }

    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Gate::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(!impl.IsOpen.load(std::memory_order_consume)) {
      result = ::pthread_cond_wait(&impl.Condition, &impl.Mutex);
      if(unlikely(result != 0)) {
        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert(
          (unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler"
        );
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wait on pthread conditional variable", result
        );
      }
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  bool Gate::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Do a single check for whether the gate is currently open. This is not
    // a race condition because the futex syscall will do the check again atomically,
    // but checking once in userspace may allow us to avoid the syscall().
    std::uint32_t safeFutexWord = __atomic_load_n(&impl.FutexWord, __ATOMIC_CONSUME);
    if(safeFutexWord != 0) {
      return true; // Gate was open
    }

    // Query the time, but don't do anything with it yet (the futex wait is
    // relative, so unless we get EINTR, the time isn't even needed)
    struct ::timespec startTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &startTime);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get monotonic time for gate", errorNumber
      );
    }

    // Note that the timeout is a relative one
    //
    // From the docs:
    //   | Note: for FUTEX_WAIT, timeout is interpreted as a relative
    //   | value.  This differs from other futex operations, where
    //   | timeout is interpreted as an absolute value.
    struct ::timespec timeout;
    {
      const std::size_t NanoSecondsPerMicrosecond = 1000; // 1,000 ns = 1 μs

      // timespec has seconds and nanoseconds, so divide the microseconds into full seconds
      // and remainder milliseconds to deal with this
      ::ldiv_t divisionResults = ::ldiv(patience.count(), 1000000);
      timeout.tv_sec = divisionResults.quot;
      timeout.tv_nsec = divisionResults.rem * NanoSecondsPerMicrosecond;
    }

    // Check the futex word and wait on it until it changes. Normally, this loops exactly
    // once, but EINTR may still happen and require us to recalculate the relative timeout.
    for(;;) {

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::LinuxFutexApi::WaitResult result = Platform::LinuxFutexApi::PrivateFutexWait(
        impl.FutexWord,
        0, // wait while futex word is 0 (== gate closed)
        timeout // timeout after which to fail
      );
      if(likely(result == Platform::LinuxFutexApi::WaitResult::ValueChanged)) {
        safeFutexWord = __atomic_load_n(&impl.FutexWord, __ATOMIC_CONSUME);
        if(safeFutexWord != 0) {
          return true; // Gate now open
        }
      } else if(unlikely(result == Platform::LinuxFutexApi::WaitResult::TimedOut)) {
        return false; // Patience has been exceeded
      }

      // Calculate the new relative timeout. If this is some kind of spurious
      // wake-up, but the value does indeed change while we're here, that's not
      // a problem since the futex syscall will re-check the futex word.
      timeout = Platform::PosixTimeApi::GetRemainingTimeout(CLOCK_MONOTONIC, startTime, patience);

    }

    return true; // wait noticed a change to the futex word
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  bool Gate::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Do a single check for whether the gate is currently open. This is not
    // a race condition because WaitOnAddress() will do the check again atomically,
    // but checking once in userspace may allow us to avoid the kernel mode call.
    std::uint32_t safeWaitValue = impl.WaitWord; // std::atomic_load<std::uint32_t>(...);
    if(safeWaitValue != 0) {
      return true; // Gate was open
    }

    // Query the tick counter, but don't do anything with it yet (the wait time is
    // relative, so unless we get a spurious wait, the tick counter isn't even needed)
    std::chrono::milliseconds startTickCount(::GetTickCount64());
    std::chrono::milliseconds patienceTickCount = (
      std::chrono::duration_cast<std::chrono::milliseconds>(patience)
    );
    std::chrono::milliseconds remainingTickCount = patienceTickCount;

    // Check the wait value and wait on it until it changes. Normally, this loops exactly
    // once, but spurious wake-ups may still happen and require us to wait again
    for(;;) {

      // WaitOnAddress (Windows 8+)
      // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress
      //
      // This sends the thread to sleep for as long as the wait value has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::WindowsSyncApi::WaitResult result = Platform::WindowsSyncApi::WaitOnAddress(
        static_cast<const volatile std::uint32_t &>(impl.WaitWord),
        static_cast<std::uint32_t>(0), // wait while wait variable is 0 (== gate closed)
        remainingTickCount
      );
      if(likely(result != Platform::WindowsSyncApi::WaitResult::TimedOut)) {
        safeWaitValue = impl.WaitWord; // std::atomic_load(...);
        if(likely(safeWaitValue != 0)) { // Value was not 0, so gate is now open
          break;
        }
      }

      // Calculate the new relative timeout. If this is some kind of spurious
      // wake-up, but the value does indeed change while we're here, that's not
      // a problem since the WaitOnAddress() call will re-check the wait value.
      {
        std::chrono::milliseconds elapsedTickCount = (
          std::chrono::milliseconds(::GetTickCount64()) - startTickCount
        );
        if(elapsedTickCount >= patienceTickCount) {
          return false; // timeout expired
        } else {
          remainingTickCount = patienceTickCount - elapsedTickCount;
        }
      }

    }

    return true; // wait noticed a change to the wait variable and gate was open
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  bool Gate::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    struct ::timespec waitEndTime = Platform::PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, patience);

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthreads mutex", result
      );
    }

    while(!impl.IsOpen.load(std::memory_order_consume)) {
      result = ::pthread_cond_timedwait(&impl.Condition, &impl.Mutex, &waitEndTime);
      if(unlikely(result != 0)) {
        if(result == ETIMEDOUT) {
          result = ::pthread_mutex_unlock(&impl.Mutex);
          if(unlikely(result != 0)) {
            Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
              u8"Could not unlock pthreads mutex", result
            );
          }
          return false;
        }

        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert(
          (unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler"
        );
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wait on pthreads conditional variable", result
        );
      }
    }

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthreads mutex", result
      );
    }
    return true;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  const Gate::PlatformDependentImplementationData &Gate::getImplementationData() const {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<const PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Gate::PlatformDependentImplementationData &Gate::getImplementationData() {
    constexpr bool implementationDataFitsInBuffer = (
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData))
    );
    if constexpr(implementationDataFitsInBuffer) {
      return *reinterpret_cast<PlatformDependentImplementationData *>(
        this->implementationDataBuffer
      );
    } else {
      return *this->implementationData;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
