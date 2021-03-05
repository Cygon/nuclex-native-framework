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

#ifndef NUCLEX_STORAGE_COMPRESSION_ZLIB_ZLIBHELPER_H
#define NUCLEX_STORAGE_COMPRESSION_ZLIB_ZLIBHELPER_H

#include "Nuclex/Storage/Config.h"  

#include <zlib.h>

#include <string>

namespace Nuclex { namespace Storage { namespace Compression { namespace ZLib {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides helper methods to deal with ZLib</summary>
  class ZLibHelper {

    /// <summary>Looks up the error message for the specified result code</summary>
    /// <param name="stream">
    ///   Compression or decompression stream that may hold an error message
    /// </param>
    /// <param name="zlibResult">
    ///   Result code for which the error message will be looked up
    /// </param>
    /// <returns>The corresponding ZLib error message</returns>
    public: static std::string GetErrorMessage(const ::z_stream &stream, int zlibResult);

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Storage::Compression::ZLib

#endif // NUCLEX_STORAGE_COMPRESSION_ZLIB_ZLIBHELPER_H
