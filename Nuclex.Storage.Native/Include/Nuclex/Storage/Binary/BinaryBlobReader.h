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

#ifndef NUCLEX_STORAGE_BINARY_BINARYBLOBREADER_H
#define NUCLEX_STORAGE_BINARY_BINARYBLOBREADER_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Binary/BinaryReader.h"

#include <memory>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class Blob;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads binary data from a blob</summary>
  /// <remarks>
  ///   <para>
  ///     Each BinaryBlobReader maintains its own cursor, so accessing the same blob from
  ///     multiple readers is not a problem as long as you do not share readers. It is
  ///     also important to take this concept into consideration when designing file access
  ///     code since if you created readers on the fly, you'd start over from the beginning
  ///     of the file each time.
  ///   </para>
  ///   <para>
  ///     A new BinaryBlobReader starts with the endianness that is native to the system
  ///     Nuclex.Storage.Native has been compiled on for optimal performance. If you want
  ///     to read data in a portable way, simply switch the binary reader into little endian
  ///     mode (AMD/Intel x86, x64) or big endian mode (ARM, PowerPC) right after creating
  ///     it using the SetLittleEndian() method.
  ///   </para>
  /// </remarks>
  class BinaryBlobReader : public BinaryReader {

    /// <summary>Initializes a new binary file reader for the specified file</summary>
    /// <param name="blob">Blob the binary file reader will read from</param>
    public: NUCLEX_STORAGE_API BinaryBlobReader(const std::shared_ptr<const Blob> &blob);

    /// <summary>Destroys a binary data reader</summary>
    public: NUCLEX_STORAGE_API virtual ~BinaryBlobReader() override;

    /// <summary>Retrieves the current position of the cursor<summary>
    /// <returns>The cursor's absolute position within the blob</returns>
    public: NUCLEX_STORAGE_API std::uint64_t GetPosition() const {
      return this->position;
    }

    /// <summary>Changes the position of the cursor</summary>
    /// <param name="newPosition">New absolute position of the cursor</param>
    public: NUCLEX_STORAGE_API void SetPosition(std::uint64_t newPosition) {
      this->position = newPosition;
    }

    /// <summary>Returns the number of bytes remaining to be read</summary>
    public: NUCLEX_STORAGE_API std::uint64_t GetRemainingBytes() const;

    /// <summary>Whether data should be read in little endian (x86) format<summary>
    /// <returns>True if data is read a little endian (x86) format, otherwise false</returns>
    public: NUCLEX_STORAGE_API bool IsLittleEndian() const override;

    /// <summary>Sets whether data should be read in little endian format</summary>
    /// <param name="useLittleEndian">True if data should be read in little endian</param>
    public: NUCLEX_STORAGE_API void SetLittleEndian(bool useLittleEndian = true) override;

    /// <summary>Reads a boolean integer from the stream</summary>
    /// <param name="target">Address of a boolean the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(bool &target) override;

    /// <summary>Reads an unsigned 8 bit integer from the stream</summary>
    /// <param name="target">Address of an 8 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint8_t &target) override;

    /// <summary>Reads a signed 8 bit integer from the stream</summary>
    /// <param name="target">Address of an 8 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int8_t &target) override;

    /// <summary>Reads an unsigned 16 bit integer from the stream</summary>
    /// <param name="target">Address of a 16 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint16_t &target) override;

    /// <summary>Reads a signed 16 bit integer from the stream</summary>
    /// <param name="target">Address of a 16 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int16_t &target) override;

    /// <summary>Reads an unsigned 32 bit integer from the stream</summary>
    /// <param name="target">Address of a 32 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint32_t &target) override;

    /// <summary>Reads a signed 32 bit integer from the stream</summary>
    /// <param name="target">Address of a 32 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int32_t &target) override;

    /// <summary>Reads an unsigned 64 bit integer from the stream</summary>
    /// <param name="target">Address of a 64 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint64_t &target) override;

    /// <summary>Reads a signed 64 bit integer from the stream</summary>
    /// <param name="target">Address of a 64 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int64_t &target) override;

    /// <summary>Reads a floating point value from the stream</summary>
    /// <param name="target">Address of a floating point value that will be read into</param>
    public: NUCLEX_STORAGE_API void Read(float &target) override;

    /// <summary>Reads a double precision floating point value from the stream</summary>
    /// <param name="target">
    ///   Address of a double precision floating point value that will be read into
    /// </param>
    public: NUCLEX_STORAGE_API void Read(double &target) override;

    /// <summary>Reads a string from the stream</summary>
    /// <param name="target">Address of a string the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::string &target) override;

    /// <summary>Reads a wide character string from the stream</summary>
    /// <param name="target">
    ///   Address of a wide character string the value will be read into
    /// </param>
    /// <remarks>
    ///   Avoid using this. Wide characters are 16 bit on Windows, 32 bit on Linux and
    ///   you'll be stuck with an unportable mess of halved UTF-32 characters or
    ///   UTF-16 characters stuck in a UTF-32 wide character string. Only use UTF-8
    ///   in your public APIs.
    /// </remarks>
    public: NUCLEX_STORAGE_API void Read(std::wstring &target) override;

    /// <summary>Reads a chunk of bytes from the stream</summary>
    /// <param name="buffer">Buffer the bytes will be read into</param>
    /// <param name="byteCount">Number of bytes that will be read from the stream</param>
    public: NUCLEX_STORAGE_API void Read(void *buffer, std::size_t byteCount) override;

    /// <summary>Blob the binary reader reads from</summary>
    private: std::shared_ptr<const Blob> blob;
    /// <summary>Current position of the binary reader's file pointer</summary>
    private: std::uint64_t position;
    /// <summary>Whether the bytes will be flipped to convert endianness</summary>
    private: bool flipBytes;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary

#endif // NUCLEX_STORAGE_BINARY_BINARYBLOBREADER_H
