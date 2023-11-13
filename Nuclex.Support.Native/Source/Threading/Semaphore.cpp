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

#include "Nuclex/Support/Threading/Semaphore.h"

#if defined(NUCLEX_SUPPORT_LINUX) // Directly use futex via kernel syscalls
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetRemainingTimeout()
#include "../Platform/LinuxFutexApi.h" // for LinuxFutexApi::PrivateFutexWait() and more
#include <atomic> // for std::atomic
#elif defined(NUCLEX_SUPPORT_WINDOWS) // Use standard win32 threading primitives
#include "../Platform/WindowsApi.h" // for ::CreateEventW(), ::CloseHandle() and more
#include "../Platform/WindowsSyncApi.h" // for ::WaitOnAddress(), ::WakeByAddressAll()
#else // Posix: use a pthreads conditional variable to emulate a semaphore
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetTimePlus()
#include <ctime> // for ::clock_gettime()
#endif

#include <atomic> // for std::atomic
#include <cassert> // for assert()

#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS)
  // Just some safety checks to make sure pthread_condattr_setclock() is available.
  // https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
  //
  // You shouldn't encounter any Linux system where the Posix implementation isn't set
  // to Posix 2008-09 or something newer by default. If you do, you can set _POSIX_C_SOURCE
  // in your build system or remove the Semaphore implementation from your library.
  #if defined(_POSIX_C_SOURCE)
    #if (_POSIX_C_SOURCE < 200112L)
      #error Your C runtime library needs to at least implement Posix 2001-12
    #endif
    //#if !defined(__USE_XOPEN2K)
  #endif
#endif

// The situation on Linux/Posix systems is a bit depressing here:
//
// With ::sem_t, a native semaphore exists, but it always uses CLOCK_REALTIME which is
// prone to jumps (i.e. run ntp-client and it can easily jump seconds or minutes).
//
// There's a Bugzilla ticket for the kernel which hasn't changed status in 5 years:
// https://bugzilla.kernel.org/show_bug.cgi?id=112521
//
// And there's ::sem_timedwait_monotonic() on QNX, but not Posix:
// http://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/s/sem_timedwait.html
//
// On the other hand, Linux 2.6.28 makes its futexes use CLOCK_MONOTONIC by default
// https://man7.org/linux/man-pages/man2/futex.2.html
//
// Relying on ::sem_t is problematic. It works for a cron-style application where
// the wait is actually aiming for a time on the wall clock, but it's useless for
// normal thread synchronization where 50 ms may unexpectedly become 5 minutes or
// report ETIMEOUT after less than 1 ms.
//

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Semaphore::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the semaphore</summary>
    /// <param name="initialCount">Initial admit count for the semaphore</param>
    public: PlatformDependentImplementationData(std::size_t initialCount);

    /// <summary>Frees all resources owned by the Semaphore</summary>
    public: ~PlatformDependentImplementationData();

#if defined(NUCLEX_SUPPORT_LINUX)
    /// <summary>Switches between 0 (no waiters) and 1 (has waiters)</summary>
    public: volatile std::uint32_t FutexWord;
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    /// <summary>Switches between 0 (no waiters) and 1 (has waiters)</summary>
    public: volatile std::uint32_t WaitWord;
#else // Posix
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;
#endif
    /// <summary>Available tickets, negative for each thread waiting for a ticket</summary>
    public: std::atomic<std::size_t> AdmitCounter;

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    FutexWord(0),
    AdmitCounter(initialCount) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    WaitWord(0),
    AdmitCounter(initialCount) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Semaphore::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    AdmitCounter(initialCount),
    Condition(),
    Mutex() {

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
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    // Nothing to do. If threads are waiting, they're now waiting on dead memory.
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Semaphore::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::pthread_mutex_destroy(&this->Mutex);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread mutex is detroyed successfully");

    result = ::pthread_cond_destroy(&this->Condition);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable is detroyed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Semaphore::Semaphore(std::size_t initialCount) :
    implementationDataBuffer() {

    // If this assert hits, the buffer size assumed by the header was too small.
    // There will be a buffer overflow in the line after and the application will
    // likely crash or at least malfunction.
    assert(
      (sizeof(this->implementationDataBuffer) >= sizeof(PlatformDependentImplementationData)) &&
      u8"Private implementation data for Nuclex::Support::Threading::Process fits in buffer"
    );
    new(this->implementationDataBuffer) PlatformDependentImplementationData(initialCount);
  }

  // ------------------------------------------------------------------------------------------- //

  Semaphore::~Semaphore() {
    getImplementationData().~PlatformDependentImplementationData();
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Increment the semaphore admit counter so for each posted ticket,
    // a thread will be able to pass through the semaphore.
    std::size_t previousAdmitCounter = impl.AdmitCounter.fetch_add(
      count, std::memory_order_release // CHECK: Should this be consume?
    );

    // If there were no admits left at the time of this call, then there
    // may be waiting threads that need to be woken.
    if(previousAdmitCounter == 0) { // If there was no work available before

      // Now here's a little race condition:
      // - Some thread may have checked the admit counter before our increment
      //   (and found it was 0, so plans to go to sleep)
      // - Now we increment the admit counter and try to wake threads
      //   (but none are waiting)
      // - Finally, the earlier thread reaches the futex call and waits.
      //   (even though there's work available and the waking is already done)
      //
      // That's why our futex word is 1 if there's work available. Changing it
      // will wake *all* threads, and that sucks, so we take care to only toggle
      // it if the situation actually changes.
      //
      if(count > 0) { // check needed? nobody would post 0 tickets...
        __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> tickets available
      }

      // Futex Wake (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This will signal other threads sitting in the Semaphore::WaitAndDecrement() method to
      // re-check the semaphore's status and resume running
      //
      Platform::LinuxFutexApi::PrivateFutexWakeAll(impl.FutexWord);

    } // if(previousAdmitCounter < 0)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Increment the semaphore admit counter so for each posted ticket,
    // a thread will be able to pass through the semaphore.
    std::size_t previousAdmitCounter = impl.AdmitCounter.fetch_add(
      count, std::memory_order_release // CHECK: Should this be consume?
    );

    // If there were no admits left at the time of this call, then there
    // may be waiting threads that need to be woken.
    if(previousAdmitCounter == 0) { // If there was no work available before

      // Now here's a little race condition:
      // - Some thread may have checked the admit counter before our increment
      //   (and found it was 0, so plans to go to sleep)
      // - Now we increment the admit counter and try to wake threads
      //   (but none are waiting)
      // - Finally, the earlier thread reaches the futex call and waits.
      //   (even though there's work available and the waking is already done)
      //
      // That's why our futex word is 1 if there's work available. Changing it
      // will wake *all* threads, and that sucks, so we take care to only toggle
      // it if the situation actually changes.
      //
      if(count > 0) { // check needed? nobody would post 0 tickets...
        impl.WaitWord = 1; // 1 -> tickets available
        std::atomic_thread_fence(std::memory_order::memory_order_release);
      }

      // WakeByAddressAll() (Windows 8+)
      // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakebyaddressall
      //
      // This will signal other threads sitting in the Latch::Wait() method to re-check
      // the latch counter and resume running
      //
      Platform::WindowsSyncApi::WakeByAddressAll(impl.WaitWord);

    } // if(previousAdmitCounter < 0)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Semaphore::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    impl.AdmitCounter.fetch_add(count, std::memory_order_release);

    while(count > 0) {
      result = ::pthread_cond_signal(&impl.Condition);
      if(unlikely(result != 0)) {
        int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
        NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
        assert((unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler");
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not signal pthread conditional variable", result
        );
      }

      --count;
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
  void Semaphore::WaitThenDecrement() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Loop until we can snatch an available ticket
    std::size_t initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
    for(;;) {

      // Load the ticket counter. If there are tickets available, try to snatch
      // one ticket and, if obtained, return control to the caller. Should no
      // tickets be available (or they got used up while we were trying to snatch
      // one), we will attempt to sleep on the futex word.
      std::size_t safeAdmitCounter = initialAdmitCounter;
      while(safeAdmitCounter > 0) {
        bool success = impl.AdmitCounter.compare_exchange_weak(
          safeAdmitCounter, safeAdmitCounter - 1, std::memory_order_release
        );
        if(success) {
          return; // We snatched a ticket!
        }
      }

      // If we observed some other thread snatching the last ticket and need to go
      // to sleep, switch the futex word to the contested state.
      //
      // At this point, we're in a race with the Post() method which may just now
      // have incremented the ticket counter and be trying to pre-empt us by
      // setting the futex word to 1 (meaning tickets are available).
      //
      // Thus we need to do some double-checking here.
      //
      if(initialAdmitCounter > 0) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> threads waiting
        initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
        if(unlikely(initialAdmitCounter > 0)) {
          __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> tickets available
          continue;
        }
      }

      // Now we're safe. The futex word has been set to 0 (threads are waiting) while
      // the admit ticket counter was zero, so if any work is posted between here and
      // our futex syscall, it's no problem since the syscall does atomically check
      // that the futex word is still 0 or otherwise return EAGAIN.

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::LinuxFutexApi::PrivateFutexWait(
        impl.FutexWord,
        0 // wait while futex word is 0 (== threads are waiting, no tickets)
      );

      // At this point the thread has woken up because of either
      // - a signal (EINTR)
      // - the futex word changed (EAGAIN)
      // - an explicit wake from the Post() method (result == 0)
      //
      // In all cases, we recheck the ticket counter and try to either obtain
      // a ticket or go back to sleep using the same method as before.
      initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);

    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Semaphore::WaitThenDecrement() {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Loop until we can snatch an available ticket
    std::size_t initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
    for(;;) {

      // Load the ticket counter. If there are tickets available, try to snatch
      // one ticket and, if obtained, return control to the caller. Should no
      // tickets be available (or they got used up while we were trying to snatch
      // one), we will attempt to sleep on the futex word.
      std::size_t safeAdmitCounter = initialAdmitCounter;
      while(safeAdmitCounter > 0) {
        bool success = impl.AdmitCounter.compare_exchange_weak(
          safeAdmitCounter, safeAdmitCounter - 1, std::memory_order_release
        );
        if(success) {
          return; // We snatched a ticket!
        }
      }

      // If we observed some other thread snatching the last ticket and need to go
      // to sleep, switch the futex word to the contested state.
      //
      // At this point, we're in a race with the Post() method which may just now
      // have incremented the ticket counter and be trying to pre-empt us by
      // setting the futex word to 1 (meaning tickets are available).
      //
      // Thus we need to do some double-checking here.
      //
      if(initialAdmitCounter > 0) {
        impl.WaitWord = 0; // 0 -> threads waiting
        std::atomic_thread_fence(std::memory_order::memory_order_release);

        initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
        if(unlikely(initialAdmitCounter > 0)) {
          impl.WaitWord = 1; // 1 -> tickets available
          std::atomic_thread_fence(std::memory_order::memory_order_release);
          continue;
        }
      }

      // Now we're safe. The futex word has been set to 0 (threads are waiting) while
      // the admit ticket counter was zero, so if any work is posted between here and
      // WaitOnAddress(), it's no problem since WaitOnAddress() does atomically check
      // that the wait value is still 0 or otherwise return.

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

        // At this point the thread has woken up because of either
        // - a signal (EINTR)
        // - the futex word changed (EAGAIN)
        // - an explicit wake from the Post() method
        //
        // In all cases, we recheck the ticket counter and try to either obtain
        // a ticket or go back to sleep using the same method as before.
        initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);

      }

    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Semaphore::WaitThenDecrement() {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.AdmitCounter.load(std::memory_order_consume) == 0) {
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

    impl.AdmitCounter.fetch_sub(1, std::memory_order_release);

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
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience)  {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Obtain the starting time, but don't do anything with it yet (the futex
    // wait is relative, so unless we get EINTR, the time isn't even needed)
    struct ::timespec startTime;
    int result = ::clock_gettime(CLOCK_MONOTONIC, &startTime);
    if(result == -1) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get monotonic time for gate", errorNumber
      );
    }

    // Loop until we can either snatch an available ticket or until
    // the caller-specified timeout is up
    std::size_t initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
    for(;;) {

      // Load the ticket counter. If there are tickets available, try to snatch
      // one ticket and, if obtained, return control to the caller. Should no
      // tickets be available (or they got used up while we were trying to snatch
      // one), we will attempt to sleep on the futex word.
      std::size_t safeAdmitCounter = initialAdmitCounter;
      while(safeAdmitCounter > 0) {
        bool success = impl.AdmitCounter.compare_exchange_weak(
          safeAdmitCounter, safeAdmitCounter - 1, std::memory_order_release
        );
        if(success) {
          return true; // We snatched a ticket!
        }
      }

      // If we observed some other thread snatching the last ticket and need to go
      // to sleep, switch the futex word to the contested state.
      //
      // At this point, we're in a race with the Post() method which may just now
      // have incremented the ticket counter and be trying to pre-empt us by
      // setting the futex word to 1 (meaning tickets are available).
      //
      // Thus we need to do some double-checking here.
      //
      if(initialAdmitCounter > 0) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> threads waiting
        initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
        if(unlikely(initialAdmitCounter > 0)) {
          __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> tickets available
          continue;
        }
      }

      // Now we're safe. The futex word has been set to 0 (threads are waiting) while
      // the admit ticket counter was zero, so if any work is posted between here and
      // our futex syscall, it's no problem since the syscall does atomically check
      // that the futex word is still 0 or otherwise return EAGAIN.

      // Calculate the remaining timeout until the wait should fail. Note that this is
      // a relative timeout (in contrast to ::sem_t and most things Posix).
      //
      // From the docs:
      //   | Note: for FUTEX_WAIT, timeout is interpreted as a relative
      //   | value.  This differs from other futex operations, where
      //   | timeout is interpreted as an absolute value.
      //
      // We memorized the clock time at the beginning of this method, so if we're
      // looping through this multiple times, the remaining timeout will keep
      // decreasing each time.
      struct ::timespec timeout = Platform::PosixTimeApi::GetRemainingTimeout(
        CLOCK_MONOTONIC, startTime, patience
      );

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      Platform::LinuxFutexApi::WaitResult result = Platform::LinuxFutexApi::PrivateFutexWait(
        impl.FutexWord,
        0, // wait while futex word is 0 (== threads are waiting, no tickets)
        timeout
      );
      if(unlikely(result == Platform::LinuxFutexApi::WaitResult::TimedOut)) {
        return false;
      }

      // At this point the thread has woken up because of either
      // - a signal (EINTR)
      // - the futex word changed (EAGAIN)
      // - an explicit wake from the Post() method
      //
      // In all cases, we recheck the ticket counter and try to either obtain
      // a ticket or go back to sleep using the same method as before.
      initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);

    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience)  {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Query the tick counter, but don't do anything with it yet (the wait time is
    // relative, so unless we get a spurious wait, the tick counter isn't even needed)
    std::chrono::milliseconds startTickCount(::GetTickCount64());
    std::chrono::milliseconds patienceTickCount = (
      std::chrono::duration_cast<std::chrono::milliseconds>(patience)
    );
    std::chrono::milliseconds remainingTickCount = patienceTickCount;

    // Loop until we can either snatch an available ticket or until
    // the caller-specified timeout is up
    std::size_t initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
    for(;;) {

      // Load the ticket counter. If there are tickets available, try to snatch
      // one ticket and, if obtained, return control to the caller. Should no
      // tickets be available (or they got used up while we were trying to snatch
      // one), we will attempt to sleep on the futex word.
      std::size_t safeAdmitCounter = initialAdmitCounter;
      while(safeAdmitCounter > 0) {
        bool success = impl.AdmitCounter.compare_exchange_weak(
          safeAdmitCounter, safeAdmitCounter - 1, std::memory_order_release
        );
        if(success) {
          return true; // We snatched a ticket!
        }
      }

      // If we observed some other thread snatching the last ticket and need to go
      // to sleep, switch the futex word to the contested state.
      //
      // At this point, we're in a race with the Post() method which may just now
      // have incremented the ticket counter and be trying to pre-empt us by
      // setting the futex word to 1 (meaning tickets are available).
      //
      // Thus we need to do some double-checking here.
      //
      if(initialAdmitCounter > 0) {
        impl.WaitWord = 0; // 0 -> threads waiting
        std::atomic_thread_fence(std::memory_order::memory_order_release);

        initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);
        if(unlikely(initialAdmitCounter > 0)) {
          impl.WaitWord = 1; // 1 -> tickets available
          std::atomic_thread_fence(std::memory_order::memory_order_release);
          continue;
        }
      }

      // Now we're safe. The wait value has been set to 0 (threads are waiting) while
      // the admit ticket counter was zero, so if any work is posted between here and
      // WaitOnAddres(), it's no problem since WaitOnAddress() does atomically check
      // that the wait value is still 0 or otherwise return.

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
      if(unlikely(result == Platform::WindowsSyncApi::WaitResult::TimedOut)) {
        return false;
      }

      // At this point the thread has woken up because of either
      // - a signal (EINTR)
      // - the futex word changed (EAGAIN)
      // - an explicit wake from the Post() method (result == 0)
      //
      // In all cases, we recheck the ticket counter and try to either obtain
      // a ticket or go back to sleep using the same method as before.
      initialAdmitCounter = impl.AdmitCounter.load(std::memory_order_consume);

    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  bool Semaphore::WaitForThenDecrement(const std::chrono::microseconds &patience) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Forced to use CLOCK_REALTIME, which means the semaphore is broken :-(
    struct ::timespec endTime = Platform::PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, patience);

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.AdmitCounter.load(std::memory_order_consume) == 0) {
      result = ::pthread_cond_timedwait(&impl.Condition, &impl.Mutex, &endTime);
      if(unlikely(result != 0)) {
        if(result == ETIMEDOUT) {
          result = ::pthread_mutex_unlock(&impl.Mutex);
          if(unlikely(result != 0)) {
            Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
              u8"Could not unlock pthread mutex", result
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
          u8"Could not wait on pthread conditional variable", result
        );
      }
    }

    impl.AdmitCounter.fetch_sub(1, std::memory_order_release);

    result = ::pthread_mutex_unlock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not unlock pthread mutex", result
      );
    }

    return true;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Semaphore::PlatformDependentImplementationData &Semaphore::getImplementationData() {
    return *reinterpret_cast<PlatformDependentImplementationData *>(
      this->implementationDataBuffer
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
