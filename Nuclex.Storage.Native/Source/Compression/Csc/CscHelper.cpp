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

#include "CscHelper.h"

#include <csc_common.h> // for CSCEncProps_Init() etc.

#include "Nuclex/Storage/Errors/CompressionError.h"

namespace Nuclex { namespace Storage { namespace Compression { namespace Csc {

  // ------------------------------------------------------------------------------------------- //

  void throwErrorForThrownInteger(int thrownInteger) {
    switch(thrownInteger) {
      case DECODE_ERROR: {
        throw Errors::CompressionError(u8"CSC failed with a decoding error");
      }
      case WRITE_ERROR: {
        throw Errors::CompressionError(u8"CSC failed with a write error");
      }
      case READ_ERROR: {
        throw Errors::CompressionError(u8"CSC failed with a read error");
      }
      default: {
        throw Errors::CompressionError(u8"CSC failed with an unknown error");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::Csc
