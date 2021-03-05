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

#ifndef NUCLEX_PIXELS_BITMAP_H
#define NUCLEX_PIXELS_BITMAP_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/BitmapMemory.h"

#include <cstddef>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Raster-based image of variable size and color depth</summary>
  /// <remarks>
  ///   <para>
  ///     This bitmap implementation attempts to be a very thin wrapper around a block
  ///     of raw memory. The layout and pixel format of the memory area is completely
  ///     described by the bitmap, making it the ideal basic building block for storing
  ///     and passing raster images.
  ///   </para>
  ///   <para>
  ///     Bitmaps can be created on their own (in which case they will maintain their
  ///     own memory) or they can be set up to use an externally provided memory buffer
  ///     (whose the lifetime must be managed separately).
  ///   </para>
  ///   <para>
  ///     Another option is to create bitmaps as views into other bitmaps. In this case
  ///     ownership of the memory block will be shared (memory stays allocated until
  ///     the last bitmap referencing it is destroyed). A manually triggered copy-on-write
  ///     system allows a bitmaps to make itself autonomous, allocating its own memory
  ///     block and storing a unique copy of all pixels it was referencing.
  ///   </para>
  /// </remarks>
  class Bitmap {

    /// <summary>Creates a bitmap that accesses an existing memory area</summary>
    /// <param name="bitmapMemory">
    ///   Description of the existing memory area that will be accessed
    /// </param>
    /// <returns>A bitmap that accesses pixels in the provided existing memory area</returns>
    /// <remarks>
    ///   Ownership of the memory area is not transferred to the bitmap. Destroying
    ///   the bitmap will thus not free the existing memory. If you want an autonomous
    ///   bitmap that is initialized from an existing memory buffer, immediately
    ///   call the <see cref="Autonomize" /> method on it.
    /// </remarks>
    public: NUCLEX_PIXELS_API static Bitmap FromExistingMemory(const BitmapMemory &bitmapMemory);

    /// <summary>Initializes a new bitmap</summary>
    /// <param name="width">Width of the bitmap in pixels</param>
    /// <param name="height">Height of the bitmap in pixels</param>
    /// <param name="pixelFormat">Pixel format in which the pixels will be stored</param>
    public: NUCLEX_PIXELS_API Bitmap(
      std::size_t width,
      std::size_t height,
      PixelFormat pixelFormat = PixelFormat::R8_G8_B8_A8_Unsigned
    );

    /// <summary>Constructs an bitmap as a copy of an existing bitmap</summary>
    /// <param name="other">Bitmap that will be copied</param>
    public: NUCLEX_PIXELS_API Bitmap(const Bitmap &other);

    /// <summary>Constructs an bitmap by taking over an existing bitmap</summary>
    /// <param name="other">Bitmap that will be taken over</param>
    public: NUCLEX_PIXELS_API Bitmap(Bitmap &&other);

    /// <summary>Frees all resources owned by the bitmap instance</summary>
    public: NUCLEX_PIXELS_API virtual ~Bitmap();

    /// <summary>Returns the width of the bitmap in pixels</summary>
    /// <returns>The width of the bitmap in pixels</returns>
    public: NUCLEX_PIXELS_API std::size_t GetWidth() const {
      return this->memory.Width;
    }

    /// <summary>Returns the height of the bitmap in pixels</summary>
    /// <returns>The height of the bitmap in pixels</returns>
    public: NUCLEX_PIXELS_API std::size_t GetHeight() const {
      return this->memory.Height;
    }

    /// <summary>Returns the pixel format in which the pixels are stored</summary>
    /// <returns>The pixel format used to store the bitmap's pixels</returns>
    public: NUCLEX_PIXELS_API PixelFormat GetPixelFormat() const {
      return this->memory.PixelFormat;
    }

    /// <summary>Accesses the bitmap's pixels</summary>
    /// <returns>A description of the bitmap's memory layout</returns>
    public: NUCLEX_PIXELS_API const BitmapMemory &Access() const {
      return this->memory;
    }

    /// <summary>
    ///   If the bitmap is sharing memory with another bitmap, forces it to create
    ///   its own copy of the image data
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     When creating a bitmap as a view into another bitmap (think of it as
    ///     bitmap-inside-a-bitmap), the view can be set to keep accessing the same
    ///     memory buffer, thus changing pixels in the view also changes them in
    ///     the bitmap the view was created in.
    ///   </para>
    ///   <para>
    ///     This method will clone all pixels accessible by the bitmap so that
    ///     it has its own standalone memory buffer.
    ///   </para>
    ///   <para>
    ///     It can also be used on bitmaps accessing externally managed memory to
    ///     clone the external memory and become independent of it.
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API void Autonomize();

    /// <summary>Creates a bitmap that accesses a region within this bitmap</summary>
    /// <param name="x">X coordinate of the region's left border</param>
    /// <param name="y">Y coordinate of the region's top border</param>
    /// <param name="width">Width of the region</param>
    /// <param name="height">Height of the region</param>
    /// <returns>A bitmap that accesses a region within this bitmap</returns>
    /// <remarks>
    ///   <para>
    ///     The region will still be using the memory of this bitmap, so changes
    ///     to the sub-bitmap will change the parent. This can be useful for clipping
    ///     and atlases.
    ///   </para>
    ///   <para>
    ///     If the parent bitmap gets destroyed, the sub-bitmap will still hold onto
    ///     the whole memory buffer. In this case, the <see cref="IsWastingMemory" />
    ///     method will return true.
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API const Bitmap GetView(
      std::size_t x, std::size_t y, std::size_t width, std::size_t height
    ) const;

    /// <summary>Creates a bitmap that accesses a region within this bitmap</summary>
    /// <param name="x">X coordinate of the region's left border</param>
    /// <param name="y">Y coordinate of the region's top border</param>
    /// <param name="width">Width of the region</param>
    /// <param name="height">Height of the region</param>
    /// <returns>A bitmap that accesses a region within this bitmap</returns>
    /// <remarks>
    ///   <para>
    ///     The region will still be using the memory of this bitmap, so changes
    ///     to the sub-bitmap will change the parent. This can be useful for clipping
    ///     and atlases.
    ///   </para>
    ///   <para>
    ///     If the parent bitmap gets destroyed, the sub-bitmap will still hold onto
    ///     the whole memory buffer. In this case, the <see cref="IsWastingMemory" />
    ///     method will return true.
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API Bitmap GetView(
      std::size_t x, std::size_t y, std::size_t width, std::size_t height
    );
#if 0
    /// <summary>Returns the amount of memory used by the bitmap</summary>
    /// <returns>The number of bytes used to store the bitmap's pixels</returns>
    /// <remarks>
    ///   <para>
    ///     The number of bytes used by the bitmap to store its pixels. This number
    ///     can be slightly higher than the actual memory required for a tight fit
    ///     due to alignment and a header, or much higher if the bitmap is a region
    ///     within a larger bitmap.
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API std::size_t GetMemoryUsed() const;

    public: NUCLEX_PIXELS_API bool IsWastingMemory() const;
#endif
    /// <summary>Copies another bitmap instance into this one</summary>
    /// <param name="other">Other bitmap instance that will be copied</param>
    /// <returns>This bitmap instance</returns>
    public: NUCLEX_PIXELS_API Bitmap &operator =(const Bitmap &other);

    /// <summary>Takes over another bitmap instance</summary>
    /// <param name="other">Other bitmap instance that will be taken over</param>
    /// <returns>This bitmap instance</returns>
    public: NUCLEX_PIXELS_API Bitmap &operator =(Bitmap &&other);

    /// <summary>Detachable memory buffer that allows for shared ownership</summary>
    private: struct SharedBuffer;

    /// <summary>Initializes a bitmap used the specified buffer and memory description</summary>
    /// <param name="buffer">Buffer that will be used by the new bitmap</param>
    /// <param name="memory">Description of the bitmap's memory layout</param>
    private: Bitmap(SharedBuffer *buffer, const BitmapMemory &memory);

    /// <summary>Creates a new detachable buffer for a bitmap of the specified size</summary>
    /// <param name="resolution">Resolution for which a buffer will be created</param>
    /// <param name="pixelFormat">Pixel format usd by the bitmap</param>
    private: static SharedBuffer *newSharedBuffer(
      const Size &resolution, PixelFormat pixelFormat
    );

    /// <summary>Creates a new detachable buffer by copying an existing buffer</summary>
    /// <param name="memory">Existing bitmap memory that will be copied</param>
    /// <returns>A new detachable buffer with a copy of the existing buffer's contents</returns>
    private: static SharedBuffer *newSharedBuffer(const BitmapMemory &memory);

    /// <summary>Releases a shared buffer, freeing its memory if possible</summary>
    /// <param name="buffer">Buffer that will be released</param>
    private: static void releaseSharedBuffer(SharedBuffer *buffer) throw();

    /// <summary>Description of the memory allocated for the bitmaps and its layout</summary>
    private: BitmapMemory memory; 
    /// <summary>Memory buffer holding or accessing the bitmap's pixels</summary>
    private: SharedBuffer *buffer;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_BITMAP_H
