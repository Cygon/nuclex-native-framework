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

#ifndef NUCLEX_STORAGE_COMPRESSION_ZLIB_DEFLATEDECOMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_ZLIB_DEFLATEDECOMPRESSOR_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_ZLIB)

#include "Nuclex/Storage/Compression/Decompressor.h"

#include <zlib.h>

namespace Nuclex { namespace Storage { namespace Compression { namespace ZLib {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Decompresses data using the ZLib implementation of the deflate algorithm</summary>
  class DeflateDecompressor : public Decompressor {

    /// <summary>Initializes a new ZLib decompressor</summary>
    public: DeflateDecompressor();

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~DeflateDecompressor() override;

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
    ///   all available input was decompressed, or because the decompressor ran out of
    ///   space in the output buffer.
    /// </returns>
    public: StopReason Process(
      const std::uint8_t *compressedBuffer, std::size_t &compressedByteCount,
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) override;

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
    public: StopReason Finish(
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) override;

    /// <summary>Maintains the ZLib deflate decompressor's state</summary>
    private: ::z_stream stream;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::ZLib

#endif // defined(NUCLEX_STORAGE_HAVE_ZLIB)

#endif // NUCLEX_STORAGE_COMPRESSION_ZLIB_DEFLATEDECOMPRESSOR_H
