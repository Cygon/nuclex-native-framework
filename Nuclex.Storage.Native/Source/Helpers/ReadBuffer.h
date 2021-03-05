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

#ifndef NUCLEX_STORAGE_HELPERS_READBUFFER_H
#define NUCLEX_STORAGE_HELPERS_READBUFFER_H

#include "Nuclex/Storage/Config.h"

#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t
#include <algorithm> // for std::copy_n()
#include <cassert> // for assert()

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Reads data from a fixed-size buffer but also holds data in an internal buffer
  ///   that can be filled from the fixed-size buffer if it cannot be consumed right away
  /// </summary>
  class ReadBuffer {

    /// <summary>Initializes a new read buffer</summary>
    public: ReadBuffer() :
      fixedBuffer(nullptr),
      remainingFixedBufferBytes(0),
      sideBuffer(),
      sideBufferReadIndex(0) {}

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~ReadBuffer() = default;

    /// <summary>Counts the number of cached bytes without the fixed buffer</summary>
    /// <returns>The number of bytes the read buffer has cached internally</returns>
    /// <remarks>
    ///   This is useful if compression algorithms want to be fed a pointer because
    ///   it can be passed directly from this buffer.
    /// </remarks>
    public: std::size_t CountCachedBytes() const {
      return this->sideBuffer.size() - this->sideBufferReadIndex;
    }

    /// <summary>Retrieves the number of bytes available to read</summary>
    /// <returns>The number of bytes available for reading</returns>
    public: std::size_t CountAvailableBytes() const {
      if(this->fixedBuffer == nullptr) {
        return this->sideBuffer.size() - this->sideBufferReadIndex;
      } else {
        return (
          (this->sideBuffer.size() - this->sideBufferReadIndex) +
          this->remainingFixedBufferBytes
        );
      }
    }

    /// <summary>Returns a pointer to the buffer's internally cached data</summary>
    /// <returns>A point to the buffer's cached data</returns>
    public: const std::uint8_t *GetCachedData() const {
      return this->sideBuffer.data() + this->sideBufferReadIndex;
    }

    /// <summary>Skips data in the internal cache (and only there!)</summary>
    /// <param name="count">Number of bytes the will be skipped</param>
    /// <remarks>
    ///   This is useful if you used GetCachedData() to provide data from the internal
    ///   cache to an external library and some were processed / consumed.
    /// </remarks>
    public: void SkipCachedBytes(std::size_t count) {
      this->sideBufferReadIndex += count;
    }

    /// <summary>Reads data from the buffer</summary>
    /// <param name="target">Target buffer into which data will be read</param>
    /// <param name="count">Number of bytes the will be read</param>
    /// <remarks>
    ///   <para>
    ///     Normally, you wouldn't use this method because is enforces a buffer copy.
    ///     In the worst case, all data would be copied two times (once from input into
    ///     the cache, then from cache into the read buffer).
    ///   <para>
    ///     However, some third-party libraries may want you to implement
    ///     a &quot;StreamReader&quot;-like interface and this allows you to do that.
    ///     It's the caller's responsibility to check that the requested number of
    ///     bytes is actually available from the ReadBuffer.
    ///   </para>
    /// </remarks>
    public: void Read(std::uint8_t *target, std::size_t count) {
      if(this->fixedBuffer == nullptr) {
        assert(
          ((this->sideBuffer.size() - this->sideBufferReadIndex) >= count) &&
          u8"Buffer holds enough data for read request"
        );
        std::copy_n(this->sideBuffer.data() + this->sideBufferReadIndex, count, target);
        this->sideBufferReadIndex += count;
      } else {
        std::size_t sideBufferByteCount = this->sideBuffer.size() - this->sideBufferReadIndex;
        if(sideBufferByteCount >= count) {
          std::copy_n(this->sideBuffer.data() + this->sideBufferReadIndex, count, target);
          this->sideBufferReadIndex += count;
        } else {
          std::copy_n(
            this->sideBuffer.data() + this->sideBufferReadIndex, sideBufferByteCount, target
          );
          count -= sideBufferByteCount;

          assert(
            (this->remainingFixedBufferBytes >= count) &&
            u8"Fixed buffer holds enough data for read request"
          );
          std::copy_n(this->fixedBuffer, count, target + sideBufferByteCount);

          this->sideBuffer.clear();
          this->sideBufferReadIndex = 0;
          this->fixedBuffer += count;
          this->remainingFixedBufferBytes -= count;
        }
      }
    }

    /// <summary>Assigns a fixed buffer as the data source</summary>
    /// <param name="buffer">Buffer from which data will be read</param>
    /// <param name="byteCount">Number of bytes the buffer is holding</param>
    /// <remarks>
    ///   In the simplest case, the ReadBuffer will simply pass through any read
    ///   requests to this buffer. If there is cached data, however, reads will first
    ///   take data from there (FIFO concept) and then use this buffer. The entirety
    ///   of data remaining in this fixed buffer can be added to the internal cache
    ///   by calling the <see cref="CacheFixedBufferContents" /> method.
    /// </remarks>
    public: void UseFixedBuffer(const std::uint8_t *buffer, std::size_t byteCount) {
      assert(
        (this->remainingFixedBufferBytes == 0) &&
        u8"Fixed buffer switched before all its contents were consumed"
      );

      this->fixedBuffer = buffer;
      this->remainingFixedBufferBytes = byteCount;
    }

    /// <summary>Caches all remaining contents of the assigned fixed buffer</summary>
    /// <remarks>
    ///   This releases the fixed buffer and adds any of its contents that have not been
    ///   used yet to the internal cache.
    /// </remarks>
    public: void CacheFixedBufferContents() {
      std::size_t sideBufferAppendIndex = this->sideBuffer.size();

      // Shift back the side buffer if it can be done cheaply. Doing so when it's
      // half-empty mostly guesswork, the important part is to do it at some point
      // in order to avoid endless buffer growth as the read index moves ever further.
      if((sideBufferAppendIndex / 2) < this->sideBufferReadIndex) {
        sideBufferAppendIndex -= this->sideBufferReadIndex;
        std::copy_n(
          this->sideBuffer.data() + this->sideBufferReadIndex,
          sideBufferAppendIndex,
          this->sideBuffer.data()
        );
        this->sideBufferReadIndex = 0;

        // No resize() here because the next block is going to do that anyway.
      }

      // Append the fixed buffer contents to the side buffer      
      {
        this->sideBuffer.resize(sideBufferAppendIndex + this->remainingFixedBufferBytes);
        std::copy_n(
          this->fixedBuffer, this->remainingFixedBufferBytes,
          this->sideBuffer.data() + sideBufferAppendIndex
        );

        this->fixedBuffer = nullptr;
        this->remainingFixedBufferBytes = 0;
      }
    }

    /// <summary>Fixed buffer currently assigned to the reader</summary>
    private: const std::uint8_t *fixedBuffer;
    /// <summary>Number of bytes remaining to be read from the fixed buffer</summary>
    private: std::size_t remainingFixedBufferBytes;

    /// <summary>
    ///   Buffer into which extra data is written when the output buffer is full
    /// </summary>
    private: std::vector<std::uint8_t> sideBuffer;
    /// <summary>Index at which the next read of the overflow buffer takes place</summary>
    private: std::size_t sideBufferReadIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers

#endif // NUCLEX_STORAGE_HELPERS_OVERFLOWABLEBUFFER_H
