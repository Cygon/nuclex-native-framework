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

#ifndef NUCLEX_STORAGE_BINARYBLOBWRITER_H
#define NUCLEX_STORAGE_BINARYBLOBWRITER_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Binary/BinaryWriter.h"

#include <memory>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class Blob;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes binary data into a blob</summary>
  /// <remarks>
  ///   <para>
  ///     Each BinaryBlobWriter maintains its own cursor, so when writing complex
  ///     data structures, you should use the same writer throughout the process instead
  ///     of creating creating writers on the fly, otherwise you'd start writing from
  ///     the beginning of the blob over and over again.
  ///   </para>
  ///   <para>
  ///     A new BinaryBlobWriter starts with the endianness that is native to the system
  ///     Nuclex.Storage.Native has been compiled for optimal performance. If you want to
  ///     write data in a portable way, simply switch the binary writer into little endian
  ///     mode (AMD/Intel x86, x64) or big endian mode (ARM, PowerPC) right after creating
  ///     it using the SetLittleEndian() method.
  ///   </para>
  /// </remarks>
  class BinaryBlobWriter : public BinaryWriter {

    /// <summary>Initializes a new binary writer for the specified blob</summary>
    /// <param name="blob">Blob the binary writer will write into</param>
    public: NUCLEX_STORAGE_API BinaryBlobWriter(const std::shared_ptr<Blob> &blob);

    /// <summary>Destroys the binary writer</summary>
    public: NUCLEX_STORAGE_API virtual ~BinaryBlobWriter() override = default;

    /// <summary>Retrieves the current position of the cursor<summary>
    /// <returns>The cursor's absolute position within the blob</returns>
    public: NUCLEX_STORAGE_API std::uint64_t GetPosition() const {
      return this->position;
    }

    /// <summary>Changes the position of the blob cursor</summary>
    /// <param name="newPosition">New absolute position of the cursor</param>
    public: NUCLEX_STORAGE_API void SetPosition(std::uint64_t newPosition) {
      this->position = newPosition;
    }

    /// <summary>Whether data should be written in little endian (x86) format<summary>
    /// <returns>True if data is written in little endian (x86) format, otherwise false</returns>
    public: NUCLEX_STORAGE_API bool IsLittleEndian() const;

    /// <summary>Sets whether data should be read in little endian format</summary>
    /// <param name="useLittleEndian">True if data should be read in little endian</param>
    public: NUCLEX_STORAGE_API void SetLittleEndian(bool useLittleEndian = true);

    /// <summary>Writes a boolean into the stream</summary>
    /// <param name="value">Boolean that will be written</param>
    public: NUCLEX_STORAGE_API void Write(bool value) override;

    /// <summary>Writes an unsigned 8 bit integer into the stream</summary>
    /// <param name="value">8 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint8_t value) override;

    /// <summary>Writes a signed 8 bit integer into the stream</summary>
    /// <param name="value">8 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int8_t value) override;

    /// <summary>Writes an unsigned 16 bit integer into the stream</summary>
    /// <param name="value">16 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint16_t value) override;

    /// <summary>Writes a signed 16 bit integer into the stream</summary>
    /// <param name="value">16 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int16_t value) override;

    /// <summary>Writes an unsigned 32 bit integer into the stream</summary>
    /// <param name="value">32 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint32_t value) override;

    /// <summary>Writes a signed 32 bit integer into the stream</summary>
    /// <param name="value">32 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int32_t value) override;

    /// <summary>Writes an unsigned 64 bit integer into the stream</summary>
    /// <param name="value">64 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint64_t value) override;

    /// <summary>Writes a signed 64 bit integer into the stream</summary>
    /// <param name="value">64 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int64_t value) override;

    /// <summary>Writes a floating point value into the stream</summary>
    /// <param name="value">Floating point value that will be written</param>
    public: NUCLEX_STORAGE_API void Write(float value) override;

    /// <summary>Writes a double precision floating point value into the stream</summary>
    /// <param name="value">Double precision floating point value that will be written</param>
    public: NUCLEX_STORAGE_API void Write(double value) override;

    /// <summary>Writes a string into the stream</summary>
    /// <param name="value">String that will be written</param>
    public: NUCLEX_STORAGE_API void Write(const std::string &value) override;

    /// <summary>Writes a wide character string into the stream</summary>
    /// <param name="value">Wide character string that will be written</param>
    /// <remarks>
    ///   Avoid using this. Wide characters are 16 bit on Windows, 32 bit on Linux and
    ///   you'll be stuck with an unportable mess of halved UTF-32 characters or
    ///   UTF-16 characters stuck in a UTF-32 wide character string. Only use UTF-8
    ///   in your public APIs.
    /// </remarks>
    public: NUCLEX_STORAGE_API void Write(const std::wstring &value) override;

    /// <summary>Writes a chunk of bytes into the stream</summary>
    /// <param name="buffer">Buffer the bytes that will be written</param>
    /// <param name="byteCount">Number of bytes to write</param>
    public: NUCLEX_STORAGE_API void Write(const void *buffer, std::size_t byteCount) override;

    /// <summary>Blob the binary reader writes into</summary>
    private: std::shared_ptr<Blob> blob;
    /// <summary>Current position of the binary writer's blob pointer</summary>
    private: std::uint64_t position;
    /// <summary>Whether the bytes will be flipped to convert endianness</summary>
    private: bool flipBytes;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary

#endif // NUCLEX_STORAGE_BINARY_BINARYBLOBWRITER_H
