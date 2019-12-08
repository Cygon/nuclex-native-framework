#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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
#define NUCLEX_STORAGE_SOURCE 1

#include "Nuclex/Storage/MemoryBlob.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Grows the vector to at least the specified capacity</summary>
  /// <param name="vector">Vector that will be grown</param>
  /// <param name="requiredCapacity">Minimum capacity the vector will be grown to</param>
  /// <remarks>
  ///   <para>
  ///     The <code>std::vector&lt;T&gt;::reserve()</code> method is free to do this itself,
  ///     but most implementations grow to the exact size, probably in the assumption that if
  ///     the developer goes through the effort of calling reserve(), he knows exactly
  ///     how much memory will be required (I'm not complaining).
  ///   </para>
  ///   <para>
  ///     This behavior could wreak havoc on performance if a binary blob writer was to
  ///     append small chunks of data (eg. 4 byte integers) to an already large blob.
  ///     Thus we ensure the reserve() behavior matches the scheme employed by push_back()
  ///     in most implementations of <code>std::vector&lt;T&gt;</code>.
  ///   </para>
  /// </remarks>
  template<typename T>
  void growVector(std::vector<T> &vector, std::size_t requiredCapacity) {
    std::size_t capacity = vector.capacity();
    if(capacity < requiredCapacity) {
      capacity += capacity / 2;
      if(capacity < requiredCapacity) {
        capacity = requiredCapacity;
      }

      vector.reserve(capacity);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  void MemoryBlob::ReadAt(std::uint64_t location, void *buffer, std::size_t count) const {
    if(location > std::numeric_limits<std::size_t>::max()) {
      throw std::out_of_range("Read location exceeds std::size_t for in-memory blob");
    }
    if(location + count > std::numeric_limits<std::size_t>::max()) {
      throw std::out_of_range("Read location exceeds std::size_t for in-memory blob");
    }

    std::lock_guard<std::mutex> scope(this->mutex); {
      std::copy_n(
        &this->memory.at(static_cast<std::size_t>(location)),
        count,
        static_cast<unsigned char *>(buffer)
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void MemoryBlob::WriteAt(std::uint64_t location, const void *buffer, std::size_t count) {
    if(location > std::numeric_limits<std::size_t>::max()) {
      throw std::out_of_range("Write location exceeds std::size_t for in-memory blob");
    }
    if(location + count > std::numeric_limits<std::size_t>::max()) {
      throw std::out_of_range("Write location exceeds std::size_t for in-memory blob");
    }

    std::size_t start = static_cast<std::size_t>(location);
    std::size_t end = start + count;

    std::lock_guard<std::mutex> scope(this->mutex); {

      // Grow our vector if its capacity might not be sufficient
      std::size_t blobSize = this->memory.size();
      if(start > blobSize) {
        throw std::out_of_range(
          "Attempted write past the end of the memory blob (would create undefined gap)"
        );
      }
      if(end > blobSize) {
        growVector(this->memory, end);
      }

      // Copy those bytes that overwrite existing data in the vector directly
      std::size_t overwriteCount = std::min(blobSize, end) - start;
      if(overwriteCount > 0) {
        std::copy_n(
          static_cast<const unsigned char *>(buffer),
          overwriteCount,
          &this->memory.at(static_cast<std::size_t>(location))
        );
      }

      // Append those bytes that grow the buffer using the insert() method
      if(count > overwriteCount) {
        this->memory.insert(
          this->memory.end(),
          static_cast<const unsigned char *>(buffer) + overwriteCount,
          static_cast<const unsigned char *>(buffer) + count
        );
      }

    } // mutex lock
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage
