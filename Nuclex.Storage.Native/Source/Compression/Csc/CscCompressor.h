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

#ifndef NUCLEX_STORAGE_COMPRESSION_CSC_CSCCOMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_CSC_CSCCOMPRESSOR_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include "Nuclex/Storage/Compression/Compressor.h"

#include "../../Helpers/WriteBuffer.h"
#include "CscHelper.h"

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Compresses data using the CSC library and algorithm</summary>
  class CscCompressor : public Compressor {

    /// <summary>Initializes a new CSC compressor</summary>
    /// <param name="level">
    ///   Compression level that will be passed to the CSC compressor
    /// </param>
    public: CscCompressor(int level = 2);

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~CscCompressor() override;

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

    #pragma region class BufferedSequentialOuputStream

    /// <summary>Implements a libcsc output stream into the split buffer writer>/summary>
    private: class BufferedSequentialOuputStream : public ISeqOutStream {

      /// <summary>Initializes a new buffered output stream</summary>
      /// <param name="writer">
      ///   Split buffer writer into which all written data will be directed
      /// </param>
      public: BufferedSequentialOuputStream(Nuclex::Storage::Helpers::WriteBuffer &writeBuffer) :
        writeBuffer(writeBuffer) {
        this->Write = &write;
      }

      /// <summary>Method that is registered to libcsc as the write function pointer</summary>
      /// <param name="self">Pointer to the output stream</param>
      /// <param name="buffer">Buffer holding the data that will be written</param>
      /// <param name="byteCount">Number of bytes that will be written</param>
      /// <returns>The number of bytes actually written</returns>
      private: static size_t write(void *self, const void *buffer, size_t byteCount) {
        reinterpret_cast<BufferedSequentialOuputStream *>(self)->writeBuffer.Write(
          reinterpret_cast<const std::uint8_t *>(buffer), byteCount
        );
        return byteCount;
      }

      /// <summary>Split buffer writer into which all written data is directed</summary>
      private: Nuclex::Storage::Helpers::WriteBuffer &writeBuffer;

    };

    #pragma endregion // class BufferedSequentialOuputStream

    /// <summary>Allocator that allows libcsc to reuse memory</summary>
    private: ReusingAllocator allocator;
    /// <summary>Writes to the output buffer and holds onto data that doesn't fit</summary>
    private: Nuclex::Storage::Helpers::WriteBuffer writeBuffer;
    /// <summary>Holds a write function pointer and a pointer to the output buffer</summary>
    private: BufferedSequentialOuputStream outputStream;

    /// <summary>Properties the CSC encoder has been configured with</summary>
    private: ::CSCProps encoderProperties;
    /// <summary>Maintains the CSC encoder's state</summary>
    private: ::CSCEncoder encoder;
    /// <summary>IO handler for output data generated by the encoder</summary>
    private: ::MemIO outputHandler;

    /// <summary>Buffer storing data to be compressed</summary>
    /// <remarks>
    ///   CSC doesn't do its own buffering and flushes the encoder after each Compress()
    ///   call, so compression ratio would be dependent on how you feed the compressor.
    ///   Thus, we collect data in this buffer until we have a full block before compressing.
    /// </remarks>
    private: std::uint8_t *inputBuffer;
    /// <summary>Number of bytes currently stored in the input buffer</summary>
    private: std::size_t inputBufferByteCount;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)

#endif // NUCLEX_STORAGE_COMPRESSION_CSC_CSCCOMPRESSOR_H
