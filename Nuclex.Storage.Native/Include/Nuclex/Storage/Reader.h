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

#ifndef NUCLEX_STORAGE_READER_H
#define NUCLEX_STORAGE_READER_H

#include "Config.h"

#include <cstdint>
#include <string>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data from a stream or similar source</summary>
  class Reader {

    /// <summary>Destroys a data reader</summary>
    public: NUCLEX_STORAGE_API virtual ~Reader() {}

    /// <summary>Reads a boolean from the stream</summary>
    /// <param name="target">Address of a boolean the value will be read into</param>
    public: virtual void Read(bool &target) = 0;

    /// <summary>Reads an unsigned 8 bit integer from the stream</summary>
    /// <param name="target">Address of an 8 bit integer the value will be read into</param>
    public: virtual void Read(std::uint8_t &target) = 0;

    /// <summary>Reads a signed 8 bit integer from the stream</summary>
    /// <param name="target">Address of an 8 bit integer the value will be read into</param>
    public: virtual void Read(std::int8_t &target) = 0;

    /// <summary>Reads an unsigned 16 bit integer from the stream</summary>
    /// <param name="target">Address of a 16 bit integer the value will be read into</param>
    public: virtual void Read(std::uint16_t &target) = 0;

    /// <summary>Reads a signed 16 bit integer from the stream</summary>
    /// <param name="target">Address of a 16 bit integer the value will be read into</param>
    public: virtual void Read(std::int16_t &target) = 0;

    /// <summary>Reads an unsigned 32 bit integer from the stream</summary>
    /// <param name="target">Address of a 32 bit integer the value will be read into</param>
    public: virtual void Read(std::uint32_t &target) = 0;

    /// <summary>Reads a signed 32 bit integer from the stream</summary>
    /// <param name="target">Address of a 32 bit integer the value will be read into</param>
    public: virtual void Read(std::int32_t &target) = 0;

    /// <summary>Reads an unsigned 64 bit integer from the stream</summary>
    /// <param name="target">Address of a 64 bit integer the value will be read into</param>
    public: virtual void Read(std::uint64_t &target) = 0;

    /// <summary>Reads a signed 64 bit integer from the stream</summary>
    /// <param name="target">Address of a 64 bit integer the value will be read into</param>
    public: virtual void Read(std::int64_t &target) = 0;

    /// <summary>Reads a floating point value from the stream</summary>
    /// <param name="target">Address of a floating point value that will be read into</param>
    public: virtual void Read(float &target) = 0;

    /// <summary>Reads a double precision floating point value from the stream</summary>
    /// <param name="target">
    ///   Address of a double precision floating point value that will be read into
    /// </param>
    public: virtual void Read(double &target) = 0;

    /// <summary>Reads a string from the stream</summary>
    /// <param name="target">Address of a string the value will be read into</param>
    public: virtual void Read(std::string &target) = 0;

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
    public: virtual void Read(std::wstring &target) = 0;

    /// <summary>Reads a chunk of bytes from the stream</summary>
    /// <param name="buffer">Buffer the bytes will be read into</param>
    /// <param name="byteCount">Number of bytes that will be read from the stream</param>
    public: virtual void Read(void *buffer, std::size_t byteCount) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

#endif // NUCLEX_STORAGE_READER_H
