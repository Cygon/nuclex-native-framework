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

#include "LzmaDecompressor.h"

#if defined(NUCLEX_STORAGE_HAVE_LZIP)

#include "Nuclex/Storage/Errors/CompressionError.h"

#include <algorithm> // for std::min and std::max
#include <new> // for std::nothrow
#include <limits> // for std::numeric_limits
#include <cassert> // for assert()

namespace Nuclex { namespace Storage { namespace Compression { namespace LZip {

  // ------------------------------------------------------------------------------------------- //

  LzmaDecompressor::LzmaDecompressor() :
    decoder(::LZ_decompress_open()),
    decoderStillHoldsOutputData(false) {}

  // ------------------------------------------------------------------------------------------- //

  LzmaDecompressor::~LzmaDecompressor() {
    int result = ::LZ_decompress_close(this->decoder);
    assert((result == 0) && u8"LZip decoder was destroyed");
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason LzmaDecompressor::Process(
    const std::uint8_t *compressedBuffer, std::size_t &compressedByteCount,
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    std::size_t remainingOutputByteCount = outputByteCount;
    for(;;) {

      // Skip feeding the decoder if we still suspect something in its output buffer.
      // This is done to ensure we're not accumulating tons of data in the output
      // buffer when someone loops the Process() method over a large file.
      if(!this->decoderStillHoldsOutputData) {
        if(compressedByteCount == 0) {
          outputByteCount -= remainingOutputByteCount;
          return StopReason::InputBufferExhausted;
        }

        // Check the available space in the decompressor's write buffer and fill it up
        // as much as either the available data or the buffer space allows
        std::size_t fittingByteCount = static_cast<std::size_t>(
          ::LZ_decompress_write_size(this->decoder)
        );
        if(fittingByteCount > 0) {
          int writtenByteCount = ::LZ_decompress_write(
            this->decoder, compressedBuffer,
            static_cast<int>(std::min(compressedByteCount, fittingByteCount))
          );
          if(writtenByteCount < 0) {
            throw Errors::CompressionError(
              u8"LZMA decoder reported an error adding new data to tbe buffer"
            );
          }

          compressedByteCount -= writtenByteCount;
          compressedBuffer += writtenByteCount;
        }
      }

      // Now read the compressed data back out of the 
      int producedByteCount = ::LZ_decompress_read(
        this->decoder, outputBuffer, static_cast<int>(remainingOutputByteCount)
      );
      if(producedByteCount < 0) {
        throw Errors::CompressionError(
          u8"LZMA decoder reported an error delivering decompressed data"
        );
      }
      remainingOutputByteCount -= producedByteCount;

      if(remainingOutputByteCount == 0) {
        outputByteCount -= remainingOutputByteCount;
        this->decoderStillHoldsOutputData = true;
        return StopReason::OutputBufferFull;
      } else {
        this->decoderStillHoldsOutputData = false;
      }

      outputBuffer += producedByteCount;

    }
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason LzmaDecompressor::Finish(
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    int finishedResult = ::LZ_decompress_finished(this->decoder);
    if(finishedResult != 1) {
      int result = ::LZ_decompress_finish(this->decoder);
      if(result != 0) {
        throw Errors::CompressionError(u8"LZMA decoder reported an error finishing compression");
      }
    }

    int producedByteCount = ::LZ_decompress_read(
      this->decoder, outputBuffer, static_cast<int>(outputByteCount)
    );
    if(producedByteCount < 0) {
      throw Errors::CompressionError(
        u8"LZMA decoder reported an error delivering compressed data"
      );
    }

    // How to differentiate between an exact hit of the output buffer size
    // and more data actually following?
    if(static_cast<std::size_t>(producedByteCount) == outputByteCount) {
      return StopReason::OutputBufferFull;
    } else {
      outputByteCount = producedByteCount;
      return StopReason::Finished;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::LZip

#endif // defined(NUCLEX_STORAGE_HAVE_LZIP)
