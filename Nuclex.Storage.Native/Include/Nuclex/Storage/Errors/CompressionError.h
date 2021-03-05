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

#ifndef NUCLEX_STORAGE_ERRORS_COMPRESSIONERROR_H
#define NUCLEX_STORAGE_ERRORS_COMPRESSIONERROR_H

#include "Nuclex/Storage/Config.h"

#include <exception> // for std::runtime_error
#include <stdexcept> // also std::runtime_error on nonconforming compilers (hello MSVC)

// TODO: Derive from new base class 'CorruptDataError' that is useful to serializers?

namespace Nuclex { namespace Storage { namespace Errors {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Indicates that a data compression / decompression algorithm ran into a problem
  /// </summary>
  /// <remarks>
  ///   Typically you would get this when decompressing data that got corrupted somehow,
  ///   but if there's an implementation issue with one of the compression algorithms,
  ///   this exception can also be triggered during data compression.
  /// </remarks>
  class CompressionError : public std::runtime_error {

    /// <summary>Initializes a new path problem error</summary>
    /// <param name="message">Message that describes the error</param>
    public: explicit CompressionError(const std::string &message) :
      std::runtime_error(message) {}

    /// <summary>Initializes a new path problem error</summary>
    /// <param name="errorCode">Error code reported by the operating system</param>
    /// <param name="message">Message that describes the error</param>
    public: explicit CompressionError(const char *message) :
      std::runtime_error(message) {}

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Errors

#endif // NUCLEX_STORAGE_ERRORS_COMPRESSIONERROR_H
