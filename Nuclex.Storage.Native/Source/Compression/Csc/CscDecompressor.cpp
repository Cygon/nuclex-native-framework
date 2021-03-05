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

#include "CscDecompressor.h"

#if defined(NUCLEX_STORAGE_HAVE_CSC)

#include "Nuclex/Storage/Errors/CompressionError.h"

// CSC includes, through 7Zip's types.h, the terrible Windows header,
// so at least try to prevent it from messing up our whole environment...
#include "../../Helpers/WindowsApi.h"

#include <csc_dec.h> // for CSCDec_ReadProperties(), CSCDec_Create(), etc.

// --------------------------------------------------------------------------------------------- //

/// <summary>CSCDecoder class from csc_dec.cpp</summary>
/// <remarks>
///   <para>
///     This terrible, no-good cut & paste of the private implementation class from CSC
///     is needed because the public API only exposes functions that compress whole files
///     in one uninterrupted call.
///   </para>
///   <para>
///     I don't want to be responsible for maintaining a fork (or patch on top) of
///     libcsc to expose these methods in the public API. So the next best thing to do
///     was to declare the whole class here via copy & paste from csc_dec.cpp. If you
///     get linker errors, then libcsc has changed and you need to repeat the copy & paste.
///   </para>
/// </remarks>
class CSCDecoder {
  private: uint32_t coder_decode_direct(uint32_t len);
  private: uint32_t decode_int();
  private: int decode_bad(uint8_t *dst, uint32_t *size, uint32_t max_bsize);
  private: int decode_rle(uint8_t *dst, uint32_t *size, uint32_t max_bsize);
  private: uint32_t decode_literal();
  private: int decode_literals(uint8_t *dst, uint32_t *size, uint32_t max_bsize);
  private: uint32_t decode_matchlen_1();
  private: uint32_t decode_matchlen_2();
  private: void decode_match(uint32_t &dist, uint32_t &len);
  private: void set_lit_ctx(uint32_t c);
  private: void decode_1byte_match(void);
  private: void decode_repdist_match(uint32_t &rep_idx, uint32_t &match_len);
  private: int lz_decode(uint8_t *dst, uint32_t *size, uint32_t limit);
  private: void lz_copy2dict(uint8_t *src, uint32_t size);
  public: int Init(MemIO *io, uint32_t dict_size, uint32_t csc_blocksize, ISzAlloc *alloc);
  public: void Destroy();
  public: int Decompress(uint8_t *dst, uint32_t *size, uint32_t max_bsize);
  public: uint64_t GetCompressedSize();
  private: ISzAlloc *alloc_;
  private: MemIO *io_;
  private: Filters *filters_;
  private: uint8_t *rc_buf_;
  private: uint8_t *bc_buf_;
  private: uint32_t rc_bufsize_;
  private: uint32_t bc_bufsize_;
  private: uint32_t m_op;
  private: uint64_t rc_low_, rc_cachesize_;
  private: uint32_t rc_range_, rc_code_;
  private: uint8_t rc_cache_;
  private: uint32_t bc_curbits_;
  private: uint32_t bc_curval_;
  private: uint8_t *prc_;
  private: uint8_t *pbc_;
  private: uint32_t bc_size_;
  private: uint32_t rc_size_;
  private: int64_t outsize_;
  private: uint32_t p_rle_flag_;
  private: uint32_t p_rle_len_[16];
  private: uint32_t *p_lit_;
  private: uint32_t *p_delta_;
  private: uint32_t p_repdist_[64 * 4];
  private: uint32_t p_dist_[8 + 16 * 2 + 32 * 4];
  private: uint32_t p_matchdist_extra_[29 * 16];
  private: uint32_t p_matchlen_slot_[2];
  private: uint32_t p_matchlen_extra1_[8];
  private: uint32_t p_matchlen_extra2_[8];
  private: uint32_t p_matchlen_extra3_[128];
  private: uint32_t p_longlen_;
  private: uint32_t ctx_;
  private: uint32_t p_state_[4 * 4 * 4 * 3];
  private: uint32_t state_;
  private: uint32_t rep_dist_[4];
  private: uint32_t wnd_size_;
  private: uint8_t  *wnd_;
  private: uint32_t wnd_curpos_;
};

// --------------------------------------------------------------------------------------------- //

/// <summary>Structure taken from csc_encoder_main</summary>
/// <remarks>
///   This is what hides behind the CSCDecHandle type. We need to access it
///   because the public CSCDec_ API would decompress everything in one go
///   rather than allow us to feed it small buckets of data.
/// </remarks>
struct CSCDecInstance {
    CSCDecoder *decoder;
    MemIO *io;
    ISzAlloc *alloc;
    uint32_t raw_blocksize;
};

// ------------------------------------------------------------------------------------------- //

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  CscDecompressor::CscDecompressor() :
    allocator(),
    encoderProperties(),
    decoderInitialized(false),
    decoderHandle(nullptr),
    decoder(nullptr),
    decoderPropertyMemory(),
    decoderPropertyMemoryByteCount(0),
    inputBuffer(),
    inputStream(inputBuffer),
    outputBuffer(nullptr),
    outputBufferStartIndex(0),
    outputBufferEndIndex(0) {

    // Cannot create the actual decoder here because we need at least 10 bytes to
    // read the encoder properties passed to its factory method and two full blocks
    // (two times 65536 bytes) for it to get going.
  }

  // ------------------------------------------------------------------------------------------- //

  CscDecompressor::~CscDecompressor() {
    if(this->outputBuffer != nullptr) {
      delete[] this->outputBuffer;
    }
    if(this->decoder != nullptr) {
      ::CSCDec_Destroy(this->decoderHandle);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason CscDecompressor::Process(
    const std::uint8_t *compressedBuffer, std::size_t &compressedByteCount,
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    std::size_t outputBufferCapacity = outputByteCount;

    // If data remains in the side buffer, move it into the output buffer
    if(this->outputBufferStartIndex < this->outputBufferEndIndex) {
      std::size_t byteCount = extractPendingOutput(outputBuffer, outputByteCount);
      if(byteCount == outputByteCount) {
        // Leave outputByteCount as is since we filled the whole buffer
        return StopReason::OutputBufferFull;
      }

      // Ugh... we report total output bytes and this would invalidate that...
      outputBuffer += byteCount;
      outputBufferCapacity -= byteCount;
    }

    // Did we get a chance to initialize the decoder yet? If so, verify that we've got
    // at least two full blocks of input available (the CSC decoder must never hit the end
    // of input, so we must hide from it that we don't have the full stream available!)
    if(this->decoderInitialized) {

      this->inputBuffer.UseFixedBuffer(compressedBuffer, compressedByteCount);

      std::size_t requiredByteCount = (this->encoderProperties.raw_blocksize * 2 + 2);
      if(this->inputBuffer.CountAvailableBytes() < requiredByteCount) {
        this->inputBuffer.CacheFixedBufferContents();
        compressedByteCount = 0; // All input data has been cached
        outputByteCount -= outputBufferCapacity; // output = bytes provided
        return StopReason::InputBufferExhausted;
      }

    } else { // Nope, wait for the property header and two full blocks that are needed

      // If less than 10 bytes have been provided so far, wait until we've got 10 bytes
      // together to read the decoder properties.
      if(this->decoderPropertyMemoryByteCount < CSC_PROP_SIZE) {
        while(
          (compressedByteCount > 0) && (this->decoderPropertyMemoryByteCount < CSC_PROP_SIZE)
        ) {
          this->decoderPropertyMemory[this->decoderPropertyMemoryByteCount] = *compressedBuffer;

          ++compressedBuffer;
          --compressedByteCount;
          ++this->decoderPropertyMemoryByteCount;
        }

        // Still not enough to initialize the encoder properties
        if(this->decoderPropertyMemoryByteCount < CSC_PROP_SIZE) {
          outputByteCount = 0; // We know there was no pending output data in this case
          return StopReason::InputBufferExhausted;
        }
      }

      this->inputBuffer.UseFixedBuffer(compressedBuffer, compressedByteCount);

      // Upon initialization, CSC will call its internal ReadRCData() and ReadBCData()
      // methods which will read 1 byte and then 65536 bytes each. Thus, we have to wait
      // until the decompressor is either fed 65536 bytes or Finish() is called earlier.
      {
        // Fetch the block size. This is done by CSCDec_ReadProperties(), but we don't
        // want to call that repeatedly here.
        std::size_t cscBlockSize = (
          (static_cast<std::uint32_t>(this->decoderPropertyMemory[4]) << 16) +
          (static_cast<std::uint32_t>(this->decoderPropertyMemory[5]) << 8) +
          (static_cast<std::uint32_t>(this->decoderPropertyMemory[6]) << 0)
        );

        std::size_t requiredByteCount = (cscBlockSize * 2 + 2);
        if(this->inputBuffer.CountAvailableBytes() < requiredByteCount) {
          this->inputBuffer.CacheFixedBufferContents();
          compressedByteCount = 0; // All input data has been cached
          outputByteCount = 0; // We know there was no pending output data in this case
          return StopReason::InputBufferExhausted;
        }
      }

      // We have enough data to initialize the decoder!
      initializeCscDecoder();

    }

    // At this point:
    // - The input buffer is assigned
    // - There are at least 2 blocks + 2 bytes of input data available
    // - No pending output exists

    // Let CSC decompress the current chunk
    {
      const ::CSCDecInstance &decoderInfo = *reinterpret_cast<const ::CSCDecInstance *>(
        this->decoderHandle
      );

      // Some remarks on this call: the outputByteCount parameter is only ever written to,
      // so it doesn't imit the amount of output generated. The third parameter would be
      // able to do that, but setting it lower than the block size causes segfaults.
      std::uint32_t outputByteCount32 = 0;
      try {
        int result = this->decoder->Decompress(
          this->outputBuffer, &outputByteCount32, decoderInfo.raw_blocksize
        );

        // I've done a grep of all return statements in CSC's code. There's 0 for success
        // and somewhat inconsistently -1 or 1 for failure. No error codes here.
        if(result != 0) {
          throw Errors::CompressionError(u8"Error reported by CSC decompressor");
        }
      }
      catch(int errorCode) {
        throwErrorForThrownInteger(errorCode);
        throw std::logic_error(u8"throwErrorForThrownInteger() did not throw");
      }

      this->outputBufferStartIndex = 0;
      this->outputBufferEndIndex = static_cast<std::size_t>(outputByteCount32);
    }

    this->inputBuffer.CacheFixedBufferContents();
    compressedByteCount = 0;

    // If data remains in the side buffer, move it into the output buffer
    if(this->outputBufferStartIndex < this->outputBufferEndIndex) {
      std::size_t byteCount = extractPendingOutput(outputBuffer, outputBufferCapacity);
      if(byteCount == outputBufferCapacity) {
        return StopReason::OutputBufferFull;
      } else {
        outputBufferCapacity -= byteCount;
      }
    }

    outputByteCount -= outputBufferCapacity; // Store actually written byte count
    return StopReason::Finished;
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason CscDecompressor::Finish(
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    std::size_t outputBufferCapacity = outputByteCount;

    // If the decoder isn't initialized yet, attempt to do so now. This is a likely
    // code path for compressed streams under ~128 KiB because CSC attempts to read
    // two blocks of 65536 bytes to normally initialize unless the whole stream is
    // shorter (which we only when Finish(), this method, is called).
    if(!this->decoderInitialized) {

      // If less than 10 bytes have been provided so far, wait until we've got 10 bytes
      // together to read the decoder properties.
      if(this->decoderPropertyMemoryByteCount < CSC_PROP_SIZE) {
        throw Errors::CompressionError(
          u8"Decompression not possible, not enough input data provided"
        );
      }

      // Upon initialization, CSC will call its internal ReadRCData() and ReadBCData()
      // methods which will read 1 byte and then 65536 bytes each. Thus, we have to wait
      // until the decompressor is either fed 65536 bytes or Finish() is called earlier.
      // We know that there isn't that much data coming (information we didn't have during
      // Process()), so we can now expose this fact to the CSC decoder.
      initializeCscDecoder();

    }

    // If data remains in the side buffer, move it into the output buffer
    if(this->outputBufferStartIndex < this->outputBufferEndIndex) {
      std::size_t byteCount = extractPendingOutput(outputBuffer, outputByteCount);
      if(byteCount == outputByteCount) {
        // Leave outputByteCount as is since we filled the whole buffer
        return StopReason::OutputBufferFull;
      }

      // Ugh... we report total output bytes and this would invalidate that...
      outputBuffer += byteCount;
      outputBufferCapacity -= byteCount;
    }

    // Let CSC decompress all of the remaining data
    if(this->inputBuffer.CountAvailableBytes() >= 1) {
      const ::CSCDecInstance &decoderInfo = *reinterpret_cast<const ::CSCDecInstance *>(
        this->decoderHandle
      );

      // Some remarks on this call: the outputByteCount parameter is only ever written to,
      // so it doesn't imit the amount of output generated. The third parameter would be
      // able to do that, but setting it lower than the block size causes segfaults.
      std::uint32_t outputByteCount32 = 0;
      try {
        int result = this->decoder->Decompress(
          this->outputBuffer, &outputByteCount32, decoderInfo.raw_blocksize
        );

        // I've done a grep of all return statements in CSC's code. There's 0 for success
        // and somewhat inconsistently -1 or 1 for failure. No error codes here.
        if(result != 0) {
          throw Errors::CompressionError(u8"Error reported by CSC decompressor");
        }
      }
      catch(int errorCode) {
        throwErrorForThrownInteger(errorCode);
        throw std::logic_error(u8"throwErrorForThrownInteger() did not throw");
      }

      this->outputBufferStartIndex = 0;
      this->outputBufferEndIndex = static_cast<std::size_t>(outputByteCount32);
    }

    // If data remains in the side buffer, move it into the output buffer
    if(this->outputBufferStartIndex < this->outputBufferEndIndex) {
      std::size_t byteCount = extractPendingOutput(outputBuffer, outputBufferCapacity);
      if(byteCount == outputBufferCapacity) {
        return StopReason::OutputBufferFull;
      } else {
        outputBufferCapacity -= byteCount;
      }
    }

    outputByteCount -= outputBufferCapacity; // Store actually written bytes
    return StopReason::Finished;
  }

  // ------------------------------------------------------------------------------------------- //

  void CscDecompressor::initializeCscDecoder() {

    // Read the properties that were used for compressing the data so the CSC decoder
    // can be initialized knowing what block sizes and bits coutns the encoder used
    ::CSCDec_ReadProperties(&this->encoderProperties, this->decoderPropertyMemory);

    // CSCDec_Create() will blow up if its allocated memory is not zero-initialized.
    // The command line tool just calls malloc() and it seems common desktop platforms
    // do zero-initialize. Our new[]-based implementation doesn't.
    this->allocator.EnableZeroInitialization(true);

    this->decoderHandle = ::CSCDec_Create(
      &this->encoderProperties, &this->inputStream, &this->allocator
    );
    if(this->decoderHandle == nullptr) {
      throw Errors::CompressionError(
        u8"CSC Decoder failed to initialize. Corrupted or invalid header in compressed stream?"
      );
    }

    // Sketchy. Zero-initialization not needed anymore. I hope...
    this->allocator.EnableZeroInitialization(false);

    // Grab the CSCDecoder (private implementation) from the opaque decoder handle
    // (which we also rummage around in)
    CSCDecInstance *decoderInstance = reinterpret_cast<CSCDecInstance *>(this->decoderHandle);
    this->decoder = decoderInstance->decoder;

    // Allocate an output buffer because CSC cannot be told to stop generating output before
    // one block is filled
    assert((this->outputBuffer == nullptr) && u8"Output buffer has not been initialized before");
    this->outputBuffer = new std::uint8_t[decoderInstance->raw_blocksize];
    // This method is only called sonce, constructor-initialized outputBufferStartIndex/EndIndex
    // are fine and left as-is.

    // decoder.Init() will have been called by ::CSCDec_Create() already
    this->decoderInitialized = true;

  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t CscDecompressor::extractPendingOutput(
    std::uint8_t *outputBuffer, std::size_t outputByteCount
  ) {
    assert(
      (this->outputBufferStartIndex < this->outputBufferEndIndex) &&
      u8"Pending out is present"
    );

    std::size_t availableByteCount = this->outputBufferEndIndex - this->outputBufferStartIndex;
    if(outputByteCount < availableByteCount) {
      std::copy_n(
        this->outputBuffer + this->outputBufferStartIndex,
        outputByteCount,
        outputBuffer
      );
      this->outputBufferStartIndex += outputByteCount;
      return outputByteCount;
    } else {
      std::copy_n(
        this->outputBuffer + this->outputBufferStartIndex,
        availableByteCount,
        outputBuffer
      );
      this->outputBufferEndIndex = 0; // Mark the side buffer as empty
      // Start index is reset when filling the output buffer again
      return availableByteCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::ZLib

#endif // defined(NUCLEX_STORAGE_HAVE_CSC)
