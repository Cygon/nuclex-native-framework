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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_STORAGE_SOURCE 1

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include "CscCompressor.h"

#include "Nuclex/Storage/Errors/CompressionError.h"

#include <algorithm> // for std::min() and std::max()
#include <cassert> // for assert()

// CSC includes, through 7Zip's types.h, the terrible Windows header,
// so at least try to prevent it from messing up our whole environment...
#include "../../Helpers/WindowsApi.h"

#include <csc_enc.h> // for CSCEncProps_Init() etc.

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  CscCompressor::CscCompressor(int level) :
    allocator(),
    writeBuffer(),
    outputStream(writeBuffer),
    encoderProperties(),
    encoder(),
    outputHandler(),
    inputBuffer(nullptr),
    inputBufferByteCount(0) {

    // Set up the encoder properties according to the selected compression level
    static const std::uint32_t kiloBytes = 1024;
    static const std::uint32_t megaBytes = 1024 * kiloBytes;
    static const std::uint32_t dictionarySizes[4] = {
      32 * kiloBytes - 10 * kiloBytes,
      1 * megaBytes - 10 * kiloBytes,
      16 * megaBytes - 10 * kiloBytes,
      1024 * megaBytes - 10 * kiloBytes
    };
    ::CSCEncProps_Init(&this->encoderProperties, dictionarySizes[level], level);

    // The encoder properties need to be written to the stream so they can be handed
    // to the decoder on decompression. This API is a bit weird, by looking at the code
    // we know it's writing exactly 10 bytes. It also has a dummy parameter.
    {
      const int unusedParameter = 123456789;

      std::uint8_t propertyMemory[CSC_PROP_SIZE];
      ::CSCEnc_WriteProperties(&this->encoderProperties, propertyMemory, unusedParameter);

      this->writeBuffer.Write(propertyMemory, CSC_PROP_SIZE);
    }

    // With the encoder properties set up, create an encoder
    this->outputHandler.Init(
      &this->outputStream, this->encoderProperties.csc_blocksize, &this->allocator
    );

    this->encoder.Init(&this->encoderProperties, &this->outputHandler, &this->allocator);

    this->inputBuffer = new std::uint8_t[this->encoderProperties.raw_blocksize];
  }

  // ------------------------------------------------------------------------------------------- //

  CscCompressor::~CscCompressor() {
    if(this->inputBuffer != nullptr) {
      delete[] this->inputBuffer;
    }

    this->encoder.Destroy();
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason CscCompressor::Process(
    const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {

    // Because we cannot stop the CSC compressor until it has processed all input
    // bytes, it may generate more output than we want. This will have been saved by
    // our special buffer writer. If there's still output waiting in the buffer writer,
    // it'll be written to the output buffer here (potentially even filling the new
    // output buffer before calling into the CSC library)
    std::size_t overflowedByteCount = this->writeBuffer.UseFixedBuffer(
      outputBuffer, outputByteCount
    );
    if(overflowedByteCount >= outputByteCount) {
      return StopReason::OutputBufferFull; // outputByteCount can remain as-is
    }

    for(;;) {

      // If all input provided to this call can be eaten up by the input buffer,
      // just do so (this is the most likely case as block size is typically huge)
      {
        std::size_t totalInputByteCount = this->inputBufferByteCount + uncompressedByteCount;
        if(totalInputByteCount < this->encoderProperties.raw_blocksize) {
          std::copy_n(
            uncompressedBuffer,
            uncompressedByteCount,
            this->inputBuffer + this->inputBufferByteCount
          );
          this->inputBufferByteCount = totalInputByteCount;
          uncompressedByteCount = 0; // No more input remains
          outputByteCount -= this->writeBuffer.CountRemainingBytes();
          return StopReason::InputBufferExhausted;
        }
      }

      {
        std::size_t usableInputByteCount = std::min(
          this->encoderProperties.raw_blocksize - this->inputBufferByteCount,
          uncompressedByteCount
        );
        std::copy_n(
          uncompressedBuffer,
          usableInputByteCount,
          this->inputBuffer + this->inputBufferByteCount
        );
        uncompressedByteCount -= usableInputByteCount;
        uncompressedBuffer += usableInputByteCount;
        // We don't update inputBufferByteCount because it can only match block size now
      }

      // There's space for more data. Let the CSC encoder compress as much input as it can.
      try {
        this->encoder.Compress(
          const_cast<std::uint8_t *>(this->inputBuffer), this->encoderProperties.raw_blocksize
        );
      }
      catch(int errorCode) {
        throwErrorForThrownInteger(errorCode);
        throw std::logic_error(u8"throwErrorForThrownInteger() did not throw");
      }
      
      std::size_t remainingOutputByteCount = this->writeBuffer.CountRemainingBytes();
      if(remainingOutputByteCount == 0) {
        this->inputBufferByteCount = 0; // Start over
        return StopReason::OutputBufferFull;
      }

    } // for(;;)
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason CscCompressor::Finish(
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {

    // Because we cannot stop the ZPaq compressor until it has processed all input
    // bytes, it may generate more output than we want. This will have been saved by
    // our special buffer writer. If there's still output waiting in the buffer writer,
    // it'll be written to the output buffer here
    std::size_t overflowedByteCount = this->writeBuffer.UseFixedBuffer(
      outputBuffer, outputByteCount
    );
    if(overflowedByteCount >= outputByteCount) {
      return StopReason::OutputBufferFull; // outputByteCount can remain as-is
    }

    try {
      // If the processed data was not a multiple of the block size, compress one final
      // block that is less than the block size
      if(this->inputBufferByteCount >= 1) {
        this->encoder.Compress(
          const_cast<std::uint8_t *>(this->inputBuffer),
          static_cast<std::uint32_t>(this->inputBufferByteCount)
        );
      }

      // And flush the encoder to ensure our output stream is complete
      this->encoder.Flush();
    }
    catch(int errorCode) {
      throwErrorForThrownInteger(errorCode);
      throw std::logic_error(u8"throwErrorForThrownInteger() did not throw");
    }

    // Was enough data generated to fill the output buffer?
    if(this->writeBuffer.HasOverflowBytes()) {
      return StopReason::OutputBufferFull;
    } else {
      outputByteCount -= this->writeBuffer.CountRemainingBytes();
      return StopReason::Finished;
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)
