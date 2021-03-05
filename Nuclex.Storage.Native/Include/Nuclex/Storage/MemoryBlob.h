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

#ifndef NUCLEX_STORAGE_MEMORYBLOB_H
#define NUCLEX_STORAGE_MEMORYBLOB_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Blob.h"

#include <vector>
#include <mutex>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Chunk of arbitrary data stored completely in system memory</summary>
  class MemoryBlob : public Blob {

    /// <summary>Initializes a new in-memory blob</summary>
    public: NUCLEX_STORAGE_API MemoryBlob() = default;

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_STORAGE_API virtual ~MemoryBlob() override = default;

    /// <summary>Determines the size of the binary data in bytes</summary>
    /// <returns>The size of the binary data in bytes</returns>
    public: NUCLEX_STORAGE_API virtual std::uint64_t GetSize() const override {
      std::lock_guard<std::mutex> scope(this->mutex);
      return this->memory.size();
    }

    /// <summary>Reads raw data from the blob</summary>
    /// <param name="location">Absolute position data will be read from</param>
    /// <param name="buffer">Buffer into which data will be read</param>
    /// <param name="count">Number of bytes that will be read</param>
    public: NUCLEX_STORAGE_API virtual void ReadAt(
      std::uint64_t location, void *buffer, std::size_t count
    ) const override;

    /// <summary>Writes raw data into the blob</summary>
    /// <param name="location">Absolute position data will be written to</param>
    /// <param name="buffer">Buffer from which data will be taken</param>
    /// <param name="count">Number of bytes that will be written</param>
    public: NUCLEX_STORAGE_API virtual void WriteAt(
      std::uint64_t location, const void *buffer, std::size_t count
    ) override;

    /// <summary>Flushes all caches that may be chained to the blob</summary>
    public: NUCLEX_STORAGE_API virtual void Flush() override {}

    /// <summary>Stores the data of the in-memory blob</summary>
    private: std::vector<std::uint8_t> memory;
    /// <summary>Mutex used to sequentialize accesses to the blob</summary>
    private: mutable std::mutex mutex;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

#endif // NUCLEX_STORAGE_MEMORYBLOB_H
