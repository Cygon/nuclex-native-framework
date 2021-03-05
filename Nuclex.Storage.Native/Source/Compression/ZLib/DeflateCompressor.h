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

#ifndef NUCLEX_STORAGE_COMPRESSION_ZLIB_DEFLATECOMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_ZLIB_DEFLATECOMPRESSOR_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_ZLIB)

#include "Nuclex/Storage/Compression/Compressor.h"

#include <zlib.h>

namespace Nuclex { namespace Storage { namespace Compression { namespace ZLib {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compresses data using the ZLib implementation of the deflate algorithm</summary>
  class DeflateCompressor : public Compressor {

    /// <summary>Initializes a new ZLib compressor</summary>
    /// <param name="level">
    ///   Compression level that will be passed to the ZLib compressor
    /// </param>
    public: DeflateCompressor(int level = Z_DEFAULT_COMPRESSION);

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~DeflateCompressor() override;

    /// <summary>
    ///   Compresses the data in the input buffer and writes it to the output buffer
    /// </summary>
    /// <param name="uncompressedBuffer">Buffer containing the uncompressed data</param>
    /// <param name="uncompressedByteCount">
    ///   Number of uncompressed bytes in the uncompressed buffer. Will be set to
    ///   the number of remaining bytes when the method returns
    /// </param>
    /// <param name="outputBuffer">Buffer in which the compressed data will be stored</param>
    /// <param name="outputByteCount">
    ///   Available space in the output buffer. Will be set to the numer of bytes actually
    ///   stored in the output buffer when the method returns
    /// </param>
    /// <returns>
    ///   The reason why the method stopped processing data. This may either be because
    ///   all available input was compressed (<see cref="StopReason.InputBufferExhausted" />),
    ///   or because the compressor ran out of space in the output buffer
    ///   (<see cref="StopReason.OutputBufferFull" />).
    /// </returns>
    public: StopReason Process(
      const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) override;

    /// <summary>Finishes compressing and writes any remaining output bytes</summary>
    /// <param name="outputBuffer">Buffer in which the compressed data will be stored</param>
    /// <param name="outputByteCount">
    ///   Available space in the output buffer. Will be set to the numer of bytes actually
    ///   stored in the output buffer when the method returns
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

    /// <summary>Maintains the ZLib deflate compressor's state</summary>
    private: ::z_stream stream;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::ZLib

#endif // defined(NUCLEX_STORAGE_HAVE_ZLIB)

#endif // NUCLEX_STORAGE_COMPRESSION_ZLIB_DEFLATECOMPRESSOR_H
