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

#ifndef NUCLEX_SUPPORT_PLATFORM_WINDOWSSYNCAPI_H
#define NUCLEX_SUPPORT_PLATFORM_WINDOWSSYNCAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"

#include <chrono> // for std::chrono::milliseconds

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the API used for advanced thread synchronization on Windows</summary>
  class WindowsSyncApi {

    #pragma region enum WaitResult

    /// <summary>Reasons for why <see cref="WaitOnAddress" /> has returned</summary>
    public: enum WaitResult {

      /// <summary>The wait was cancelled because the timeout was reached</summary>>
      TimedOut = -1,
      /// <summary>Either the monitored value changed or we woke spuriously</summary>
      ValueChanged = 1

    };

    #pragma endregion // enum WaitResult

    /// <summary>Waits for the specified wait variable to change in memory</summary>
    /// <typeparam name="TWaitVariable">
    ///   Type of the wait variable, must be either int8, int16, int32 or int64
    /// </typeparam>
    /// <param name="waitVariable">Wait variable that will be watched</param>
    /// <param name="comparedValue">Value the wait variable will be compared against</param>
    /// <param name="patience">Maximum time to wait in milliseconds</param>
    /// <param name="timeoutFlagOutput">
    ///   Is set to true if the wait was cancelled due to reaching its timeout
    /// </param>
    /// <returns>
    ///   True if the variable has probably changed, false if the variable remained
    ///   unchanged until the wait timeout was reached
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     There can be spurious wake-ups where the variable did not change its value
    ///     but some other unpredictable event (including false sharing) causes
    ///     this method to return.
    ///   </para>
    ///   <para>
    ///     If you have several variables to wait on, false sharing will result in a lot
    ///     of spurious wake-ups. To minimize spurious wake-ups in this specific case,
    ///     interleave the wait variables with other data (if possible) or pad the wait
    ///     variables so they each have at least a size of
    ///     <code>std::hardware_constructive_interference_size</code>. C++17 developers
    ///     can make use of <code>alignas()</code> for individual variables (but do take
    ///     care of neighboring variables if you can)
    ///   </para>
    /// </remarks>
    public: template<typename TWaitVariable>
    inline static WaitResult WaitOnAddress(
      const volatile TWaitVariable &waitVariable,
      TWaitVariable comparedValue,
      std::chrono::milliseconds patience
    );

    /// <summary>Waits for the specified wait variable to change in memory</summary>
    /// <typeparam name="TWaitVariable">
    ///   Type of the wait variable, must be either int8, int16, int32 or int64
    /// </typeparam>
    /// <param name="waitVariable">Wait variable that will be watched</param>
    /// <param name="comparedValue">Value the wait variable will be compared against</param>
    /// <returns>
    ///   True if the variable has probably changed, false if the wait was interrupted
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     There can be spurious wake-ups where the variable did not change its value
    ///     but some other unpredictable event (including false sharing) causes
    ///     this method to return.
    ///   </para>
    ///   <para>
    ///     If you have several variables to wait on, false sharing will result in a lot
    ///     of spurious wake-ups. To minimize spurious wake-ups in this specific case,
    ///     interleave the wait variables with other data (if possible) or pad the wait
    ///     variables so they each have at least a size of
    ///     <code>std::hardware_constructive_interference_size</code>. C++17 developers
    ///     can make use of <code>alignas()</code> for individual variables (but do take
    ///     care of neighboring variables if you can)
    ///   </para>
    /// </remarks>
    public: template<typename TWaitVariable>
    inline static WaitResult WaitOnAddress(
      const volatile TWaitVariable &waitVariable,
      TWaitVariable comparedValue
    );

    /// <summary>Wakes a single threads waiting for a value in memory to change</summary>
    /// <typeparam name="TWaitVariable">
    ///   Type of the wait variable, must be either int8, int16, int32 or int64
    /// </typeparam>
    /// <param name="waitVariable">
    ///   Variable for which one watching observer will be waken up
    /// </param>
    public: template<typename TWaitVariable>
    inline static void WakeByAddressSingle(
      const volatile TWaitVariable &waitVariable
    );

    /// <summary>Wakes all threads waiting for a value in memory to change</summary>
    /// <typeparam name="TWaitVariable">
    ///   Type of the wait variable, must be either int8, int16, int32 or int64
    /// </typeparam>
    /// <param name="waitVariable">
    ///   Variable for which all watching observers will be waken up
    /// </param>
    public: template<typename TWaitVariable>
    inline static void WakeByAddressAll(
      const volatile TWaitVariable &waitVariable
    );

    /// <summary>Waits for a value to change in memory</summary>
    /// <param name="waitVariable">Wait variable that will be watched</param>
    /// <param name="comparisonValue">Value the wait variable will be compared against</param>
    /// <param name="waitVariableByteCount">Size of the wait variable in bytes</param>
    /// <param name="patience">Maximum time to wait in milliseconds</param>
    /// <returns>
    ///   True if the variable has probably changed, false if the variable remained
    ///   unchanged until the wait timeout was reached
    /// </returns>
    private: static WaitResult waitOnAddressWithTimeout(
      const volatile void *waitVariableAddress,
      void *comparisonValue,
      std::size_t waitVariableByteCount,
      std::chrono::milliseconds patience
    );

    /// <summary>Waits for a value to change in memory</summary>
    /// <param name="waitVariableAddress">Memory address of the wait variable</param>
    /// <param name="comparedValue">
    ///   Memory address holding the value the wait variable will be compared against
    /// </param>
    /// <param name="waitVariableByteCount">Size of the wait variable in bytes</param>
    /// <returns>
    ///   True if the variable has probably changed, false if the wait was interrupted
    /// </returns>
    private: static WaitResult waitOnAddressNoTimeout(
      const volatile void *waitVariableAddress,
      void *comparisonValue,
      std::size_t waitVariableByteCount
    );

    /// <summary>Wakes a single thread waiting for a value in memory to change</summary>
    /// <param name="waitVariableAddress">
    ///   Memory address of the value for which one observer will be waken up
    /// </param>
    private: static void wakeByAddressSingle(const volatile void *waitVariableAddress);

    /// <summary>Wakes all threads waiting for a value in memory to change</summary>
    /// <param name="waitVariableAddress">
    ///   Memory address of the value for which any observers will be waken up
    /// </param>
    private: static void wakeByAddressAll(const volatile void *waitVariableAddress);

  };

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint8_t &waitVariable,
    std::uint8_t comparedValue,
    std::chrono::milliseconds patience
  ) {
    return waitOnAddressWithTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint8_t), patience
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint16_t &waitVariable,
    std::uint16_t comparedValue,
    std::chrono::milliseconds patience
  ) {
    return waitOnAddressWithTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint16_t), patience
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint32_t &waitVariable,
    std::uint32_t comparedValue,
    std::chrono::milliseconds patience
  ) {
    return waitOnAddressWithTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint32_t), patience
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint64_t &waitVariable,
    std::uint64_t comparedValue,
    std::chrono::milliseconds patience
  ) {
    return waitOnAddressWithTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint64_t), patience
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint8_t &waitVariable,
    std::uint8_t comparedValue
  ) {
    return waitOnAddressNoTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint8_t)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint16_t &waitVariable,
    std::uint16_t comparedValue
  ) {
    return waitOnAddressNoTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint16_t)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint32_t &waitVariable,
    std::uint32_t comparedValue
  ) {
    return waitOnAddressNoTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint32_t)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline WindowsSyncApi::WaitResult WindowsSyncApi::WaitOnAddress(
    const volatile std::uint64_t &waitVariable,
    std::uint64_t comparedValue
  ) {
    return waitOnAddressNoTimeout(
      &waitVariable, &comparedValue, sizeof(std::uint64_t)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressSingle(const volatile std::uint8_t &waitVariable) {
    wakeByAddressSingle(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressSingle(const volatile std::uint16_t &waitVariable) {
    wakeByAddressSingle(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressSingle(const volatile std::uint32_t &waitVariable) {
    wakeByAddressSingle(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressSingle(const volatile std::uint64_t &waitVariable) {
    wakeByAddressSingle(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressAll(const volatile std::uint8_t &waitVariable) {
    wakeByAddressAll(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressAll(const volatile std::uint16_t &waitVariable) {
    wakeByAddressAll(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressAll(const volatile std::uint32_t &waitVariable) {
    wakeByAddressAll(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  inline void WindowsSyncApi::WakeByAddressAll(const volatile std::uint64_t &waitVariable) {
    wakeByAddressAll(&waitVariable);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_WINDOWSSYNCAPI_H
