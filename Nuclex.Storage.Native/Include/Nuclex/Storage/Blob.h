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

#ifndef NUCLEX_STORAGE_BLOB_H
#define NUCLEX_STORAGE_BLOB_H

#include "Config.h"

#include <cstdint>
#include <cstddef>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Large binary piece of data supporting random access</summary>
  class Blob {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_STORAGE_API virtual ~Blob() = default;

    /// <summary>Determines the size of the binary data in bytes</summary>
    /// <returns>The size of the binary data in bytes</returns>
    public: virtual std::uint64_t GetSize() const = 0;

    /// <summary>Reads raw data from the blob</summary>
    /// <param name="location">Absolute position data will be read from</param>
    /// <param name="buffer">Buffer into which data will be read</param>
    /// <param name="count">Number of bytes that will be read</param>
    public: virtual void ReadAt(
      std::uint64_t location, void *buffer, std::size_t count
    ) const = 0;

    /// <summary>Writes raw data into the blob</summary>
    /// <param name="location">Absolute position data will be written to</param>
    /// <param name="buffer">Buffer from which data will be taken</param>
    /// <param name="count">Number of bytes that will be written</param>
    public: virtual void WriteAt(
      std::uint64_t location, const void *buffer, std::size_t count
    ) = 0;

    /// <summary>Flushes all caches that may be chained to the blob</summary>
    public: virtual void Flush() = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

#endif // NUCLEX_STORAGE_BLOB_H
