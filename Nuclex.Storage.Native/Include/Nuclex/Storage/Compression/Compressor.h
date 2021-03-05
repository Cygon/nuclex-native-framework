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

#ifndef NUCLEX_STORAGE_COMPRESSION_COMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_COMPRESSOR_H

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

  /// <summary>Compresses data</summary>
  /// <remarks>
  ///   <para>
  ///     Compressors are likely to keep a lot of internal state (such as dictionaries and
  ///     streaming buffers for look-ahead), so rather than reusing compressors, you should
  ///     create a new compressor for each individual &quot;thing&quot; you want to compress.
  ///   </para>
  ///   <para>
  ///     It may also be expensive to keep compressors around (some of the high-ratio
  ///     compressors will have internal data structures measuring gigabytes), so only create
  ///     a compressor when you actually want to compress something and destroy it right
  ///     away when you're finished compressing.
  ///   </para>
  /// </remarks>
  class Compressor {

    /// <summary>Frees all resources owned by the instance</summary>
    public: NUCLEX_STORAGE_API virtual ~Compressor() = default;

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
    /// <remarks>
    ///   The compressor may keep some data in an internal buffer to use for more optimally
    ///   compressing the next chunk you provide it with. This, however, means that when
    ///   the method returns, the output is not guaranteed to be complete. You need to
    ///   call <see cref="Finish" /> as the final method before the output is guaranteed
    ///   to decompress into the complete input again.
    /// </remarks>
    public: virtual StopReason Process(
      const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) = 0;

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
    public: virtual StopReason Finish(
      std::uint8_t *outputBuffer, std::size_t &outputByteCount
    ) = 0;

#if defined(NUCLEX_STORAGE_COMPRESSION_CONVENIENCE_METHODS)
    /// <summary>Compresses the provided buffer</summary>
    /// <param name="uncompressedBuffer">Buffer holding the uncompressed data</param>
    /// <param name="uncompressedByteCount">Number of bytes that will be compressed</param>
    /// <param name="outputStream">Stream into which the compressed data will be written</param>
    /// <returns>
    ///   The reason why the method stopped processing data. This may either be because
    ///   all available input was compressed, or because the compressor ran out of space
    ///   in the output buffer.
    /// </returns>
    /// <remarks>
    ///   <para>
    ///     This variant of the compress method will compress all bytes in the input
    ///     buffer and write the compressed data into the output stream until all input
    ///     has been processed.
    ///   </para>
    ///   <para>
    ///     The compressor may keep some data in an internal buffer to use for more optimally
    ///     compressing the next chunk you provide it with. This, however, means that when
    ///     the method returns, the output is not guaranteed to be complete. You need to
    ///     call <see cref="Finish" /> as the final method before the output is guaranteed
    ///     to decompress into the complete input again.
    ///   </para>
    /// </remarks>
    public: virtual StopReason Process(
      const std::uint8_t *uncompressedBuffer, std::size_t uncompressedByteCount,
      Binary::OutputStream &outputStream
    );
#endif

    /// <summary>Compresses an input stream up to its end</summary>
    /// <param name="inputStream">Stream from which the uncompressed data will be taken</param>
    /// <param name="outputStream">Stream into which the compressed data will be written</param>
    /// <remarks>
    ///   <para>
    ///     This variant of the compress method will compress all data provided by the input
    ///     stream and write the compressed data into the output stream until all input has
    ///     been processed. You still need to call <see cref="Finish" /> after this.
    ///   </para>
    ///   <para>
    ///     It can be overridden by compression algorithms thatare 
    ///   </para>
    /// </remarks>
    public: virtual void Process(
      Binary::InputStream &inputStream, Binary::OutputStream &outputStream
    );

    /// <summary>Finishes compressing and writes any remaining output bytes</summary>
    /// <param name="outputStream">
    ///   Stream into which the remaining output will be written
    /// </param>
    public: virtual void Finish(Binary::OutputStream &outputStream);

    //private:: void processStream(std::uint8_t *inputBuffer, )
    //private: void finishWithOutputStreamBuffer(Binary::OutputStream &outputStream);
    //private: void finishWithOwnBuffer(std::uint8_t &buffer, )

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression

#endif // NUCLEX_STORAGE_COMPRESSION_COMPRESSOR_H
