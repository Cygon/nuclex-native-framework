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

#include "LzmaCompressor.h"

#if defined(NUCLEX_STORAGE_HAVE_LZIP)

#include "Nuclex/Storage/Errors/CompressionError.h"

#include <cassert> // for assert()
#include <new> // for std::nothrow
#include <limits> // for std::numeric_limits
#include <algorithm>
#include <cmath>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the closest power of two for the specified number</summary>
  /// <param name="n">Number of which the closest power of two will be calculated</param>
  /// <returns>The closest power of two to the specified number</returns>
  int getNearestPowerOfTwo(int n) {
    int v = n;

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++; // next power of 2

    int x = v >> 1; // previous power of 2

    return (v - n) > (n - x) ? x : v;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the dictionary size to use depending on the compression level
  /// </summary>
  /// <param name="level">Compression level from 0 to 9</param>
  /// <returns>The dictionary size for the LZMA encoder</returns>
  int getDictionarySizeFromCompressionLevel(int level) {
    if(level < 1) {
      return 65535; // Special value to select 'fast' encoder
    } else {
      level = std::min(level - 1, 8); // 0 .. 8

      int minimumDictionarySize = ::LZ_min_dictionary_size();
      int maximumDictionarySize = ::LZ_max_dictionary_size();

      // For the lowest and the highest level, pick the limits and do not
      // round to the closest power of 2
      if(level == 0) {
        return minimumDictionarySize;
      } else if(level == 8) {
        return maximumDictionarySize;
      }

      // For the levels inbetween, interpolate (and use an exponential curve
      // because using a low compression level and having to allocate 256 megabytes
      // of RAM is kinda not what we want).
      int interpolated;
      {
        double highest = std::pow(2, static_cast<double>(8));
        double selected = std::pow(2, static_cast<double>(level));
        double factor = selected / highest;

        interpolated = static_cast<int>(
          (maximumDictionarySize - minimumDictionarySize) * factor
        ) + minimumDictionarySize;
      }

      return getNearestPowerOfTwo(interpolated);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Determines the match length limit to use depending on the compression level
  /// </summary>
  /// <param name="level">Compression level from 0 to 9</param>
  /// <returns>The match length limit for the LZMA encoder</returns>
  int getMatchLenghtLimitFromCompressionLevel(int level) {
    if(level < 1) {
      return 16;
    } else {
      level = std::min(level - 1, 8); // 0 .. 8

      int minimumLengthLimit = ::LZ_min_match_len_limit();
      int maximumLengthLimit = ::LZ_max_match_len_limit();
      return (
        (maximumLengthLimit - minimumLengthLimit) * level / 8 + minimumLengthLimit
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression { namespace LZip {

  // ------------------------------------------------------------------------------------------- //

  LzmaCompressor::LzmaCompressor(int quality) :
    encoder(
      ::LZ_compress_open(
        getDictionarySizeFromCompressionLevel(quality),
        getMatchLenghtLimitFromCompressionLevel(quality),
        std::numeric_limits<std::size_t>::max()
      )
    ),
    encoderStillHoldsOutputData(false) {

    if(this->encoder == nullptr) {
      throw std::bad_alloc();
      //throw std::runtime_error(u8"Error allocating memory for LZip encoder");
    } else if(::LZ_compress_errno(this->encoder) != LZ_ok) {
      throw std::logic_error(u8"Error setting up LZip encoder");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  LzmaCompressor::~LzmaCompressor() {
    int result = ::LZ_compress_close(this->encoder);
    assert((result == 0) && u8"LZip encoder was destroyed");
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason LzmaCompressor::Process(
    const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    std::size_t remainingOutputByteCount = outputByteCount;
    for(;;) {

      // Skip feeding the decoder if we still suspect something in its output buffer.
      // This is done to ensure we're not accumulating tons of data in the output
      // buffer when someone loops the Process() method over a large file.
      if(!this->encoderStillHoldsOutputData) {
        if(uncompressedByteCount == 0) {
          outputByteCount -= remainingOutputByteCount;
          return StopReason::InputBufferExhausted;
        }

        // Check the available space in the compressor's write buffer and fill it up
        // as much as either the available data or the buffer space allows
        std::size_t fittingByteCount = static_cast<std::size_t>(
          ::LZ_compress_write_size(this->encoder)
        );
        if(fittingByteCount > 0) {

          int writtenByteCount = ::LZ_compress_write(
            this->encoder, uncompressedBuffer,
            static_cast<int>(std::min(uncompressedByteCount, fittingByteCount))
          );
          if(writtenByteCount < 0) {
            throw Errors::CompressionError(
              u8"LZMA encoder reported an error adding new data to tbe buffer"
            );
          }

          uncompressedByteCount -= writtenByteCount;
          uncompressedBuffer += writtenByteCount;
        }
      }

      // Now read the compressed data back out of the LZip implementation's buffers 
      int producedByteCount = ::LZ_compress_read(
        this->encoder, outputBuffer, static_cast<int>(remainingOutputByteCount)
      );
      if(producedByteCount < 0) {
        throw Errors::CompressionError(
          u8"LZMA encoder reported an error delivering compressed data"
        );
      }
      remainingOutputByteCount -= producedByteCount;

      if(remainingOutputByteCount == 0) {
        outputByteCount -= remainingOutputByteCount;
        this->encoderStillHoldsOutputData = true;
        return StopReason::OutputBufferFull;
      } else {
        this->encoderStillHoldsOutputData = false;
      }

      outputBuffer += producedByteCount;

    }
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason LzmaCompressor::Finish(
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    int finishedResult = ::LZ_compress_finished(this->encoder);
    if(finishedResult != 1) {
      int result = ::LZ_compress_finish(this->encoder);
      if(result != 0) {
        throw Errors::CompressionError(u8"LZMA encoder reported an error finishing compression");
      }
    }

    int producedByteCount = ::LZ_compress_read(
      this->encoder, outputBuffer, static_cast<int>(outputByteCount)
    );
    if(producedByteCount < 0) {
      throw Errors::CompressionError(
        u8"LZMA encoder reported an error delivering compressed data"
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
