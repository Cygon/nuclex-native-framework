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

#ifndef NUCLEX_STORAGE_COMPRESSION_STOPREASON_H
#define NUCLEX_STORAGE_COMPRESSION_STOPREASON_H

#include "Nuclex/Storage/Config.h"

namespace Nuclex { namespace Storage { namespace Compression {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reasons a compressor or decompressor has stopped processing</summary>
  enum class StopReason {

    /// <summary>Compression or decompression has finished</summary>
    /// <remarks>
    ///   <para>
    ///     This value is returned by <see cref="Compressor.Finish" /> when all data has
    ///     been flushed into the output buffer. After receiving this result, the compressor
    ///     is finished and the matching decompressor will be able to reconstruct your
    ///     full, original input from the compressed data.
    ///   </para>
    /// </remarks>
    Finished = 0,

    /// <summary>The compressor or decompressor has consumed all provided input</summary>
    /// <remarks>
    ///   <para>
    ///     This stop reason may be returned by <see cref="Compressor.Process" /> to indicate
    ///     that all input was processed and the output buffer is not yet full. You can follow
    ///     up with more calls to <see cref="Compressor.Process" /> or finish compression by
    ///     calling <see cref="Compressor.Finish" />.
    ///   </para>
    ///   <para>
    ///     The output buffer is at this point still incomplete (compression algorithms
    ///     likely have internal buffers and state in which data will linger), so you will
    ///     still need to call <see cref="Compressor.Finish" /> in order to ensure all
    ///     compressed data has been flushed into the output buffer.
    ///   </para>
    /// </remarks>
    InputBufferExhausted = 1,

    /// <summary>The compressor or decompressor has filled the output buffer</summary>
    /// <remarks>
    ///   <para>
    ///     You may get this result from the <see cref="Compressor.Process" /> or from
    ///     <see cref="Compressor.Finish" /> to indicate that the respective method ran out
    ///     of space in the provided output buffer before processing all input.
    ///   </para>
    ///   <para>
    ///     To handle it, you sould write the output buffer to disk (or enlarge you buffer),
    ///     then repeat your last call until it returns something else.
    ///   </para>
    /// </remarks>
    OutputBufferFull = 2

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Compression

#endif // NUCLEX_STORAGE_COMPRESSION_STOPREASON_H
