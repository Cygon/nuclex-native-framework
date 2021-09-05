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

#ifndef NUCLEX_PIXELS_COLORMODELS_RGBPIXELITERATOR_H
#define NUCLEX_PIXELS_COLORMODELS_RGBPIXELITERATOR_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelIterator.h"
#include "Nuclex/Pixels/ColorModels/RgbColor.h"

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Moves through an image's pixels and allows them to be accessed in uniform format
  /// </summary>
  /// <remarks>
  ///   <para>
  ///     The RGB pixel accessor is a convenient way to process and alter pixels in an image
  ///     without having to deal with different pixel formats and color channels. All channels
  ///     are converted to normalized floating point values using hand-optimized pixel format
  ///     translation code, making it easy to apply filters, draw or do post-processing.
  ///   </para>
  ///   <para>
  ///     Converting color channels to floats and back does carry a certain overhead.
  ///     The recommended use of the pixel accessor is therefore for offline processing and
  ///     other non-time critical tasks. It can be used as a fallback and for prototyping in
  ///     real-time scenarios, using optimized code paths that work directly on the target
  ///     pixel formats for those formats you know you will be encountering.
  ///   </para>
  ///   <para>
  ///     By design, the pixel access iterator only requires a <see cref="BitmapMemory" />
  ///     instance to work on, so that it can be used on various targets, including locked
  ///     textures of 3D rendering APIs or bitmaps in various windowing systems.
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE RgbPixelIterator : public PixelIterator {

    /// <summary>Function that reads a pixel and converts it to an RGB color</summary>
    /// <param name="address">Address from which a pixel will be read</param>
    /// <returns>The RGB color of the pixel</returns>
    private: typedef RgbColor ReadPixelFunction(const void *address);

    /// <summary>Function that converts an RGB color to the pixel format and writes it</summary>
    /// <param name="address">Address at which the pixel will be written</param>
    /// <param name="color">RGB color whose equivalent the pixel will assume</param>
    private: typedef void WritePixelFunction(void *address, const RgbColor &color);

    #pragma region struct PixelProxy

    /// <summary>Accesses an individual pixel the iterator is pointing at</summary>
    public: struct PixelProxy {
      friend RgbPixelIterator;

      /// <summary>Initializes a new pixel proxy at the specified address</summary>
      /// <param name="readPixel">Function used to read the pixel colors</param>
      /// <param name="writePixel">Function used to write pixel colors</param>
      /// <param name="address">Address at which the accessed pixel is stored</param>
      protected: NUCLEX_PIXELS_API PixelProxy(
        ReadPixelFunction *readPixel, WritePixelFunction *writePixel, void *address
      ) :
        readPixel(readPixel),
        writePixel(writePixel),
        address(address) {}

      /// <summary>Changes the pixel to the specified color</summary>
      /// <param name="color">Color to which the pixel will be changed</param>
      /// <returns>The pixel accessor at its current position</returns>
      public: NUCLEX_PIXELS_API PixelProxy &operator =(const RgbColor &color) {
        this->writePixel(this->address, color);
        return *this;
      }

      /// <summary>Reads the current color of the pixel</summary>
      /// <returns>The current color of the pixel</returns>
      public: NUCLEX_PIXELS_API operator RgbColor() const {
        return this->readPixel(this->address);
      }

      /// <summary>Reads the current color of the pixel and accesses its elements</summary>
      /// <returns>The current color of the pixel</returns>
      public: NUCLEX_PIXELS_API RgbColor operator ->() const {
        return this->readPixel(this->address);
      }

      /// <summary>Function used for reading pixels</summary>
      private: ReadPixelFunction *readPixel;
      /// <summary>Function used for writing pixels</summary>
      private: WritePixelFunction *writePixel;
      /// <summary>Address the pixel is stored at</summary>
      protected: void *address;

    };

    #pragma endregion // struct PixelProxy

    #pragma region struct ConstPixelProxy

    /// <summary>Accesses an individual pixel the iterator is pointing at</summary>
    public: struct ConstPixelProxy {
      friend RgbPixelIterator;

      /// <summary>Initializes a new pixel proxy at the specified address</summary>
      /// <param name="readPixel">Function used to read the pixel colors</param>
      /// <param name="address">Address at which the accessed pixel is stored</param>
      protected: NUCLEX_PIXELS_API ConstPixelProxy(
        ReadPixelFunction *readPixel, const void *address
      ) :
        readPixel(readPixel),
        address(address) {}

      /// <summary>Reads the current color of the pixel</summary>
      /// <returns>The current color of the pixel</returns>
      public: NUCLEX_PIXELS_API operator RgbColor() const {
        return this->readPixel(this->address);
      }

      /// <summary>Reads the current color of the pixel and accesses its elements</summary>
      /// <returns>The current color of the pixel</returns>
      public: NUCLEX_PIXELS_API RgbColor operator ->() const {
        return this->readPixel(this->address);
      }

      /// <summary>Function used for reading pixels</summary>
      private: ReadPixelFunction *readPixel;
      /// <summary>Address the pixel is stored at</summary>
      protected: const void *address;

    };

    #pragma endregion // struct ConstPixelProxy

    /// <summary>Type that results when the distance of two iterators is calculated</summary>
    public: using PixelIterator::difference_type;
    //public: typedef std::ptrdiff_t difference_type;

    /// <summary>Type of the values this iterator iterates over</summary>
    public: typedef RgbColor value_type;

    /// <summary>Reference to an element addressed by this iterator</summary>
    public: typedef RgbColor &reference;

    /// <summary>Type of pointer this iterator emulates</summary>
    public: typedef RgbColor *pointer;

    /// <summary>Which type of iterator this is</summary>
    public: using PixelIterator::iterator_category;
    //public: typedef std::random_access_iterator_tag iterator_category;

    /// <summary>Initializes a new pixel accessor for the specified bitmap memory</summary>
    /// <param name="memory">Bitmap memory the pixel accessor will work on</param>
    public: NUCLEX_PIXELS_API RgbPixelIterator(const BitmapMemory &memory);

    /// <summary>Returns the memory address of the iterator's current position</summary>
    /// <returns>The memory address the pixel iterator is currently at</returns>
    public: NUCLEX_PIXELS_API PixelProxy operator *() {
      return PixelProxy(this->readPixel, this->writePixel, GetAddress());
    }

    /// <summary>Returns the memory address of the iterator's current position</summary>
    /// <returns>The memory address the pixel iterator is currently at</returns>
    public: NUCLEX_PIXELS_API const ConstPixelProxy operator *() const {
      return ConstPixelProxy(this->readPixel, GetAddress());
    }

/*
    /// <summary>Writes a pixel to the iterator's current position</summary>
    /// <param name="pixel">Pixel that will be stored at the iterator's current position</param>
    public: NUCLEX_PIXELS_API void Update(const RgbColor &pixel);
*/

    /// <summary>Function used for reading pixels in the bitmap's pixel format</summary>
    private: ReadPixelFunction *readPixel;
    /// <summary>Function used for writing pixels in the bitmap's pixel format</summary>
    private: WritePixelFunction *writePixel;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels

#endif // NUCLEX_PIXELS_COLORMODELS_RGBPIXELITERATOR_H
