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

#ifndef NUCLEX_SUPPORT_ERRORS_TIMEOUTERROR_H
#define NUCLEX_SUPPORT_ERRORS_TIMEOUTERROR_H

#include "Nuclex/Support/Config.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Support { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that a time-limited action was not completed within the alloted time
  /// </summary>
  class NUCLEX_SUPPORT_TYPE TimeoutError : public std::runtime_error {

    /// <summary>Initializes a new wait timeout error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit TimeoutError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a new wait timeout error</summary>
    /// <param name="message">Message that describes the error</param>
    public: NUCLEX_SUPPORT_API explicit TimeoutError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Errors

#endif // NUCLEX_SUPPORT_ERRORS_TIMEOUTERROR_H
