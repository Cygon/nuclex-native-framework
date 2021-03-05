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

#ifndef NUCLEX_STORAGE_HELPERS_WRITEBUFFER_H
#define NUCLEX_STORAGE_HELPERS_WRITEBUFFER_H

#include "Nuclex/Storage/Config.h"

#include <vector> // for std::vector
#include <cstdint> // for std::uint8_t
#include <algorithm> // for std::copy_n()

namespace Nuclex { namespace Storage { namespace Helpers {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Writes data into a fixed-size buffer and when that one is full, stores any
  ///   overflow bytes for later processing
  /// </summary>
  /// <remarks>
  ///   This curious adapter is necessary because some libraries, like libzpaq and libcsc,
  ///   cannot be stopped generating output until all input bytes are consumed. Thus we
  ///   need to hold onto the extra bytes produced and hand them over to the caller when
  ///   he tries to process the next chunk of data.
  /// </remarks>
  class WriteBuffer {

    /// <summary>Initializes a new overflowable buffer</summary>
    public: WriteBuffer() :
      fixedBuffer(nullptr),
      remainingFixedBufferBytes(0),
      sideBuffer(),
      sideBufferReadIndex(0) {}

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~WriteBuffer() = default;

    /// <summary>Retrieves the number of bytes still available in the active buffer</summary>
    /// <returns>The number of bytes available in the fixed output buffer</returns>
    public: std::size_t CountRemainingBytes() const {
      return this->remainingFixedBufferBytes;
    }

    /// <summary>Checks whether the overflow buffer has been used</summary>
    /// <returns>True if there are bytes waiting in the overflow buffer</returns>
    public: bool HasOverflowBytes() const {
      return !this->sideBuffer.empty();
    }

    /// <summary>Assigns a new buffer into which output should be written</summary>
    /// <param name="newBuffer">Buffer into which output should be written until full</param>
    /// <param name="byteCount">Number of bytes that fit in the output buffer</param>
    /// <returns>
    ///   The number of bytes that have been copied into the output buffer straight away
    ///   because they were still waiting from overflow of the previous output buffer
    /// </param>
    public: std::size_t UseFixedBuffer(
      std::uint8_t *newBuffer, std::size_t byteCount
    ) {

      // If we have buffered no overflowed data, we can just adopt the buffer
      // directly and avoid most of the bookkeeping
      if(this->sideBuffer.empty()) {

        this->fixedBuffer = newBuffer;
        this->remainingFixedBufferBytes = byteCount;
        return 0;

      } else { // There was overflow, push as much as we can into the new buffer

        std::size_t overflowedByteCount = (
          this->sideBuffer.size() - this->sideBufferReadIndex
        );

        // Can the new buffer hold all of the overflowed bytes? If so, we can empty
        // the overflow buffer and reset the counters
        if(byteCount >= overflowedByteCount) { // Does it all fit into the new buffer?
          std::copy_n(
            &this->sideBuffer[this->sideBufferReadIndex], overflowedByteCount,
            newBuffer
          );
          this->sideBuffer.clear();
          this->sideBufferReadIndex = 0;
          this->fixedBuffer = newBuffer + overflowedByteCount;
          this->remainingFixedBufferBytes = byteCount - overflowedByteCount;
          return overflowedByteCount;
        } else { // There's more overflow than can fit into the new buffer
          std::copy_n(
            &this->sideBuffer[this->sideBufferReadIndex], byteCount, newBuffer
          );
          this->sideBufferReadIndex += byteCount;
          this->remainingFixedBufferBytes = 0;
          return byteCount;
        }

      }
    }

    /// <summary>Writes a single byte to the output buffer</summary>
    /// <param name="outputByte">Byte that will be written to the output buffer</param> 
    public: void PutSingleByte(std::uint8_t outputByte) {
      if(this->remainingFixedBufferBytes > 0) {
        this->fixedBuffer[0] = outputByte;
        ++this->fixedBuffer;
        --this->remainingFixedBufferBytes;
      } else {
        this->sideBuffer.push_back(outputByte);
      }
    }

    /// <summary>Writes multiple bytes to the output buffer</summary>
    /// <param name="buffer">Buffer holding the bytes that will be written</param>
    /// <param name="byteCount">Number of bytes that will be written</param>
    public: void Write(const std::uint8_t *buffer, std::size_t byteCount) {

      // If the fixed-size buffer is assigned and large enough to hold all bytes,
      // dump the data there with the minimum amount of fuzz possible
      if(byteCount <= this->remainingFixedBufferBytes) {

        std::copy_n(buffer, byteCount, this->fixedBuffer);
        this->fixedBuffer += byteCount;
        this->remainingFixedBufferBytes -= byteCount;

      } else { // Otherwise, some or all bytes need to go into the overflow buffer

        // Is there any space left in the fixed-size buffer? If so, we're in
        // a situation where we must write one part of the data into the fixed-size
        // buffer and the remainder into the overflow buffer
        if(this->remainingFixedBufferBytes > 0) {
          std::copy_n(buffer, this->remainingFixedBufferBytes, this->fixedBuffer);
          buffer += this->remainingFixedBufferBytes;
          byteCount -= static_cast<int>(this->remainingFixedBufferBytes);
          this->remainingFixedBufferBytes = 0;
        }

        // Any data that didn't fit into the fixed-size buffer is now appended to
        // our overflow buffer
        std::size_t firstByteIndex = this->sideBuffer.size();
        this->sideBuffer.resize(firstByteIndex + byteCount);
        std::copy_n(buffer, byteCount, &this->sideBuffer[firstByteIndex]);

      }
    };

    /// <summary>Buffer into which output data should be written until full</summary>
    private: std::uint8_t *fixedBuffer;
    /// <summary>Number of bytes still available in the output buffer</summary>
    private: std::size_t remainingFixedBufferBytes; // replace with end pointer

    /// <summary>Buffer for data added after the output buffer became full</summary>
    private: std::vector<std::uint8_t> sideBuffer;
    /// <summary>Index at which the next read of the overflow buffer takes place</summary>
    private: std::size_t sideBufferReadIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Helpers

#endif // NUCLEX_STORAGE_HELPERS_WRITEBUFFER_H
