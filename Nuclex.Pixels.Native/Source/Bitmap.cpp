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
#define NUCLEX_PIXELS_SOURCE 1

#include "Nuclex/Pixels/Bitmap.h"
#include "Nuclex/Pixels/Errors/PixelFormatError.h"

#include <algorithm> // for std::min
#include <cstdint> // for std::uint8_t
#include <memory> // for std::unique_ptr

#include <cassert> // for assert()

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Rounds the specified value to the next multiple of a factor</summary>
  /// <param name="value">Value that will be rounded up</param>
  /// <param name="factor">Factor to which the value will be rounded up</param>
  /// <returns>The next multiple of the specified factor</returns>
  std::size_t nextMultiple(std::size_t value, std::size_t factor) {
    std::size_t remainder = value % factor;
    if(remainder > 0) {
      value += (factor - remainder);
    }

    return value;
    // Alternative trick: no conditions, but fails on zero
    //return value + factor - 1 - (value - 1) % factor;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Initializes a bitmap memory instance for specified format</summary>
  /// <param name="width">Width of the bitmap in pixels</param>
  /// <param name="height">Height of the bitmap in pixels</param>
  /// <param name="pixelFormat">Pixel format used by the bitmap</param>
  /// <returns>A bitmap memory instance with the specified attributes</returns>
  Nuclex::Pixels::BitmapMemory makeBitmapMemory(
    std::size_t width, std::size_t height, Nuclex::Pixels::PixelFormat pixelFormat
  ) {
    Nuclex::Pixels::BitmapMemory memory;
    memory.Width = width;
    memory.Height = height;
    memory.PixelFormat = pixelFormat;
    return memory;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Determines the stride (bytes per line) required for a bitmap</summary>
  /// <param name="width">Desired width of the bitmap in pixels</param>
  /// <param name="pixelFormat">Pixel format used by the bitmap</param>
  /// <returns>The number of bytes required to store one line of the bitmap</returns>
  int determineStride(std::size_t width, Nuclex::Pixels::PixelFormat pixelFormat) {
    Nuclex::Pixels::Size blockSize = Nuclex::Pixels::GetBlockSize(pixelFormat);
    width = nextMultiple(width, blockSize.Width);
    return static_cast<int>(Nuclex::Pixels::CountRequiredBytes(pixelFormat, width));
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Calculates the minimum required resolution for the buffer</summary>
  /// <param name="width">Target width of the bitmap in pixels</param>
  /// <param name="height">Target height of the bitmap in pixels</param>
  /// <param name="pixelFormat">Pixel format used to store the bitmap's pixels</param>
  /// <returns>The minimum required resolution of a buffer to hold the bitmap</returns>
  /// <remarks>
  ///   For compressed pixel formats, the bitmap's dimensions in one or both directions
  ///   must be multiples of the block size. For example, a BCn compressed bitmap needs
  ///   to have dimensions that are a multiple of 4. This method adusts the target
  ///   resolution to the required actual resolution.
  /// </remarks>
  Nuclex::Pixels::Size getRequiredBufferSize(
    std::size_t width, std::size_t height, Nuclex::Pixels::PixelFormat pixelFormat
  ) {
    Nuclex::Pixels::Size size = Nuclex::Pixels::GetBlockSize(pixelFormat);

    size.Width = nextMultiple(width, size.Width);
    size.Height = nextMultiple(height, size.Height);

    return size;
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels {

  /// <summary>Detachable memory buffer that allows for shared ownership</summary>
  struct Bitmap::SharedBuffer {
    /// <summary>Number of owners that are holding onto the bitmap's memory</summary>
    public: mutable std::size_t OwnerCount;
    /// <summary>Memory the buffer is managing</summary>
    public: void *Memory;
    // This structure is followed by the actual bitmap data if created by this library
  };

  // ------------------------------------------------------------------------------------------- //

  Bitmap Bitmap::InExistingMemory(const BitmapMemory &bitmapMemory) {

    // Allocate the shared buffer this way because it's released via delete[]
    // (other constructors allocate the shared buffer + bitmap memory in one block)
    std::uint8_t *memory = new std::uint8_t[sizeof(SharedBuffer)];
    SharedBuffer *buffer = new(memory) SharedBuffer();
    buffer->OwnerCount = 1;
    buffer->Memory = bitmapMemory.Pixels;

    return Bitmap(buffer, bitmapMemory);

  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::Bitmap(
    std::size_t width,
    std::size_t height,
    PixelFormat pixelFormat /* = PixelFormat::R8_G8_B8_A8_Unsigned */
  ) :
    memory(makeBitmapMemory(width, height, pixelFormat)),
    buffer(newSharedBuffer(getRequiredBufferSize(width, height, pixelFormat), pixelFormat)) {

    this->memory.Stride = determineStride(width, pixelFormat);
    this->memory.Pixels = this->buffer->Memory;
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::Bitmap(const Bitmap &other) :
    memory(other.memory),
    buffer(newSharedBuffer(other.memory)) {

    this->memory.Stride = determineStride(this->memory.Width, this->memory.PixelFormat);
    this->memory.Pixels = this->buffer->Memory;
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::Bitmap(Bitmap &&other) :
    memory(other.memory),
    buffer(other.buffer) {
    other.buffer = nullptr;
#if !defined(NDEBUG)
    other.memory.Pixels = nullptr;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::~Bitmap() {
    if(this->buffer != nullptr) {
      releaseSharedBuffer(this->buffer);
    }

#if !defined(NDEBUG)
    this->buffer = nullptr;
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::Bitmap(SharedBuffer *buffer, const BitmapMemory &memory) :
    memory(memory),
    buffer(buffer) {}

  // ------------------------------------------------------------------------------------------- //

  void Bitmap::Autonomize() {
    if(this->buffer->OwnerCount > 1) { // Only autonomize if the bitmap still has other owners
      Bitmap::SharedBuffer *oldBuffer = this->buffer;
      this->buffer = newSharedBuffer(this->memory);
      --oldBuffer->OwnerCount;

      this->memory.Stride = determineStride(this->memory.Width, this->memory.PixelFormat);
      this->memory.Pixels = this->buffer->Memory;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap Bitmap::GetView(
    std::size_t x, std::size_t y, std::size_t width, std::size_t height
  ) {
    BitmapMemory viewMemory;
    viewMemory.Width = width;
    viewMemory.Height = height;
    viewMemory.Stride = this->memory.Stride;
    viewMemory.PixelFormat = this->memory.PixelFormat;
    viewMemory.Pixels = reinterpret_cast<void *>(
      reinterpret_cast<std::uint8_t *>(this->memory.Pixels) +
      (y * this->memory.Stride) +
      CountRequiredBytes(this->memory.PixelFormat, x)
    );

    // Assumption: allocation-free Bitmap constructor will not throw.
    ++this->buffer->OwnerCount;
    return Bitmap(this->buffer, viewMemory);
  }

  // ------------------------------------------------------------------------------------------- //

  void Bitmap::ReinterpretPixelFormat(PixelFormat newPixelFormat) {
    std::size_t currentBitsPerPixel = CountBitsPerPixel(this->memory.PixelFormat);
    std::size_t newBitsPerPixel = CountBitsPerPixel(newPixelFormat);
    if(newBitsPerPixel != currentBitsPerPixel) {
      throw Errors::PixelFormatError(
        u8"Cannot reinterpret as a pixel format with different bits per pixel"
      );
    }

    // It's that simple :)
    this->memory.PixelFormat = newPixelFormat;
  }

  // ------------------------------------------------------------------------------------------- //

  const Bitmap Bitmap::GetView(
    std::size_t x, std::size_t y, std::size_t width, std::size_t height
  ) const {
    BitmapMemory viewMemory;
    viewMemory.Width = width;
    viewMemory.Height = height;
    viewMemory.Stride = this->memory.Stride;
    viewMemory.PixelFormat = this->memory.PixelFormat;
    viewMemory.Pixels = reinterpret_cast<void *>(
      reinterpret_cast<std::uint8_t *>(this->memory.Pixels) +
      (y * this->memory.Stride) +
      CountRequiredBytes(this->memory.PixelFormat, x)
    );

    // Assumption: allocation-free Bitmap constructor will not throw.
    ++this->buffer->OwnerCount;
    return Bitmap(this->buffer, viewMemory);
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap &Bitmap::operator =(const Bitmap &other) {
    if(this->buffer != nullptr) {
      releaseSharedBuffer(this->buffer);
    }

    this->buffer = other.buffer;
    ++this->buffer->OwnerCount;

    this->memory = other.memory;

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap &Bitmap::operator =(Bitmap &&other) {
    if(this->buffer != nullptr) {
      releaseSharedBuffer(this->buffer);
    }

    this->buffer = other.buffer;
    this->memory = other.memory;

    other.buffer = nullptr;
    #if !defined(NDEBUG)
    other.memory.Pixels = nullptr;
    #endif

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::SharedBuffer *Bitmap::newSharedBuffer(const Size &resolution, PixelFormat pixelFormat) {

    // Calculate the (aligned) space reserved for the detachable buffer structure
    const std::size_t Alignment = 16;
    std::size_t headerSize = nextMultiple(sizeof(SharedBuffer), Alignment);

    // Add the amount of memory required to store the bitmap
    std::size_t byteCount = headerSize;
    byteCount += CountRequiredBytes(pixelFormat, resolution.Width) * resolution.Height;

    // Allocate memory to hold the detachable buffer AND the pixel data,
    // then construct the detachable buffer in it and set the address of the first pixel
    // to the memory behind the detachable buffer.
    std::uint8_t *memory = new std::uint8_t[byteCount];
    SharedBuffer *buffer = new(memory) SharedBuffer();
    buffer->OwnerCount = 1;
    buffer->Memory = memory + headerSize;

    return buffer;

  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap::SharedBuffer *Bitmap::newSharedBuffer(const BitmapMemory &memory) {
    Size bufferSize = getRequiredBufferSize(memory.Width, memory.Height, memory.PixelFormat);
    std::unique_ptr<Bitmap::SharedBuffer, void(*)(Bitmap::SharedBuffer *)> newBuffer(
      newSharedBuffer(bufferSize, memory.PixelFormat), releaseSharedBuffer
    );

    // Copy all pixels into the new buffer
    {
      int newStride = static_cast<int>(CountRequiredBytes(memory.PixelFormat, bufferSize.Width));

      const std::uint8_t *source = reinterpret_cast<std::uint8_t *>(memory.Pixels);
      std::uint8_t *target = reinterpret_cast<std::uint8_t *>(newBuffer->Memory);

      // Do the copy line-by-line because stride may be different
      for(std::size_t index = 0; index < bufferSize.Height; ++index) {
        std::copy_n(source, newStride, target);
        source += memory.Stride;
        target += newStride;
      }
    }

    return newBuffer.release();
  }

  // ------------------------------------------------------------------------------------------- //

  void Bitmap::releaseSharedBuffer(Bitmap::SharedBuffer *sharedBuffer) throw() {
    if(sharedBuffer->OwnerCount == 1) {
      std::uint8_t *memory = reinterpret_cast<std::uint8_t *>(sharedBuffer);
      sharedBuffer->~SharedBuffer();
      delete memory;
    } else {
      --sharedBuffer->OwnerCount;
    }
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
