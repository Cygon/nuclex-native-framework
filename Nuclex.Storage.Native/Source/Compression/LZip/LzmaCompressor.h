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

#ifndef NUCLEX_STORAGE_COMPRESSION_LZIP_LZMACOMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_LZIP_LZMACOMPRESSOR_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_LZIP)

#include "Nuclex/Storage/Compression/Compressor.h"

#ifndef NUCLEX_STORAGE_COMPRESSION_LZIP_LZLIB_H
#define NUCLEX_STORAGE_COMPRESSION_LZIP_LZLIB_H
#include <lzlib.h>
#endif

namespace Nuclex { namespace Storage { namespace Compression { namespace LZip {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compresses data using the LZip implementation of the LZMA algorithm</summary>
  class LzmaCompressor : public Compressor {

    /// <summary>Initializes a new LZMA compressor</summary>
    /// <param name="quality">
    ///   Compression quality that will be passed to the LZMA encoder
    /// </param>
    public: LzmaCompressor(int quality = 5);

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~LzmaCompressor() override;

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
    ///   Available space in the output buffer. Will be set to the bytes actually stored
    ///   in the output buffer when the method returns
    /// </param>
    /// <returns>
    ///   The reason why the method stopped processing data. This may either be because
    ///   all available input was compressed, or because the compressor ran out of space
    ///   in the output buffer.
    /// </returns>
    public: StopReason Process(
      const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) override;

    /// <summary>Finishes compressing and writes any remaining output bytes</summary>
    /// <param name="outputBuffer">Buffer in which the compressed data will be stored</param>
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

    /// <summary>State of the LZMA encoder</summary>
    private: ::LZ_Encoder *encoder;
    /// <summary>Whether we still suspect data in the encoder's output buffer</summary>
    private: bool encoderStillHoldsOutputData;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::LZip

#endif // defined(NUCLEX_STORAGE_HAVE_LZIP)

#endif // NUCLEX_STORAGE_COMPRESSION_LZIP_LZMACOMPRESSOR_H
