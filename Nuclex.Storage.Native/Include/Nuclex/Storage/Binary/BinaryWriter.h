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

#ifndef NUCLEX_STORAGE_BINARY_BINARYWRITER_H
#define NUCLEX_STORAGE_BINARY_BINARYWRITER_H

#include "Nuclex/Storage/Config.h"

#include "Nuclex/Storage/Writer.h"

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes binary data into a stream</summary>
  /// <remarks>
  ///   <para>
  ///     The binary writer enables classes to write binary data in a precisely controlled
  ///     format into files and other destinations a binary writer can be built for.
  ///     The binary writer provides methods for integers of any size (so implementations
  ///     have the ability to convert between big endian to little endian as needed),
  ///     floating point values, strings and binary blobs.
  ///   </para>
  ///   <para>
  ///     Binary writers are useful to save data into existing, binary formats and have
  ///     very low overhead due to most operations happening directly on the data store.
  ///     They can also be used to persist states using little disk space, offering high
  ///     throughput and marginal protection against casual tampering.
  ///   </para>
  ///   <para>
  ///     If you want to employ binary serialization to store your game's state, you can
  ///     use the SerializationManager to obtain a BinarySerializationWriter which has
  ///     the added ability to write any type for which a Serializer (classes responsible
  ///     for turning objects into data and back) was registered earlier.
  ///   </para>
  /// </remarks>
  class BinaryWriter : public Writer {

    /// <summary>Destroys a binary data writer</summary>
    public: NUCLEX_STORAGE_API virtual ~BinaryWriter() = default;

    /// <summary>Whether data should be written in little endian (x86) format<summary>
    /// <returns>True if data is written in little endian (x86) format, otherwise false</returns>
    public: virtual bool IsLittleEndian() const = 0;

    /// <summary>Sets whether data should be read in little endian format</summary>
    /// <param name="useLittleEndian">True if data should be read in little endian</param>
    public: virtual void SetLittleEndian(bool useLittleEndian = true) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary

#endif // NUCLEX_STORAGE_BINARY_BINARYWRITER_H
