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

#include "DeflateCompressor.h"

#if defined(NUCLEX_STORAGE_HAVE_ZLIB)

#include "Nuclex/Storage/Errors/CompressionError.h"

#include "ZLibHelper.h"

#include <cassert> // for assert()
#include <new> // for std::nothrow
#include <limits> // for std::numeric_limits

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Allocates memory for ZLib</summary>
  /// <param name="itemCount">Number of items that will be allocated</param>
  /// <param name="itemSize">Size of an individual item</param>
  /// <returns>
  ///   The address of the first item in a contiguous block large enough to hold
  ///   all requested items; or NULL in case the memory could not be allocated
  /// </returns>
  void *allocateZLibMemory(void *, uInt itemCount, uInt itemSize) {
    return new(std::nothrow) std::uint8_t[itemCount * itemSize];
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Frees memory previously allocated via the allocate() method</summary>
  /// <param name="memory">Adrress of a memory block return by allocate()</summary>
  void freeZLibMemory(void *, void *memory) {
    delete []reinterpret_cast<std::uint8_t *>(memory);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression { namespace ZLib {

  // ------------------------------------------------------------------------------------------- //

  DeflateCompressor::DeflateCompressor(int level) {

    // Try to initialize a ZLib deflate stream. With Z_SOLO we need to provide
    // custom allocation functions or ZLib will fail. We do so in any case because
    // we want ZLib to work with and without Z_SOLO.
    this->stream.msg = nullptr;
    this->stream.state = nullptr;
    this->stream.zalloc = &allocateZLibMemory;
    this->stream.zfree = &freeZLibMemory;
    this->stream.data_type = Z_BINARY;

    // Initialize the deflate stream encoder
    {
      // This is the code behind ::deflateInit() (i.e. same defaults) except with
      // the window bits negative, which sets this->stream.wrap to '0', leaving out
      // the trailer (containing an adler32 checksum and/or uncompressed length)
      int result = ::deflateInit2(
        &this->stream, level, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY
      );
      if(result != Z_OK) {
        std::string errorMessage(u8"Could not initialize ZLib deflate stream. ");
        errorMessage.append(ZLibHelper::GetErrorMessage(this->stream, result));
        throw std::logic_error(errorMessage);
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  DeflateCompressor::~DeflateCompressor() {

    // Shut down the deflate stream encoder. This will release any memory allocated
    // by ZLib, whch should be possible at any time.
    int result = ::deflateEnd(&this->stream);
    assert(
      ((result == Z_OK) || (result == Z_DATA_ERROR)) &&
      u8"Deflate must either finish normally or be cancelled along the way"
    );

    // CHECK: What does Z_DATA_ERROR indicate here and when does it pop up?

  }

  // ------------------------------------------------------------------------------------------- //

  StopReason DeflateCompressor::Process(
    const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {

    // ZLib API docs state that at least one action (compress more or deliver
    // pending output) should be possible when calling deflate(), so we make
    // sure our caller has provided input and some space in the output buffer.
    if(outputByteCount == 0) {
      return StopReason::OutputBufferFull;
    }

    // This check could be disabled, allowing the caller to 'poll' for more output
    // without providing input. However, we assume ZLib has internal buffer limits,
    // so that, if too much pending output accumulates, it will simply not take
    // in more input.
    if(uncompressedByteCount == 0) {
      return StopReason::InputBufferExhausted;
    }

    // Verification for the unlikely case that someone goes overboard on 64 bit
    // platforms and tries to compress >4 GiB of data (:
    assert(
      (uncompressedByteCount < std::numeric_limits<uInt>::max()) &&
      u8"Uncompressed input buffer size fits within the limitations of the ZLib API"
    );
    assert(
      (outputByteCount < std::numeric_limits<uInt>::max()) &&
      u8"Output buffer size fits within the limitations of the ZLib API"
    );

    this->stream.next_in = uncompressedBuffer;
    this->stream.avail_in = static_cast<uInt>(uncompressedByteCount);
    this->stream.next_out = outputBuffer;
    this->stream.avail_out = static_cast<uInt>(outputByteCount);

    // Let ZLib perform the compression. It will first try to fill a compression
    // window (so the first few calls likely produce no output depending on
    // how much data you feed it). Once enough compressible data is available,
    // each call generates as much output as becomes available as the compression
    // window slides through the input data.
    int result = ::deflate(&this->stream, Z_NO_FLUSH);
    if((result != Z_OK) && (result != Z_STREAM_END)) {
      std::string errorMessage(u8"ZLib deflate compressor failed to compress data. ");
      errorMessage.append(ZLibHelper::GetErrorMessage(this->stream, result));
      throw Errors::CompressionError(errorMessage);
    }

    // Update the in/out parameters for the caller's buffer accounting
    uncompressedByteCount = this->stream.avail_in; // input = remaining bytes, counts down
    outputByteCount -= this->stream.avail_out; // output = produced bytes, counts up

    // Provide the caller with the reason the call completed
    if(this->stream.avail_out == 0) {
      return StopReason::OutputBufferFull;
    } else {
      // Sanity check. ZLib promises to fill the output buffer to the last byte if it
      // stops due to running out of space. So if it did /not/ run out of space,
      // it should have consumed all input there is.
      assert(
        (this->stream.avail_in == 0) && u8"ZLib processed all input that was available"
      );
      return StopReason::InputBufferExhausted;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  StopReason DeflateCompressor::Finish(
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {

    // Calling this without providing space in the output buffer makes no sense
    if(outputByteCount == 0) {
      return StopReason::OutputBufferFull;
    }

    // Once again, protect against jokers wanting to compress >4 GiB in one call
    // on a 64 bit platform with 64 bit size_t (:
    assert(
      (outputByteCount < std::numeric_limits<uInt>::max()) &&
      u8"Output buffer size fits within the limitations of the ZLib API"
    );

    this->stream.next_in = nullptr;
    this->stream.avail_in = 0;
    this->stream.next_out = outputBuffer;
    this->stream.avail_out = static_cast<uInt>(outputByteCount);

    // Empty the deflate stream's pending output as much as fits into our output buffer
    int result = ::deflate(&this->stream, Z_FINISH);
    if((result != Z_OK) && (result != Z_STREAM_END)) {
      std::string errorMessage(u8"ZLib deflate compressor failed to compress data. ");
      errorMessage.append(ZLibHelper::GetErrorMessage(this->stream, result));
      throw Errors::CompressionError(errorMessage);
    }

    // Update the in/out parameter for the caller's buffer accounting
    outputByteCount -= this->stream.avail_out;

    // Provide the caller with the reason the call completed
    if(this->stream.avail_out == 0) {
      return StopReason::OutputBufferFull;
    } else {
      // Docs state that when deflate() stops due to a full output buffer, not even
      // one byte will be left over. So at this point, ZLib should have written any
      // stream 'end' markers and flushed all pending output.
      assert(
        (result == Z_STREAM_END) && u8"ZLib reported that compression is complete"
      );

      // Should always result in StopReason::Finished. Still, just in case, if ZLib does
      // not return Z_STREAM_END, we signal the caller that we want to go another round.
      return (result == Z_STREAM_END) ? StopReason::Finished : StopReason::OutputBufferFull;
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::ZLib

#endif // defined(NUCLEX_STORAGE_HAVE_ZLIB)
