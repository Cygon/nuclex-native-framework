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

#ifndef NUCLEX_SUPPORT_ERRORS_CANCELEDERROR_H
#define NUCLEX_SUPPORT_ERRORS_CANCELEDERROR_H

#include "Nuclex/Support/Config.h"

#include <future> // for std::future_error

namespace Nuclex { namespace Support { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Indicates that a task or process has been canceled</summary>
  /// <remarks>
  ///   This is often used together with std::future as the exception that's assigned to
  ///   the std::future when its normal result is no longer going to arrive (for example,
  ///   because the thread performing the work is shutting down).
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE CanceledError : public std::future_error {

    /// <summary>Initializes a cancellation-indicating error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CanceledError(const std::string &message) :
      std::future_error(std::future_errc::broken_promise),
      message(message) {}

    /// <summary>Initializes a cancellation-indicating error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit CanceledError(const char *message) :
      std::future_error(std::future_errc::broken_promise),
      message(message) {}

    /// <summary>Retrieves an error message describing the cancellation reason</summary>
    /// <returns>A message describing te reason for the cancellation</returns>>
    public: NUCLEX_SUPPORT_API virtual const char *what() const noexcept override {
      if(this->message.empty()) {
        return std::future_error::what();
      } else {
        return this->message.c_str();
      }
    }

    /// <summary>Error message describing the reason for the cancellation</summary>
    private: std::string message;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_CANCELEDERROR_H
