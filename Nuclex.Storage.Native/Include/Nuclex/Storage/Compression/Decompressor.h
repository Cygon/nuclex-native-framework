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

#ifndef NUCLEX_STORAGE_COMPRESSION_DECOMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_DECOMPRESSOR_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Compression/StopReason.h"

#include <cstddef>
#include <cstdint>

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  class InputStream;
  class OutputStream;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decompresses data</summary>
  /// <remarks>
  ///   <para>
  ///     You should symmetrically create one decompressor to decompress each thing you
  ///     used a compressor to compress (i.e. it is not guaranteed that you can use
  ///     the same decompressor to successively decompress two buffers that were compressed
  ///     by two separate compressors.
  ///   </para>
  ///   <para>
  ///     It may also be expensive to keep decompressors around (some of the high-ratio
  ///     compressors will have large internal data structures), so you should only create
  ///     decompressors when needed and destroy them right away after you're done.
  ///   </para>
  /// </remarks>
  class Decompressor {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_STORAGE_API virtual ~Decompressor() = default;

    /// <summary>
    ///   Decompresses the data in the input buffer and writes it to the output buffer
    /// </summary>
    /// <param name="uncompressedBuffer">Buffer containing the compressed data</param>
    /// <param name="uncompressedByteCount">
    ///   Number of compressed bytes in the compressed buffer. Will be set to
    ///   the number of remaining bytes when the method returns
    /// </param>
    /// <param name="outputBuffer">Buffer in which the uncompressed data will be stored</param>
    /// <param name="outputByteCount">
    ///   Available space in the output buffer. Will be set to the bytes actually stored
    ///   in the output buffer when the method returns
    /// </param>
    /// <returns>
    ///   The reason why the method stopped processing data. This may either be because
    ///   all available input was decompressed, (<see cref="StopReason.InputBufferExhausted" />)
    ///   or because the decompressor ran out of space in the output buffer
    ///   (<see cref="StopReason.OutputBufferFull" />).
    /// </returns>
    /// <remarks>
    ///   The decompressor may keep some data in an internal buffer. When the method
    ///   returns, the output is not guaranteed to be complete. You need to call
    ///   <see cref="Finish" /> as the final method before the output is guaranteed
    ///   to include all bytes that were originally compressed.
    /// </remarks>
    public: virtual StopReason Process(
      const std::uint8_t *compressedBuffer, std::size_t &compressedByteCount,
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) = 0;

    /// <summary>Finishes decompressing and writes any remaining output bytes</summary>
    /// <param name="outputBuffer">Buffer in which the decompressed data will be stored</param>
    /// <param name="outputByteCount">
    ///   Available space in the output buffer. Will be set to the bytes actually stored
    ///   in the output buffer when the method returns.
    /// </param>
    /// <returns>
    ///   The reason why the method stopped processing. This should normally be the
    ///   value <see cref="StopReason.Finished" /> but may also be
    ///   <see cref="StopReason.OutputBufferFull" /> if the output buffer was insufficient
    ///   to output all data (in which case you need to call Finish() another time).
    /// </returns>
    public: virtual StopReason Finish(
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) = 0;

#if defined(NUCLEX_STORAGE_COMPRESSION_CONVENIENCE_METHODS)
    /// <summary>Decompresses the provided buffer</summary>
    /// <param name="compressedBuffer">Buffer holding the compressed data</param>
    /// <param name="compressedByteCount">
    ///   Number of compressed bytes that will be decompressed
    /// </paam>
    /// <param name="outputStream">
    ///   Stream into which the uncompressed data will be written
    /// </param>
    public: virtual void Process(
      const std::uint8_t *compressedBuffer, std::size_t compressedByteCount,
      Binary::OutputStream &outputStream
    ) = 0;
#endif

#if defined(NUCLEX_STORAGE_COMPRESSION_CONVENIENCE_METHODS)
    /// <summary>Decompresses an input stream up to its end</summary>
    /// <param name="inputStream">Stream from which the compressed data will be taken</param>
    /// <param name="outputStream">
    ///   Stream into which the uncompressed data will be written
    /// </param>
    public: virtual void Process(
      Binary::InputStream &inputStream, Binary::OutputStream &outputStream
    ) = 0;
#endif

#if defined(NUCLEX_STORAGE_COMPRESSION_CONVENIENCE_METHODS)
    public: virtual void Finish(Binary::OutputStream &outputStream);
#endif

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression

#endif // NUCLEX_STORAGE_COMPRESSION_DECOMPRESSOR_H
