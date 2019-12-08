#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#include "ZLibHelper.h"

#include <cstring>

namespace Nuclex { namespace Storage { namespace Compression { namespace ZLib {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Appends the error message stored by a ZLib stream, if any</summary>
  /// <param name="errorMessage">
  ///   Error message the stream's last error will be appended to
  /// </param>
  /// <param name="stream">Stream that will be checked for additional error information</param>
  void appendStreamErrorMessageIfAvailable(std::string &errorMessage, const ::z_stream &stream) {
    if(stream.msg != nullptr) {
      errorMessage.append(u8" (");
      errorMessage.append(stream.msg);
      errorMessage.append(u8")");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Generates an error message for the specified ZLib result code</summary>
  /// <param name="stream">ZLib stream with which the error was encountered</param>
  /// <param name="zlibResult">Result code return by a ZLib function</param>
  /// <returns>
  ///   An error message for the specified ZLib error with all available extra information
  /// </returns>
  std::string ZLibHelper::GetErrorMessage(const ::z_stream &stream, int zlibResult) {
    switch(zlibResult) {
      case Z_ERRNO: {
        int errorNumber = errno;
        std::string errorMessage(u8"System error ");
        errorMessage.append(std::to_string(errorNumber));
        errorMessage.append(u8": ");
        errorMessage.append(::strerror(errorNumber));
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
      case Z_STREAM_ERROR: {
        std::string errorMessage(
          u8"Stream error - invalid compression level, or invalid data passed to ZLib"
        );
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
      case Z_DATA_ERROR: {
        std::string errorMessage(
          u8"Data error - input data corrupted, dictionary mismatch or stream freed prematurely"
        );
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
      case Z_MEM_ERROR: {
        std::string errorMessage(
          u8"Memory error - output buffer too small or memory allocation problem"
        );
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
      case Z_BUF_ERROR: {
        std::string errorMessage(
          u8"Buffer error - output buffer too small, internal buffer too small or out of input"
        );
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
      case Z_VERSION_ERROR: {
        std::string errorMessage(
          u8"Version error - ZLib library version is not compatible"
        );
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
      default: {
        std::string errorMessage(
          u8"Generic error - ZLib returned undocumented result "
        );
        errorMessage.append(std::to_string(zlibResult));
        appendStreamErrorMessageIfAvailable(errorMessage, stream);
        return errorMessage;
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::ZLib
