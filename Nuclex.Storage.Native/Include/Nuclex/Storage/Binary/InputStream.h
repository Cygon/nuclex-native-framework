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

#ifndef NUCLEX_STORAGE_BINARY_INPUTSTREAM_H
#define NUCLEX_STORAGE_BINARY_INPUTSTREAM_H

#include "Nuclex/Storage/Config.h"

#include <cstddef>
#include <cstdint>

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stream from which data can be read</summary>
  class InputStream {

    /// <summary>Releases all resources owned by the input stream</summary>
    /// <remarks>
    ///   If any callers are still waiting in a blocking read call, they will receive
    ///   an exception before the stream is destroyed.
    /// </remarks>
    public: NUCLEX_STORAGE_API virtual ~InputStream() = default;

    /// <summary>Checks whether more data is available from the stream</summary>
    /// <returns>True if there's more data that can be read from the stream</returns>
    /// <remarks>
    ///   <para>
    ///     Streams do not always have a known length (i.e. more data can arrive from
    ///     a network connection or a file being piped to the process). If the stream
    ///     is of such a type, this method checks whether there's at least 1 more byte
    ///     that can be read from the stream without blocking at the time of the call.
    ///   </para>
    ///   <para>
    ///     If the stream is of a fixed length, it will merely return whether
    ///     the file cursor is at least one byte away from the end.
    ///   </para>
    /// </remarks>
    public: virtual bool IsMoreDataAvailable() const = 0;

    /// <summary>Reads up to the specified number of bytes from the stream</summary>
    /// <param name="buffer">Buffer in which the data will be stored</param>
    /// <param name="byteCount">
    ///   Maximum number of bytes to read from the stream, will receive the number
    ///   of bytes actually placed in the buffer
    /// </param>
    /// <param name="requiredByteCount">
    ///   Number of bytes that should at least be written to the buffer
    /// </param>
    /// <returns>True if the end of the stream was reached, false otherwise</returns>
    /// <remarks>
    ///   <para>
    ///     This method tries to read up to <see cref="maximumByteCount" /> into
    ///     the provided buffer. It will provide as many bytes as it can provide
    ///     without waiting for its source.
    ///   </para>
    ///   <para>
    ///     The <see cref="requiredByteCount" /> parameter can modify that behavior.
    ///     If the required byte count is greater than 0, the method will wait until
    ///     at least the specified number of bytes can be provided to the caller.
    ///     An exception will be thrown if a stream-dependent timeout is reached while
    ///     waiting for data or if the stream is closed while waiting for data.
    ///   </para>
    ///   <para>
    ///     This behavior is useful to avoid needless busywork, i.e. if an algorithm
    ///     knows that it needs at least 100 bytes to continue, it can request at least
    ///     that number of bytes and avoid being trickle-fed individual bytes that it
    ///     can merely append to its buffer.
    ///   </para>
    ///   <para>
    ///     If the required byte count is greather than 0 and the stream is closed,
    ///     the method will throw an exception. There is no successful return where
    ///     less bytes will be provided than asked for via the required byte count.
    ///   </para>
    /// </remarks>
    public: virtual bool ReadUpTo(
      std::uint8_t *buffer, std::size_t &byteCount, std::size_t requiredByteCount = 1
    ) = 0;

#if 0
    /// <summary>
    ///   Asks the input stream for an optional buffer into which data from ReadUpTo()
    ///   can be read
    /// </summary>
    /// <param name="bufferSize">
    ///   Will be set to the number of bytes that fit in the buffer
    /// </param>
    /// <returns>
    ///   The address of a buffer that can be used to read from the stream or NULL if
    ///   the stream does not provide one
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This method allows for some advanced optimization but can be safely ignored if
    ///     you're just normally reading from a stream. If implemented, it will return
    ///     a buffer provided by the stream (and in turn, could be part of the stream through
    ///     a memory-mapped file, socket buffer, locked GPU memory or other source).
    ///   </para>
    ///   <para>
    ///     A stream implementing the GetBuffer() method should check in its
    ///     <see cref="ReadUpTo" /> method whether the passed pointer matches this returned
    ///     buffer, in which case an additional memory copy from the socket buffer, GPU 
    ///     memory or other internal memory block to the caller's buffer can be avoided.
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
#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary

#endif // NUCLEX_STORAGE_BINARY_INPUTSTREAM_H
