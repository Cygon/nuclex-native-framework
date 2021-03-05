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

#include "BrotliCompressor.h"

#if defined(NUCLEX_STORAGE_HAVE_BROTLI)

#include "Nuclex/Storage/Errors/CompressionError.h"

#include <cassert> // for assert()
#include <new> // for std::nothrow
#include <limits> // for std::numeric_limits

namespace Nuclex { namespace Storage { namespace Compression { namespace Brotli {

  // ------------------------------------------------------------------------------------------- //

  BrotliCompressor::BrotliCompressor(int quality) :
    state(::BrotliEncoderCreateInstance(nullptr, nullptr, nullptr)) {

    BROTLI_BOOL result = ::BrotliEncoderSetParameter(
      this->state, BrotliEncoderParameter::BROTLI_PARAM_QUALITY, quality
    );
    if(result == BROTLI_FALSE) {
      throw std::logic_error(u8"Could not set compression quality parameter");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  BrotliCompressor::~BrotliCompressor() {
    ::BrotliEncoderDestroyInstance(this->state);
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason BrotliCompressor::Process(
    const std::uint8_t *uncompressedBuffer, std::size_t &uncompressedByteCount,
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    std::size_t remainingOutputByteCount = outputByteCount;
    BROTLI_BOOL result = ::BrotliEncoderCompressStream(
      this->state,
      BrotliEncoderOperation::BROTLI_OPERATION_PROCESS,
      &uncompressedByteCount, &uncompressedBuffer,
      &remainingOutputByteCount, &outputBuffer,
      nullptr
    );
    if(result == BROTLI_FALSE) {
      throw Errors::CompressionError(
        u8"Brotli encoder reported a failure compressing the provied data"
      );
    }

    outputByteCount -= remainingOutputByteCount;

    if(::BrotliEncoderHasMoreOutput(this->state) != BROTLI_FALSE) {
      return StopReason::OutputBufferFull;
    } else {
      assert((uncompressedByteCount == 0) && u8"All input data was processed");
      return StopReason::InputBufferExhausted;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  StopReason BrotliCompressor::Finish(
    std::uint8_t *outputBuffer, std::size_t &outputByteCount
  ) {
    const std::uint8_t *inputBuffer = nullptr;
    std::size_t inputByteCount = 0;

    std::size_t remainingOutputByteCount = outputByteCount;
    BROTLI_BOOL result = ::BrotliEncoderCompressStream(
      this->state,
      BrotliEncoderOperation::BROTLI_OPERATION_FINISH,
      &inputByteCount, &inputBuffer,
      &remainingOutputByteCount, &outputBuffer,
      nullptr
    );
    if(result == BROTLI_FALSE) {
      throw Errors::CompressionError(
        u8"Brotli encoder reported a failure compressing the provided data"
      );
    }

    outputByteCount -= remainingOutputByteCount;

    if(::BrotliEncoderHasMoreOutput(this->state) != BROTLI_FALSE) {
      return StopReason::OutputBufferFull;
    } else {
      return StopReason::Finished;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Brotli

#endif // defined(NUCLEX_STORAGE_HAVE_BROTLI)
