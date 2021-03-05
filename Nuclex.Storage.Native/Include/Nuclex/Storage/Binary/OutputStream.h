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

#ifndef NUCLEX_STORAGE_BINARY_OUTPUTSTREAM_H
#define NUCLEX_STORAGE_BINARY_OUTPUTSTREAM_H

#include "Nuclex/Storage/Config.h"

#include <cstddef>
#include <cstdint>

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stream into which data can be writen</summary>
  class OutputStream {

    /// <summary>Releases all resources owned by the output stream</summary>
    /// <remarks>
    ///   If any callers are still waiting in a blocking write call, they will receive
    ///   an exception before the stream is destroyed.
    /// </remarks>
    public: NUCLEX_STORAGE_API virtual ~OutputStream() = default;

    /// <summary>Checks whether the stream is able to accept at least one more byte</summary>
    /// <returns>True if the stream is ready to accept more data</returns>
    /// <remarks>
    ///   This is relevant for network streams (i.e. uploads) where a sliding window
    ///   or RTS ("ready to receive") line is being used. For files or memory streams,
    ///   this method should simply return whether the disk is full or memory exhausted.
    /// </remarks>
    public: virtual bool CanAcceptMoreData() const = 0;

    /// <summary>Writes up to the specified number of bytes into the stream</summary>
    /// <param name="buffer">Buffer holding the data that will be written</param>
    /// <param name="byteCount">
    ///   Maximum number of bytes that will be written to the stream, set to
    ///   the number of bytes actually written.
    /// </param>
    /// <remarks>
    ///   <para>
    ///     Writes the provided data to the output stream. If the output stream is dynamic
    ///     and has a limited send buffer or the target can be busy, none or only a part
    ///     of the provided data may be written.
    ///   </para>
    ///   <para>
    ///     The <see cref="minimumByteCount" /> parameter can modify this behavior.
    ///     If the minimum byte count is greater than 0, the call will block until at
    ///     least the specified number of bytes have been written. An exception will
    ///     be thrown if a stream-dependent timeout is reached while waiting to deliver
    ///     the data or if the stream is closed before all data could be sent.
    ///   </para>
    ///   <para>
    ///     There is no guarantee as to how many bytes were already delivered if an
    ///     exception if thrown by this method.
    ///   </para>
    /// </remarks>
    public: virtual void WriteUpTo(
      const std::uint8_t *buffer, std::size_t &byteCount, std::size_t minimumByteCount = 1
    ) = 0;

    /// <summary>
    ///   Asks the output stream for an optional buffer that can be used to store data
    ///   for the <see cref="WriteUpTo" /> method
    /// </summary>
    /// <param name="bufferSize">
    ///   Should be set to the number of bytes that fit in the buffer
    /// </param>
    /// <returns>
    ///   The address of a buffer that can be used to write to the stream or NULL if
    ///   the stream does not provide one
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This method allows for some advanced optimization but can be safely ignored if
    ///     you're just normally writing to a stream. If implemented, it will return
    ///     a buffer provided by the stream (and in turn, could be part of the stream through
    ///     a memory-mapped file, socket buffer, locked GPU memory or other source).
    ///   </para>
    ///   <para>
    ///     A stream implementing the GetBuffer() method should check in its
    ///     <see cref="WriteUpTo" /> method whether the passed pointer matches this returned
    ///     buffer, in which case an additional memory copy from the caller-provided buffer
    ///     into a socket buffer, GPU memory or other can be avoided.
    ///   </para>
    ///   <para>
    ///     The caller may have ignored the provided buffer (for example, because it was
    ///     too small), so if the pointer passed to <see cref="WriteUpTo" /> doesn't match,
    ///     the output stream implementation needs to proceed as normal.
    ///   </para>
    /// </remarks>
    public: virtual std::uint8_t *GetBuffer(std::size_t &bufferSize) {
      (void)bufferSize;
      return nullptr;
    }

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary

#endif // NUCLEX_STORAGE_BINARY_OUTPUTSTREAM_H
