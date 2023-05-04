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

#include "PosixTimeApi.h"

#if !defined(NUCLEX_SUPPORT_WINDOWS)

#include <pthread.h> // for ::pthread_condattr_...()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>A condition attribute that switches timeouts to the monotonic clock</summary>
  /// <remarks>
  ///   By default, timeouts run on the REALTIME clock (historic Posix, I guess), which
  ///   would risk all wait functions either skipping their wait or waiting for over an hour
  ///   when the system clock changes due to daylight savings time.
  /// </remarks>
  class MonotonicClockConditionAttribute {

    /// <summary>Initializes the monotonic clock attribute</summary>
    public: MonotonicClockConditionAttribute();

    /// <summary>Destroys the attribute</summary>
    public: ~MonotonicClockConditionAttribute();

    /// <summary>Accesses the conditional variable attribute</summary>
    /// <returns>The address of the attribute which can be passed to pthread functions</returns>
    public: ::pthread_condattr_t *GetAttribute() {
      return &this->attribute;
    }

    /// <summary>Conditional variable attributes managed by this instance</summary>
    private: ::pthread_condattr_t attribute;

  };

  // ------------------------------------------------------------------------------------------- //

  MonotonicClockConditionAttribute::~MonotonicClockConditionAttribute() {

    int result = ::pthread_condattr_destroy(&this->attribute);
    NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
    assert((result == 0) && u8"pthread conditional variable attribute is destroyed");

  }

  // ------------------------------------------------------------------------------------------- //

  MonotonicClockConditionAttribute::MonotonicClockConditionAttribute() {

    // Initialize the conditional attribute structure
    int result = ::pthread_condattr_init(&this->attribute);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not initialize pthread conditional variable attribute", result
      );
    }

    // Change the attribute's clock settings so the monotonic clock is used
    result = ::pthread_condattr_setclock(&this->attribute, CLOCK_MONOTONIC);
    if(unlikely(result != 0)) {
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not set pthread conditional variable attribute's clock id", result
      );
    }

  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  struct ::timespec PosixTimeApi::GetTimePlus(
    ::clockid_t clock, std::chrono::microseconds addedTime
  ) {
    struct ::timespec futureTime;

    // Query the specified clock's current time
    int result = ::clock_gettime(clock, &futureTime);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get time from clock", errorNumber
      );
    }

    // Calculate the future point in time by adding the requested number of milliseconds
    {
      const std::size_t NanoSecondsPerMicrosecond = 1000; // 1,000 ns = 1 μs
      const std::size_t NanoSecondsPerSecond = 1000000000; // 1,000,000,000 ns = 1 s

      // timespec has seconds and nanoseconds, so divide the microseconds into full seconds
      // and remainder microseconds to deal with this
      ::ldiv_t divisionResults = ::ldiv(addedTime.count(), 1000000);

      // If the summed nanoseconds add up to more than one second, increment the timespec's
      // seconds, otherwise just assign the summed nanoseconds.
      std::size_t nanoseconds = (
        divisionResults.rem * NanoSecondsPerMicrosecond + futureTime.tv_nsec
      );
      if(nanoseconds >= NanoSecondsPerSecond) {
        futureTime.tv_sec += divisionResults.quot + 1;
        futureTime.tv_nsec = nanoseconds - NanoSecondsPerSecond;
      } else {
        futureTime.tv_sec += divisionResults.quot;
        futureTime.tv_nsec = nanoseconds;
      }
    }

    return futureTime;
  }

  // ------------------------------------------------------------------------------------------- //

  struct ::timespec PosixTimeApi::GetTimePlus(
    ::clockid_t clock, std::chrono::milliseconds addedTime
  ) {
    struct ::timespec futureTime;

    // Query the specified clock's current time
    int result = ::clock_gettime(clock, &futureTime);
    if(unlikely(result == -1)) {
      int errorNumber = errno;
      Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
        u8"Could not get time from clock", errorNumber
      );
    }

    // Calculate the future point in time by adding the requested number of milliseconds
    {
      const std::size_t NanoSecondsPerMillisecond = 1000000; // 1,000,000 ns = 1 ms
      const std::size_t NanoSecondsPerSecond = 1000000000; // 1,000,000,000 ns = 1 s

      // timespec has seconds and nanoseconds, so divide the milliseconds into full seconds
      // and remainder milliseconds to deal with this
      ::ldiv_t divisionResults = ::ldiv(addedTime.count(), 1000);

      // If the summed nanoseconds add up to more than one second, increment the timespec's
      // seconds, otherwise just assign the summed nanoseconds.
      std::size_t nanoseconds = (
        divisionResults.rem * NanoSecondsPerMillisecond + futureTime.tv_nsec
      );
      if(nanoseconds >= NanoSecondsPerSecond) {
        futureTime.tv_sec += divisionResults.quot + 1;
        futureTime.tv_nsec = nanoseconds - NanoSecondsPerSecond;
      } else {
        futureTime.tv_sec += divisionResults.quot;
        futureTime.tv_nsec = nanoseconds;
      }
    }

    return futureTime;
  }

  // ------------------------------------------------------------------------------------------- //

  struct ::timespec PosixTimeApi::GetRemainingTimeout(
    ::clockid_t clock,
    const struct ::timespec &startTime,
    std::chrono::microseconds timeout
  ) {

    // Get the current time on the specified clock
    struct ::timespec currentTime;
    {
      int result = ::clock_gettime(clock, &currentTime);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not get time from clock", errorNumber
        );
      }
    }

    // Calculate the time that has elapsed since the provided start time
    struct ::timespec elapsedTime;
    {
      const std::size_t NanoSecondsPerSecond = 1000000000; // 1,000,000,000 ns = 1 s

      assert(
        (
          (currentTime.tv_sec > startTime.tv_sec) ||
          (
            (currentTime.tv_sec == startTime.tv_sec) &&
            (currentTime.tv_nsec >= startTime.tv_nsec)
          )
        ) && u8"Start time lies in past"
      );

      if(currentTime.tv_nsec >= startTime.tv_nsec) {
        elapsedTime.tv_sec = currentTime.tv_sec - startTime.tv_sec;
        elapsedTime.tv_nsec = currentTime.tv_nsec - startTime.tv_nsec;
      } else {
        elapsedTime.tv_sec = currentTime.tv_sec - startTime.tv_sec - 1;
        elapsedTime.tv_nsec = NanoSecondsPerSecond - startTime.tv_nsec + currentTime.tv_nsec;
      }
    }

    // Calculate the remaining time and write the result into currentTime
    // (yes, we're misappropriating it!)
    {
      const std::size_t NanoSecondsPerMicrosecond = 1000; // 1,000 ns = 1 μs
      const std::size_t NanoSecondsPerSecond = 1000000000; // 1,000,000,000 ns = 1 s

      // timespec has seconds and nanoseconds, so divide the microseconds into full seconds
      // and remainder microseconds to deal with this
      ::ldiv_t divisionResults = ::ldiv(timeout.count(), 1000000);
      divisionResults.rem *= NanoSecondsPerMicrosecond;

      if(divisionResults.quot > elapsedTime.tv_sec) { // Difference at least one second
        if(divisionResults.rem >= elapsedTime.tv_nsec) {
          currentTime.tv_sec = divisionResults.quot - elapsedTime.tv_sec;
          currentTime.tv_nsec = divisionResults.rem - elapsedTime.tv_nsec;
        } else {
          currentTime.tv_sec = divisionResults.quot - elapsedTime.tv_sec - 1;
          currentTime.tv_nsec = NanoSecondsPerSecond - elapsedTime.tv_nsec + divisionResults.rem;
        }
      } else if(divisionResults.quot == elapsedTime.tv_sec) { // Difference less than one second
        if(divisionResults.rem >= elapsedTime.tv_nsec) { // A fraction of a second remains
          currentTime.tv_sec = 0;
          currentTime.tv_nsec = divisionResults.rem - elapsedTime.tv_nsec;
        } else { // Time has already elapsed (by a fraction of a second)
          currentTime.tv_sec = 0;
          currentTime.tv_nsec = 0;
        }
      } else { // Time has already elapsed (by more than a second)
        currentTime.tv_sec = 0;
        currentTime.tv_nsec = 0;
      }
    }

    return currentTime;

  }

  // ------------------------------------------------------------------------------------------- //

  bool PosixTimeApi::HasTimedOut(::clockid_t clock, const struct ::timespec &endTime) {
    struct ::timespec currentTime;
    {
      int result = ::clock_gettime(clock, &currentTime);
      if(unlikely(result == -1)) {
        int errorNumber = errno;
        Nuclex::Support::Platform::PosixApi::ThrowExceptionForSystemError(
          u8"Could not get time from clock", errorNumber
        );
      }
    }

    return (
      (currentTime.tv_sec > endTime.tv_sec) ||
      (
        (currentTime.tv_sec == endTime.tv_sec) &&
        (currentTime.tv_nsec >= endTime.tv_nsec)
      )
    );
  }

  // ------------------------------------------------------------------------------------------- //

  ::pthread_condattr_t *PosixTimeApi::GetMonotonicClockAttribute() {
    static MonotonicClockConditionAttribute sharedAttribute;
    return sharedAttribute.GetAttribute();
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // !defined(NUCLEX_SUPPORT_WINDOWS)
