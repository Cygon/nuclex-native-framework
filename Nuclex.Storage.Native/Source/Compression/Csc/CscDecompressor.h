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

#ifndef NUCLEX_STORAGE_COMPRESSION_CSC_CSCDECOMPRESSOR_H
#define NUCLEX_STORAGE_COMPRESSION_CSC_CSCDECOMPRESSOR_H

#include "Nuclex/Storage/Config.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include "Nuclex/Storage/Compression/Decompressor.h"
#include "../../Helpers/ReadBuffer.h"

#include "CscHelper.h"

// Prototype for internal implementation from csc_dec.cpp
class CSCDecoder;

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Decompresses data that has been compressed by the CSC library and algorithm
  /// </summary>
  class CscDecompressor : public Decompressor {

    /// <summary>Initializes a new CSC decompressor</summary>
    public: CscDecompressor();

    /// <summary>Frees all resources owned by the instance</summary>
    public: ~CscDecompressor() override;

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

    /// <summary>Initializes the CSC properties, decoder and output buffer</summary>
    private: void initializeCscDecoder();

    /// <summary>Initializes the CSC properties, decoder and output buffer</summary>
    /// <param name="outputBuffer">Buffer into which pending output is copied</param>
    /// <param name="outputByteCount">
    ///   Maximum number of bytes to place in the output buffer
    /// </apram>
    /// <returns>The number of bytes actually placed in the output buffer</returns>
    private: std::size_t extractPendingOutput(
      std::uint8_t *outputBuffer, std::size_t outputByteCount
    );

    #pragma region class SequentialInputStream

    private: class SequentialInputStream : public ISeqInStream {
      
      /// <summary>Initializes a new buffered output stream</summary>
      /// <param name="writer">
      ///   Split buffer writer into which all written data will be directed
      /// </param>
      public: SequentialInputStream(Nuclex::Storage::Helpers::ReadBuffer &buffer) :
        buffer(buffer) {
        this->Read = &read;
      }

      /// <summary>Method that is registered to libcsc as the write function pointer</summary>
      /// <param name="self">Pointer to the output stream</param>
      /// <param name="buffer">Buffer holding the data that will be written</param>
      /// <param name="byteCount">Number of bytes that will be written</param>
      /// <returns>The number of bytes actually written</returns>
      private: static SRes read(void *self, void *buffer, size_t *byteCount) {
        SequentialInputStream &selfAsStream = *reinterpret_cast<SequentialInputStream *>(self);

        std::size_t availableByteCount = selfAsStream.buffer.CountAvailableBytes();
        if(availableByteCount < *byteCount) {
          *byteCount = availableByteCount;
        }

        selfAsStream.buffer.Read(reinterpret_cast<std::uint8_t *>(buffer), *byteCount);

        return SZ_OK;
      }

      /// <summary>Buffer from which all input data is taken</summary>
      private: Nuclex::Storage::Helpers::ReadBuffer &buffer;

    };

    #pragma endregion // class SequentialInputStream

    /// <summary>Allocator that allows libcsc to reuse memory</summary>
    private: ReusingAllocator allocator;
  
    /// <summary>Properties the CSC encoder has been configured with</summary>
    private: ::CSCProps encoderProperties; // uninitialized if (decoderHandle == nullptr)
  
    /// <summary>Whether the CSC decoder has been initialized</summary>
    private: bool decoderInitialized;
    /// <summary>Maintains the CSC decoder's state</summary>
    private: ::CSCDecHandle decoderHandle;
    /// <summary>The CSC decoder instance fished out of the decoder handle</summary>
    private: ::CSCDecoder *decoder;
    /// <summary>Receives the decoder properties</summary>
    private: std::uint8_t decoderPropertyMemory[10];
    /// <summary>Number of bytes currently copied into the decoder property memory</summary>
    private: std::size_t decoderPropertyMemoryByteCount;

    /// <summary>Buffers input for when we need to accumulate enough bytes</summary>
    private: Nuclex::Storage::Helpers::ReadBuffer inputBuffer;
    /// <summary>Input stream from which data is fed to the CSC decompressor</summary>
    private: SequentialInputStream inputStream;

    /// <summary>Separate output buffer because we have no control over extraction</summary>
    private: std::uint8_t *outputBuffer;
    /// <summary>Index at which the next read takes data from the output buffer</summary>
    private: std::size_t outputBufferStartIndex;
    /// <summary>Index one past the last byte currently stored in the output buffer</summary>
    private: std::size_t outputBufferEndIndex;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)

#endif // NUCLEX_STORAGE_COMPRESSION_CSC_CSCDECOMPRESSOR_H
