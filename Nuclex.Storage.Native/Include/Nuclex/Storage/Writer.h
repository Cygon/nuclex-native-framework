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

#ifndef NUCLEX_STORAGE_WRITER_H
#define NUCLEX_STORAGE_WRITER_H

#include "Config.h"

#include <cstdint>
#include <string>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes data into a stream or other destination</summary>
  class Writer {

    /// <summary>Destroys a binary data writer</summary>
    public: NUCLEX_STORAGE_API virtual ~Writer() {}

    /// <summary>Writes a boolean into the stream</summary>
    /// <param name="value">Boolean that will be written</param>
    public: virtual void Write(bool value) = 0;

    /// <summary>Writes an unsigned 8 bit integer into the stream</summary>
    /// <param name="value">8 bit integer that will be written</param>
    public: virtual void Write(std::uint8_t value) = 0;

    /// <summary>Writes a signed 8 bit integer into the stream</summary>
    /// <param name="value">8 bit integer that will be written</param>
    public: virtual void Write(std::int8_t value) = 0;

    /// <summary>Writes an unsigned 16 bit integer into the stream</summary>
    /// <param name="value">16 bit integer that will be written</param>
    public: virtual void Write(std::uint16_t value) = 0;

    /// <summary>Writes a signed 16 bit integer into the stream</summary>
    /// <param name="value">16 bit integer that will be written</param>
    public: virtual void Write(std::int16_t value) = 0;

    /// <summary>Writes an unsigned 32 bit integer into the stream</summary>
    /// <param name="value">32 bit integer that will be written</param>
    public: virtual void Write(std::uint32_t value) = 0;

    /// <summary>Writes a signed 32 bit integer into the stream</summary>
    /// <param name="value">32 bit integer that will be written</param>
    public: virtual void Write(std::int32_t value) = 0;

    /// <summary>Writes an unsigned 64 bit integer into the stream</summary>
    /// <param name="value">64 bit integer that will be written</param>
    public: virtual void Write(std::uint64_t value) = 0;

    /// <summary>Writes a signed 64 bit integer into the stream</summary>
    /// <param name="value">64 bit integer that will be written</param>
    public: virtual void Write(std::int64_t value) = 0;

    /// <summary>Writes a floating point value into the stream</summary>
    /// <param name="value">Floating point value that will be written</param>
    public: virtual void Write(float value) = 0;

    /// <summary>Writes a double precision floating point value into the stream</summary>
    /// <param name="value">Double precision floating point value that will be written</param>
    public: virtual void Write(double value) = 0;

    /// <summary>Writes a string into the stream</summary>
    /// <param name="value">String that will be written</param>
    public: virtual void Write(const std::string &value) = 0;

    /// <summary>Writes C style zero-terminated string</summary>
    /// <param name="value">C style string that will be written</param>
    /// <remarks>
    ///   This overload is only provided because otherwise, passing a string constant
    ///   to the write method would cause the <see cref="Write(bool)" /> overload to be called.
    /// </remarks>
    public: NUCLEX_STORAGE_API void Write(const char *value) {
      Write(std::string(value));
    }

    /// <summary>Writes a unicode string into the stream</summary>
    /// <param name="value">Unicode string that will be written</param>
    /// <remarks>
    ///   Avoid using this. Wide characters are 16 bit on Windows, 32 bit on Linux and
    ///   you'll be stuck with an unportable mess of halved UTF-32 characters or
    ///   UTF-16 characters stuck in a UTF-32 wide character string. Only use UTF-8
    ///   in your public APIs.
    /// </remarks>
    public: virtual void Write(const std::wstring &value) = 0;

    /// <summary>Writes C style zero-terminated string</summary>
    /// <param name="value">C style string that will be written</param>
    /// <remarks>
    ///   This overload is only provided because otherwise, passing a string constant
    ///   to the write method would cause the <see cref="Write(bool)" /> overload to be called.
    /// </remarks>
    public: NUCLEX_STORAGE_API void Write(const wchar_t *value) {
      Write(std::wstring(value));
    }

    /// <summary>Writes a chunk of bytes into the stream</summary>
    /// <param name="buffer">Buffer the bytes that will be written</param>
    /// <param name="byteCount">Number of bytes to write</param>
    public: virtual void Write(const void *buffer, std::size_t byteCount) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

#endif // NUCLEX_STORAGE_WRITER_H
