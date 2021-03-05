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

#ifndef NUCLEX_PIXELS_STORAGE_EXR_OPENEXRHELPERS_H
#define NUCLEX_PIXELS_STORAGE_EXR_OPENEXRHELPERS_H

#include "Nuclex/Pixels/Config.h"

#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)

#include "Nuclex/Pixels/PixelFormat.h"
#include "Nuclex/Pixels/Storage/VirtualFile.h"

#include <cstdint>

// Sadly, OpenEXR has lots of really poor programming practices and
// also doesn't seem to care much about compiler warnings...
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4244) // conversion from 'int' to 'unsigned short'
#pragma warning(disable:4996) // function or variable may be unsafe
#elif defined(__clang__)
#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wfloat-equal -Wdeprecated"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy" // default cctor deprecated
#endif
#include <IlmImf/ImfIO.h>
#include <IlmImf/ImfFrameBuffer.h>
#include <IlmImf/ImfInputFile.h>
#include <IlmImf/ImfArray.h>
#include <IlmImf/ImfRgba.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

namespace Nuclex { namespace Pixels { namespace Storage { namespace Exr {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of the smallest valid .exr file</summary>
  /// <remarks>
  ///   This is just a guess. I created a 1x1 pixel .exr image in Krita and disabled
  ///   everything possible upon saving. The result was a 355 byte file. Any useful
  ///   .exr image will thus have more than 355 bytes. To be on the safe side, I went
  ///   with 256 bytes, in case there are some optimizations Krita didn't offer.
  /// </remarks>
  constexpr const std::size_t SmallestPossibleExrSize = 256;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper class for reading .ext image files using OpenEXR</summary>
  class Helpers {

    /// <summary>
    ///   Checks whether the first 8 bytes in a file are a valid .exr file header
    /// </summary>
    /// <param name="fileHeader">File header that will be checked</param>
    /// <returns>True if the file header is a valid .exr file header</returns>
    /// <remarks>
    ///   The file header must contain at least the first 8 bytes of the file,
    ///   otherwise this will segfault.
    /// </remarks>
    public: static bool IsValidExrHeader(const std::uint8_t *fileHeader);

    /// <summary>Sets up an OpenEXR frame buffer matching the specified pixel format</summary>
    /// <param name="frameBuffer">Frame buffer that will be set up</param>
    /// <param name="pixelFormat">Pixel format the frame buffer will be set to</param>
    /// <param name="pixels">Address of the pixel buffer the frame buffer will use</param>
    /// <remarks>
    ///   OpenEXR allows the frame buffer format to be set relatively freely (if one
    ///   foregoes the RgbaInputFile wrapper). This method makes use of that feature.
    /// </remarks>
    public: static void AddChannelsToFrameBuffer(
      Imf::FrameBuffer &frameBuffer, PixelFormat pixelFormat, void *pixels
    );

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Adapter that allows OpenEXR to access a VirtualFile as an IStream</summary>
  class VirtualFileInputStream : public Imf::IStream {

    /// <summary>Initializes a new VirtualFile IStream adapter</summary>
    /// <param name="file">File from which the IStream adapter will read</param>
    public: VirtualFileInputStream(const Nuclex::Pixels::Storage::VirtualFile &file) :
      IStream(u8"VirtualFile adapter stream"),
      file(file),
      position(0),
      length(file.GetSize()) {}

    /// <summary>Frees all resources owned by the virtual file steam</summary>
    public: virtual ~VirtualFileInputStream() = default;

    /// <summary>Does this input stream support memory-mapped IO?</summary>
    public: bool isMemoryMapped() const override { return false; }

    /// <summary>Read from the stream</summary>
    /// <param name="buffer">Buffer in which the data will be stored</param>
    /// <param name="byteCount">Number of bytes that will be read</param>
    /// <returns>Whether more bytes are available from the file</returns>
    public: bool read(char buffer[/*byteCount*/], int byteCount) override {
      this->file.ReadAt(
        this->position, byteCount, reinterpret_cast<std::uint8_t *>(buffer)
      );
      this->position += byteCount;

      return (this->position < this->length);
    }

    /// <summary>Throws an exception if a memory-mapped read is attempted</summary>
    /// <param name="byteCount">Numeber of bytes to read from the file</param>
    /// <returns>Address containing the memory-mapped file's data</returns>
    public: char *readMemoryMapped(int byteCount) override {
      (void)byteCount; // Unused
      throw std::logic_error(u8"Stream is not memory mapped");
    }

    /// <summary>Looks up the current position of the file cursor</summary>
    /// <returns>The current position of the file cursor</returns>
    public: Imf::Int64 tellg() override { // Signed? Non-const? WTF?!
      return static_cast<std::uint64_t>(this->length);
    }

    /// <summary>Moves the file cursor to the specified position</summary>
    /// <param name="position">Position the file cursor will be moved to</param>
    public: void seekg(Imf::Int64 newPosition) override {
      //assert(pos >= 0 && pos <= length);
      this->position = static_cast<std::uint64_t>(newPosition);
    }

    /// <summary>Clears any error conditions if a read failed</summary>
    public: void clear() override {} // Virtual files are stateless

    /// <summary>Virtual file from which the IStream adapter is reading</summary>
    private: const Nuclex::Pixels::Storage::VirtualFile &file;
    /// <summary>Current position of the file pointer</summary>
    private: std::uint64_t position;
    /// <summary>Total length of the file in bytes</summary>
    private: std::uint64_t length;

  };

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Exr

#endif // defined(NUCLEX_PIXELS_HAVE_OPENEXR)

#endif // NUCLEX_PIXELS_STORAGE_EXR_OPENEXRHELPERS_H
