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

#include "Nuclex/Storage/Binary/BinaryBlobWriter.h"
#include "Nuclex/Storage/Blob.h"

#if defined(NUCLEX_STORAGE_WIN32)
  #define BYTESWAP16 _byteswap_ushort
  #define BYTESWAP32 _byteswap_ulong
  #define BYTESWAP64 _byteswap_uint64
#elif defined(NUCLEX_STORAGE_LINUX)
  #define BYTESWAP16 __builtin_bswap16
  #define BYTESWAP32 __builtin_bswap32
  #define BYTESWAP64 __builtin_bswap64
#else
  #define BYTESWAP16(x) ( \
    (static_cast<std::uint16_t>(x) << 8) |
    (static_cast<std::uint16_t>(x) >> 8) \
  )
  #define BYTESWAP32(x) ( \
    (static_cast<std::uint32_t>(x & 0xFF000000) >> 24) |
    (static_cast<std::uint32_t>(x & 0x00FF0000) >> 8) |
    (static_cast<std::uint32_t>(x & 0x0000FF00) << 8) |
    (static_cast<std::uint32_t>(x & 0x000000FF) << 24) \
  )
  #define BYTESWAP64(x) ( \
    (static_cast<std::uint64_t>(x & 0xFF00000000000000) >> 56) |
    (static_cast<std::uint64_t>(x & 0x00FF000000000000) >> 40) |
    (static_cast<std::uint64_t>(x & 0x0000FF0000000000) >> 24) |
    (static_cast<std::uint64_t>(x & 0x000000FF00000000) >> 8) |
    (static_cast<std::uint64_t>(x & 0x00000000FF000000) << 8) |
    (static_cast<std::uint64_t>(x & 0x0000000000FF0000) << 24) |
    (static_cast<std::uint64_t>(x & 0x000000000000FF00) << 40) |
    (static_cast<std::uint64_t>(x & 0x00000000000000FF) << 56) \
  )
#endif

namespace Nuclex { namespace Storage {  namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  BinaryBlobWriter::BinaryBlobWriter(const std::shared_ptr<Blob> &blob) :
    blob(blob),
    position(0),
    flipBytes(false) {}

  // ------------------------------------------------------------------------------------------- //

  bool BinaryBlobWriter::IsLittleEndian() const {
#if defined(NUCLEX_STORAGE_BIG_ENDIAN)
    // On a big endian system, we're in little endian mode if we flip
    return this->flipBytes;
#else
    // On a little endian system, we're in little endian mode by default
    return !this->flipBytes;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::SetLittleEndian(bool useLittleEndian /* = true */) {
#if defined(NUCLEX_STORAGE_BIG_ENDIAN)
    // Big endian requires us to flip if the file should be little endian
    this->flipBytes = useLittleEndian;
#else
    // Little endian requires no operation if the file should be little endian
    this->flipBytes = !useLittleEndian;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(bool value) {
    std::uint8_t flag = value ? 1 : 0;

    this->blob->WriteAt(this->position, &flag, sizeof(flag));
    this->position += sizeof(flag);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::uint8_t value) {
    this->blob->WriteAt(this->position, &value, sizeof(value));
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::int8_t value) {
    this->blob->WriteAt(this->position, &value, sizeof(value));
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::uint16_t value) {
    if(this->flipBytes) {
      std::uint16_t temp = BYTESWAP16(value);
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::int16_t value) {
    if(this->flipBytes) {
      std::uint16_t temp = BYTESWAP16(static_cast<std::uint16_t>(value));
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::uint32_t value) {
    if(this->flipBytes) {
      std::uint32_t temp = BYTESWAP32(value);
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::int32_t value) {
    if(this->flipBytes) {
      std::uint32_t temp = BYTESWAP32(static_cast<std::uint32_t>(value));
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::uint64_t value) {
    if(this->flipBytes) {
      std::uint64_t temp = BYTESWAP64(value);
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(std::int64_t value) {
    if(this->flipBytes) {
      std::uint64_t temp = BYTESWAP64(static_cast<std::uint64_t>(value));
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(float value) {
    if(this->flipBytes) {
      std::uint32_t temp = BYTESWAP32(*reinterpret_cast<std::uint32_t *>(&value));
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(double value) {
    if(this->flipBytes) {
      std::uint64_t temp = BYTESWAP64(*reinterpret_cast<std::uint64_t *>(&value));
      this->blob->WriteAt(this->position, &temp, sizeof(temp));
    } else {
      this->blob->WriteAt(this->position, &value, sizeof(value));
    }
    this->position += sizeof(value);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(const std::string &value) {
    std::uint32_t length = static_cast<std::uint32_t>(value.length());
    Write(length);

    if(length > 0) {
      Write(value.c_str(), length * sizeof(char));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(const std::wstring &value) {
    std::uint32_t length = static_cast<std::uint32_t>(value.length());
    Write(length);

    if(length > 0) {
      Write(value.c_str(), length * sizeof(wchar_t));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobWriter::Write(const void *buffer, std::size_t byteCount) {
    this->blob->WriteAt(this->position, buffer, byteCount);
    this->position += byteCount;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary
