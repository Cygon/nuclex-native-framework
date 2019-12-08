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

#include "Nuclex/Storage/Binary/BinaryBlobReader.h"
#include "Nuclex/Storage/Blob.h"

#include <vector>

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

namespace Nuclex { namespace Storage { namespace Binary {

  // ------------------------------------------------------------------------------------------- //

  BinaryBlobReader::BinaryBlobReader(const std::shared_ptr<const Blob> &blob) :
    blob(blob),
    position(0) {}

  // ------------------------------------------------------------------------------------------- //

  BinaryBlobReader::~BinaryBlobReader() {}

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t BinaryBlobReader::GetRemainingBytes() const {
    std::uint64_t blobSize = this->blob->GetSize();
    if(this->position >= blobSize) {
      return 0;
    } else {
      return blobSize - this->position;
    }
  }
  
  // ------------------------------------------------------------------------------------------- //

  bool BinaryBlobReader::IsLittleEndian() const {
#if defined(NUCLEX_STORAGE_BIG_ENDIAN)
    // On a big endian system, we're in little endian mode if we flip
    return this->flipBytes;
#else
    // On a little endian system, we're in little endian mode by default
    return !this->flipBytes;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::SetLittleEndian(bool useLittleEndian /* = true */) {
#if defined(NUCLEX_STORAGE_BIG_ENDIAN)
    // Big endian requires us to flip if the file should be little endian
    this->flipBytes = useLittleEndian;
#else
    // Little endian requires no operation if the file should be little endian
    this->flipBytes = !useLittleEndian;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(bool &target) {
    std::uint8_t flag;

    this->blob->ReadAt(this->position, &flag, sizeof(flag));
    this->position += sizeof(flag);

    target = (flag != 0);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::uint8_t &target) {
    this->blob->ReadAt(this->position, &target, sizeof(target));
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::int8_t &target) {
    this->blob->ReadAt(this->position, &target, sizeof(target));
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::uint16_t &target) {
    if(this->flipBytes) {
      std::uint16_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      target = BYTESWAP16(temp);
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::int16_t &target) {
    if(this->flipBytes) {
      std::uint16_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      target = static_cast<std::int16_t>(BYTESWAP16(temp));
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::uint32_t &target) {
    if(this->flipBytes) {
      std::uint32_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      target = BYTESWAP32(temp);
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::int32_t &target) {
    if(this->flipBytes) {
      std::uint32_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      target = static_cast<std::int32_t>(BYTESWAP32(temp));
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::uint64_t &target) {
    if(this->flipBytes) {
      std::uint64_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      target = BYTESWAP64(temp);
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::int64_t &target) {
    if(this->flipBytes) {
      std::uint64_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      target = static_cast<std::int64_t>(BYTESWAP64(temp));
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(float &target) {
    if(this->flipBytes) {
      std::uint32_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      temp = BYTESWAP32(temp);
      target = *reinterpret_cast<float *>(&temp);
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(double &target) {
    if(this->flipBytes) {
      std::uint64_t temp;
      this->blob->ReadAt(this->position, &temp, sizeof(temp));
      temp = BYTESWAP64(temp);
      target = *reinterpret_cast<double *>(&temp);
    } else {
      this->blob->ReadAt(this->position, &target, sizeof(target));
    }
    this->position += sizeof(target);
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::string &target) {
    std::uint32_t characterCount;
    Read(characterCount);

    if(characterCount > 0) {
      std::vector<char> stringContents(characterCount);
      Read(&stringContents[0], characterCount * sizeof(char));

      target.assign(&stringContents[0], characterCount);
    } else {
      target.clear();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(std::wstring &target) {
    std::uint32_t characterCount;
    Read(characterCount);

    if(characterCount > 0) {
      std::vector<wchar_t> stringContents(characterCount);
      Read(&stringContents[0], characterCount * sizeof(wchar_t));

      target.assign(&stringContents[0], characterCount);
    } else {
      target.clear();
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BinaryBlobReader::Read(void *buffer, std::size_t byteCount) {
    this->blob->ReadAt(this->position, buffer, byteCount);
    this->position += byteCount;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Binary
