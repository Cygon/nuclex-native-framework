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

#include "Nuclex/Storage/Compression/Compressor.h"
#include "Nuclex/Storage/Binary/InputStream.h"
#include "Nuclex/Storage/Binary/OutputStream.h"

#include <stdexcept>
#include <algorithm>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the buffer used when compressing from a temporary buffer</summary>
  const std::size_t InputBufferSize = 4096;

  /// <summary>Size of the buffer used when compressing into a temporary buffer</summary>
  const std::size_t OutputBufferSize = 4096;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //
#if defined(NUCLEX_STORAGE_COMPRESSION_CONVENIENCE_METHODS)
  StopReason Compressor::Process(
    const std::uint8_t *uncompressedBuffer, std::size_t uncompressedByteCount,
    Binary::OutputStream &outputStream
  ) {
    throw -1;
  }
#endif
  // ------------------------------------------------------------------------------------------- //

  void Compressor::Process(
    Binary::InputStream &inputStream, Binary::OutputStream &outputStream
  ) {
    std::size_t inputByteCount = InputBufferSize;
    std::uint8_t inputBufferMemory[InputBufferSize];
    std::uint8_t *inputBuffer = inputBufferMemory;

    bool allInputProcessed = inputStream.ReadUpTo(inputBuffer, inputByteCount);

    // Loop that asks the output stream for a buffer
    for(;;) {
      if(inputByteCount == 0) {
        return; // All input has been processed
      }

      std::size_t processOutputByteCount = OutputBufferSize;
      std::uint8_t *outputBuffer = outputStream.GetBuffer(processOutputByteCount);
      if(outputBuffer == nullptr) {
        break; // Continue in second loop that assumes output stream provides no buffer
      }

      // We've got input, we've got a buffer to hold output, run the compressor!
      std::size_t processInputByteCount = inputByteCount;
      StopReason stopReason = Process(
        inputBuffer, processInputByteCount,
        outputBuffer, processOutputByteCount
      );

      // If output was generated, force it into the output stream all at once
      if(processOutputByteCount > 0) {
        outputStream.WriteUpTo(outputBuffer, processOutputByteCount, processOutputByteCount);
      }

      // If the compressor ran out of input, either refill the input buffer or
      // terminate if not input is left to process
      if(stopReason == StopReason::InputBufferExhausted) {
        if(allInputProcessed) {
          return; // Compressor needs more input, but all input exhausted -> we're done
        }
        inputBuffer = inputBufferMemory;
        inputByteCount = InputBufferSize;
        allInputProcessed = inputStream.ReadUpTo(inputBuffer, inputByteCount);
      } else if(!allInputProcessed) { // Stop reason is that the output buffer is full
        inputByteCount -= processInputByteCount;
        inputBuffer += processInputByteCount;

        // Try to fetch more input if our input buffer is more than half empty
        if(!allInputProcessed) {
          if(inputByteCount < InputBufferSize / 2) { // Half of input processed, fetch more
            std::copy_n(inputBuffer, inputByteCount, inputBufferMemory);
            inputBuffer = inputBufferMemory;
            std::size_t availableInputByteCont = InputBufferSize - inputByteCount;
            allInputProcessed = inputStream.ReadUpTo(inputBuffer, availableInputByteCont, 0);
            inputByteCount += availableInputByteCont;
          }
        }
      }
    } // for(;;)

    // -----------------------------------------------------
    // Notice: Copy & Paste of the above loop here
    //
    // The method switches to this copy of the loop if the output stream does not provide
    // its own buffer (default case).

    // Loop that uses its own output buffer
    {
      std::uint8_t outputBufferMemory[OutputBufferSize];
      std::uint8_t *outputBuffer = outputBufferMemory;

      for(;;) {
        if(inputByteCount == 0) {
          return; // All input has been processed
        }

        // We've got input, we've got a buffer to hold output, run the compressor!
        std::size_t processInputByteCount = inputByteCount;
        std::size_t processOutputByteCount = OutputBufferSize;
        StopReason stopReason = Process(
          inputBuffer, processInputByteCount,
          outputBuffer, processOutputByteCount
        );

        // If output was generated, force it into the output stream all at once
        if(processOutputByteCount > 0) {
          outputStream.WriteUpTo(outputBuffer, processOutputByteCount, processOutputByteCount);
        }

        // If the compressor ran out of input, either refill the input buffer or
        // terminate if not input is left to process
        if(stopReason == StopReason::InputBufferExhausted) {
          if(allInputProcessed) {
            return; // Compressor needs more input, but all input exhausted -> we're done
          }
          inputBuffer = inputBufferMemory;
          inputByteCount = InputBufferSize;
          allInputProcessed = inputStream.ReadUpTo(inputBuffer, inputByteCount);
        } else if(!allInputProcessed) { // Stop reason is that the output buffer is full
          inputByteCount -= processInputByteCount;
          inputBuffer += processInputByteCount;

          // Try to fetch more input if our input buffer is more than half empty
          if(!allInputProcessed) {
            if(inputByteCount < InputBufferSize / 2) { // Half of input processed, fetch more
              std::copy_n(inputBuffer, inputByteCount, inputBufferMemory);
              inputBuffer = inputBufferMemory;
              std::size_t availableInputByteCont = InputBufferSize - inputByteCount;
              allInputProcessed = inputStream.ReadUpTo(inputBuffer, availableInputByteCont, 0);
              inputByteCount += availableInputByteCont;
            }
          }
        }
      } // for(;;)
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void Compressor::Finish(Binary::OutputStream &outputStream) {

    for(;;) {
      std::size_t outputByteCount = OutputBufferSize;
      std::uint8_t *outputBuffer = outputStream.GetBuffer(outputByteCount);
      if(outputBuffer == nullptr) {
        break; // Continue in second loop that assumes output stream provides no buffer
      }

      StopReason reason = Finish(outputBuffer, outputByteCount);
      if(outputByteCount > 0) {
        outputStream.WriteUpTo(outputBuffer, outputByteCount, outputByteCount);
      }
      if(reason == StopReason::Finished) {
        return;
      }
    }

    // Loop that uses its own output buffer
    {
      std::uint8_t outputBufferMemory[OutputBufferSize];
      std::uint8_t *outputBuffer = outputBufferMemory;

      for(;;) {
        std::size_t outputByteCount = OutputBufferSize;
       
        StopReason reason = Finish(outputBuffer, outputByteCount);
        if(outputByteCount > 0) {
          outputStream.WriteUpTo(outputBuffer, outputByteCount, outputByteCount);
        }
        if(reason == StopReason::Finished) {
          return;
        }
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression
