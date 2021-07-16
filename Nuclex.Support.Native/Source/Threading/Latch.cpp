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

#include "Nuclex/Support/Threading/Latch.h"

#if defined(NUCLEX_SUPPORT_LINUX) // Directly use futex via kernel syscalls
#include "../Platform/PosixTimeApi.h" // for PosixTimeApi::GetRemainingTimeout()
#include <linux/futex.h> // for futex constants
#include <unistd.h> // for ::syscall()
#include <limits.h> // for INT_MAX
#include <sys/syscall.h> // for ::SYS_futex
#include <ctime> // for ::clock_gettime()
#elif defined(NUCLEX_SUPPORT_WINDOWS) // Use standard win32 threading primitives
#include "../Platform/WindowsApi.h" // for ::CreateEventW(), ::CloseHandle() and more
#include <mutex> // for std::mutex
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
  // in your build system or remove the Latch implementation from your library.
  #if defined(_POSIX_C_SOURCE)
    #if (_POSIX_C_SOURCE < 200112L)
      #error Your C runtime library needs to at least implement Posix 2001-12 
    #endif
    //#if !defined(__USE_XOPEN2K)
  #endif
#endif

namespace Nuclex { namespace Support { namespace Threading {

  // ------------------------------------------------------------------------------------------- //

  // Implementation details only known on the library-internal side
  struct Latch::PlatformDependentImplementationData {

    /// <summary>Initializes a platform dependent data members of the latch</summary>
    /// <param name="initialCount">Initial admit count for the latch</param>
    public: PlatformDependentImplementationData(std::size_t initialCount);

    /// <summary>Frees all resources owned by the Latch</summary>
    public: ~PlatformDependentImplementationData();

#if defined(NUCLEX_SUPPORT_LINUX)
    /// <summary>Switches between 0 (no waiters) and 1 (has waiters)</summary>
    public: mutable volatile std::uint32_t FutexWord;
    /// <summary>Counter, unlocks the latch when it reaches zero</summary>
    public: std::atomic<std::size_t> Countdown; 
#elif defined(NUCLEX_SUPPORT_WINDOWS)
    /// <summary>Countdown until the latch will open</summary>
    public: std::atomic<std::size_t> Countdown;
    /// <summary>Gate that lets threads through if the countdown is zero</summary>
    public: ::HANDLE EventHandle;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: std::mutex Mutex;
#else // Posix
    /// <summary>How many tasks the latch is waiting on</summary>
    public: std::atomic<std::size_t> Countdown; 
    /// <summary>Conditional variable used to signal waiting threads</summary>
    public: mutable ::pthread_cond_t Condition;
    /// <summary>Mutex required to ensure threads never miss the signal</summary>
    public: mutable ::pthread_mutex_t Mutex;
#endif

  };

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  Latch::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    FutexWord((initialCount > 0) ? 0 : 1),
    Countdown(initialCount) {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  Latch::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    Countdown(initialCount),
    EventHandle(INVALID_HANDLE_VALUE),
    Mutex() {
  
    // Create the Win32 event we'll use for this
    this->EventHandle = ::CreateEventW(
      nullptr, TRUE, (initialCount > 0) ? FALSE : TRUE, nullptr
    );
    bool eventCreationFailed = (
      (this->EventHandle == 0) || (this->EventHandle == INVALID_HANDLE_VALUE)
    );
    if(unlikely(eventCreationFailed)) {
      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not create thread synchronication event for countdown latch", lastErrorCode
      );
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Latch::PlatformDependentImplementationData::PlatformDependentImplementationData(
    std::size_t initialCount
  ) :
    Countdown(initialCount),
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
#if defined(NUCLEX_SUPPORT_LINUX)
  Latch::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    // Nothing to do. If threads are waiting, they're now waiting on dead memory.
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  Latch::PlatformDependentImplementationData::~PlatformDependentImplementationData() {}
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  Latch::PlatformDependentImplementationData::~PlatformDependentImplementationData() {
    int result = ::pthread_mutex_destroy(&this->Mutex);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread mutex is detroyed successfully");

    result = ::pthread_cond_destroy(&this->Condition);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable is detroyed successfully");
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  Latch::Latch(std::size_t initialCount) :
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

  Latch::~Latch() {
    getImplementationData().~PlatformDependentImplementationData();
  }

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_LINUX)
  void Latch::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Increment the latch counter. This locks the latch.
    std::size_t previousCountdown = impl.Countdown.fetch_add(
      count, std::memory_order_release
    );

    // If the latch was open at the time of this call, we need to close it so threads
    // can wait on the futex.
    if(unlikely(previousCountdown == 0)) {

      // There's a race condition at this point. If at this exact point, another thread
      // calls CountDown() and sets the FutexWord to 1 because the counter hit zero,
      // we'd falsely change it back to 0.
      //
      // Then a third thread which saw the countdown being greater than zero might reach
      // the futex wait and actually begin waiting even though the latch should be open.
      //
      // To fix this, we re-check the latch counter after setting the futex word.
      //
      if(likely(count > 0)) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> latch now locked
      }

      // Re-check the latch counter. This might seem like a naive hack at first sight,
      // but by only doing this re-check in Post() and not in CountDown(), we can guarantee
      // that both methods exit by checking whether the latch should be open.
      //
      // This means we can have a spurious open latch (which is easy to deal with), but
      // never a spurious closed latch (which blocks the thread and can't be dealt with). 
      //
      previousCountdown = impl.Countdown.load(
        std::memory_order_consume
      );
      if(likely(previousCountdown == 0)) {
        __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> latch open
      }

    } // if(previousAdmitCounter < 0)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Latch::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    std::size_t previousCountdown = impl.Countdown.fetch_add(
      count, std::memory_order_consume // if() below carries dependency
    );
    if(unlikely(previousCountdown == 0)) {
      std::unique_lock mutexLock(impl.Mutex);

      previousCountdown = impl.Countdown.load(
        std::memory_order_relaxed // We're in a mutex now
      );
      if(previousCountdown > 0) {
        DWORD result = ::ResetEvent(impl.EventHandle);
        if(unlikely(result == FALSE)) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not reset synchronization event to closed state", lastErrorCode
          );
        }
      }
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Latch::Post(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    impl.Countdown.fetch_add(count, std::memory_order_release);

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
  void Latch::CountDown(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    // Decrement the latch counter and fetch its previous value so we can both
    // detect when the counter goes negative and open the latch when it reaches zero
    std::size_t previousCountdown = impl.Countdown.fetch_sub(
      count, std::memory_order_release
    );
    assert((previousCountdown >= count) && u8"Latch remains zero or positive");

    // If we just decremented the latch to zero, signal the futex
    if(unlikely(previousCountdown == count)) {

      // Just like in the semaphore implementation, another thread may have incremented
      // the latch counter between our fetch_sub() and this point (a classical race
      // condition).
      //
      // This isn't a problem, however, as changing the futex wakes up all blocked threads
      // and causes them to re-check the counter. So we'll have potential spurious wake-ups,
      // but no spurious blocks.
      //
      if(likely(count > 0)) { // check needed? nobody would post 0 tasks...
        __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> countdown is zero
      }

      // Futex Wake (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This will signal other threads sitting in the Latch::Wait() method to re-check
      // the latch counter and resume running
      //
      long result = ::syscall(
        SYS_futex, // syscall id
        static_cast<volatile std::uint32_t *>(&impl.FutexWord), // futex word being accessed
        static_cast<int>(FUTEX_WAKE_PRIVATE), // process-private futex wakeup
        static_cast<int>(INT_MAX), // wake up all waiting threads
        static_cast<struct ::timespec *>(nullptr), // timeout -> ignored
        static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
        static_cast<int>(0) // second futex word value -> ignored
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not wake up threads waiting on futex", errorNumber
        );
      }

    } // if latch counter decremented to zero
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Latch::CountDown(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    std::size_t previousCountdown = impl.Countdown.fetch_sub(
      count, std::memory_order_consume // if() below carries dependency
    );
    assert((previousCountdown >= count) && u8"Latch remains zero or positive");

    if(unlikely(previousCountdown == count)) {
      std::unique_lock mutexLock(impl.Mutex);

      previousCountdown = impl.Countdown.load(
        std::memory_order_relaxed // We're in a mutex now
      );
      if(previousCountdown == 0) {
        DWORD result = ::SetEvent(impl.EventHandle);
        if(unlikely(result == FALSE)) {
          DWORD lastErrorCode = ::GetLastError();
          Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
            u8"Could not set synchronization event to signalled state", lastErrorCode
          );
        }
      }
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Latch::CountDown(std::size_t count /* = 1 */) {
    PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    {
      std::size_t previousCountdown = impl.Countdown.fetch_sub(
        count, std::memory_order_release
      );
      assert((previousCountdown >= count) && u8"Latch remains zero or positive");

      if(previousCountdown == count) {
        result = ::pthread_cond_signal(&impl.Condition);
        if(unlikely(result != 0)) {
          int unlockResult = ::pthread_mutex_unlock(&impl.Mutex);
          NUCLEX_SUPPORT_NDEBUG_UNUSED(unlockResult);
          assert((unlockResult == 0) && u8"pthread mutex is successfully unlocked in error handler");
          Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
            u8"Could not signal pthread conditional variable", result
          );
        }
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
  void Latch::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Loop until we can snatch an available ticket
    std::size_t safeCountdown = impl.Countdown.load(std::memory_order_consume);
    for(;;) {
      if(safeCountdown == 0) {
        return;
      }

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      long result = ::syscall(
        SYS_futex, // syscall id
        static_cast<const volatile std::uint32_t *>(&impl.FutexWord), // futex word being accessed
        static_cast<int>(FUTEX_WAIT_PRIVATE), // process-private futex wakeup
        static_cast<int>(0), // wait while futex word is 0 (== latch counter is greater than zero)
        static_cast<struct ::timespec *>(nullptr), // timeout -> infinite
        static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
        static_cast<int>(0) // second futex word value -> ignored
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(unlikely((errorNumber != EAGAIN) && (errorNumber != EINTR))) {
          Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
            u8"Could not sleep on latch via futex wait", errorNumber
          );
        }
      }

      // If this was a spurious wake-up, we need to adjust the futex word in order to prevent
      // a busy loop in this Wait() method.
      safeCountdown = impl.Countdown.load(std::memory_order_consume);
      if(safeCountdown > 0) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> latch now locked

        // But just like in Post(), this is a race condition with other threads potentially
        // calling CountDown(), so to err on the side of having spurious open latches, we
        // need to re-check the latch counter.
        //
        safeCountdown = impl.Countdown.load(std::memory_order_consume);
        if(safeCountdown == 0) {
          __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> latch open
        }
      }
    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  void Latch::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Check if the countdown is currently zero
    std::size_t safeCountdown = impl.Countdown.load(
      std::memory_order_consume // if() below carries dependency
    );
    if(safeCountdown > 0) {
      DWORD result = ::WaitForSingleObject(impl.EventHandle, INFINITE);
      if(likely(result != WAIT_OBJECT_0)) {
        DWORD lastErrorCode = ::GetLastError();
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Error waiting for semaphore via WaitForSingleObject()", lastErrorCode
        );
      }
    }
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  void Latch::Wait() const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.Countdown.load(std::memory_order_consume) > 0) {
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
  bool Latch::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

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

    // Loop until we can snatch an available ticket
    std::size_t safeCountdown = impl.Countdown.load(std::memory_order_consume);
    for(;;) {
      if(safeCountdown == 0) {
        return true;
      }

      // Calculate the remaining timeout until the wait should fail. Note that this is
      // a relative timeout (in contrast to ::sem_t and most things Posix).
      struct ::timespec timeout = Platform::PosixTimeApi::GetRemainingTimeout(
        CLOCK_MONOTONIC, startTime, patience
      );

      // Futex Wait (Linux 2.6.0+)
      // https://man7.org/linux/man-pages/man2/futex.2.html
      //
      // This sends the thread to sleep for as long as the futex word has the expected value.
      // Checking and entering sleep is one atomic operation, avoiding a race condition.
      long result = ::syscall(
        SYS_futex, // syscall id
        static_cast<const volatile std::uint32_t *>(&impl.FutexWord), // futex word being accessed
        static_cast<int>(FUTEX_WAIT_PRIVATE), // process-private futex wakeup
        static_cast<int>(0), // wait while futex word is 0 (== latch counter is greater than zero)
        static_cast<struct ::timespec *>(&timeout), // timeout after which to fail
        static_cast<std::uint32_t *>(nullptr), // second futex word -> ignored
        static_cast<int>(0) // second futex word value -> ignored
      );
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        if(unlikely(errorNumber == ETIMEDOUT)) {
          return false; // Timeout elapsed, so it's time to give the bad news to the caller
        }
        if(unlikely((errorNumber != EAGAIN) && (errorNumber != EINTR))) {
          Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
            u8"Could not sleep on latch via futex wait", errorNumber
          );
        }
      }

      // If this was a spurious wake-up, we need to adjust the futex word in order to prevent
      // a busy loop in this Wait() method.
      safeCountdown = impl.Countdown.load(std::memory_order_consume);
      if(likely(safeCountdown > 0)) {
        __atomic_store_n(&impl.FutexWord, 0, __ATOMIC_RELEASE); // 0 -> latch now locked

        // But just like in Post(), this is a race condition with other threads potentially
        // calling CountDown(), so to err on the side of having spurious open latches, we
        // need to re-check the latch counter.
        //
        safeCountdown = impl.Countdown.load(std::memory_order_consume);
        if(unlikely(safeCountdown == 0)) {
          __atomic_store_n(&impl.FutexWord, 1, __ATOMIC_RELEASE); // 1 -> latch open
        }
      }
    } // for(;;)
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_SUPPORT_WINDOWS)
  bool Latch::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Check if the countdown is currently zero
    std::size_t safeCountdown = impl.Countdown.load(
      std::memory_order_consume // if() below carries dependency
    );
    if(safeCountdown > 0) {
      DWORD milliseconds = static_cast<DWORD>((patience.count() + 500) / 1000);
      DWORD result = ::WaitForSingleObject(impl.EventHandle, milliseconds);
      if(likely(result == WAIT_OBJECT_0)) {
        return true;
      } else if(result == WAIT_TIMEOUT) {
        return false;
      }

      DWORD lastErrorCode = ::GetLastError();
      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
        u8"Error waiting for latch via WaitForSingleObject()", lastErrorCode
      );
    }

    return true;
  }
#endif
  // ------------------------------------------------------------------------------------------- //
#if !defined(NUCLEX_SUPPORT_LINUX) && !defined(NUCLEX_SUPPORT_WINDOWS) // -> Posix
  bool Latch::WaitFor(const std::chrono::microseconds &patience) const {
    const PlatformDependentImplementationData &impl = getImplementationData();

    // Forced to use CLOCK_REALTIME, which means the semaphore is broken :-(
    struct ::timespec endTime = Platform::PosixTimeApi::GetTimePlus(CLOCK_MONOTONIC, patience);

    int result = ::pthread_mutex_lock(&impl.Mutex);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not lock pthread mutex", result
      );
    }

    while(impl.Countdown.load(std::memory_order_consume) > 0) {
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

  const Latch::PlatformDependentImplementationData &Latch::getImplementationData() const {
    return *reinterpret_cast<const PlatformDependentImplementationData *>(
      this->implementationDataBuffer
    );
  }

  // ------------------------------------------------------------------------------------------- //

  Latch::PlatformDependentImplementationData &Latch::getImplementationData() {
    return *reinterpret_cast<PlatformDependentImplementationData *>(
      this->implementationDataBuffer
    );
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Threading
