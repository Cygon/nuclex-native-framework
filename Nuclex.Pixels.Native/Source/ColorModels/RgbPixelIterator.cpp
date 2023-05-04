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

#include "Nuclex/Pixels/ColorModels/RgbPixelIterator.h"
#include "../PixelFormats/ConvertPixel.h"
#include "../PixelFormats/OnPixelFormat.h"
#include "Nuclex/Pixels/UInt128.h"

#include <stdexcept> // for std::runtime_error
#include <utility> // for std::pair

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads a pixel and converts it to an RGB color</summary>
  /// <typeparam name="PixelFormat">Pixel format of the bitmap to read a pixel from</typeparam>
  /// <param name="address">Address from which a pixel will be read</param>
  /// <returns>The RGB color of the pixel</returns>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  Nuclex::Pixels::ColorModels::RgbColor readPixel(const void *address) {
    using Nuclex::Pixels::uint128_t;
    typedef Nuclex::Pixels::PixelFormats::PixelTypeFromFormat<TPixelFormat> SourcePixelType;

    uint128_t rgbaColor;
    Nuclex::Pixels::PixelFormats::ConvertPixel<
      TPixelFormat, Nuclex::Pixels::PixelFormat::R32_G32_B32_A32_Float_Native32
    >(
      reinterpret_cast<const SourcePixelType *>(address),
      &rgbaColor
    );

    // CHECK: Is this valid? By not going over a char pointer, does this cause UB or aliasing?
    return Nuclex::Pixels::ColorModels::RgbColor {
      reinterpret_cast<const float *>(&rgbaColor)[0],
      reinterpret_cast<const float *>(&rgbaColor)[1],
      reinterpret_cast<const float *>(&rgbaColor)[2],
      reinterpret_cast<const float *>(&rgbaColor)[3]
    };
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Function that converts an RGB color to the pixel format and writes it</summary>
  /// <typeparam name="PixelFormat">Pixel format of the bitmap to write a pixel in</typeparam>
  /// <param name="address">Address at which the pixel will be written</param>
  /// <param name="color">RGB color whose equivalent the pixel will assume</param>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  void writePixel(void *address, const Nuclex::Pixels::ColorModels::RgbColor &color) {
    using Nuclex::Pixels::uint128_t;
    typedef Nuclex::Pixels::PixelFormats::PixelTypeFromFormat<TPixelFormat> TargetPixelType;

    Nuclex::Pixels::PixelFormats::ConvertPixel<
      Nuclex::Pixels::PixelFormat::R32_G32_B32_A32_Float_Native32, TPixelFormat
    >(
      reinterpret_cast<const uint128_t *>(&color),
      reinterpret_cast<TargetPixelType *>(address)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Provides read and write methods for the specified pixel format</summary>
  /// <typeparam name="TPixelFormat">
  ///   Pixel format for which read and write methods will be provided
  /// </typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class GetReadAndWriteMethods {

    /// <summary>Function that reads a pixel and converts it to an RGB color</summary>
    /// <param name="address">Address from which a pixel will be read</param>
    /// <returns>The RGB color of the pixel</returns>
    private: typedef Nuclex::Pixels::ColorModels::RgbColor ReadPixelFunction(
      const void *address
    );

    /// <summary>Function that converts an RGB color to the pixel format and writes it</summary>
    /// <param name="address">Address at which the pixel will be written</param>
    /// <param name="color">RGB color whose equivalent the pixel will assume</param>
    private: typedef void WritePixelFunction(
      void *address, const Nuclex::Pixels::ColorModels::RgbColor &color
    );

    /// <summary>Returns the number of blue bits used by the pixel format</summary>
    /// <returns>The number of bits used for the blue channel in the pixel format</returns>
    public: std::pair<ReadPixelFunction *, WritePixelFunction *> operator()() const {
      return std::pair<ReadPixelFunction *, WritePixelFunction *>(
        &readPixel<TPixelFormat>, &writePixel<TPixelFormat>
      );
    }

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  RgbPixelIterator::RgbPixelIterator(const BitmapMemory &memory) :
    PixelIterator(memory),
    readPixel(nullptr),
    writePixel(nullptr) {

    std::pair<ReadPixelFunction *, WritePixelFunction *> readAndWriteFunctions = (
      PixelFormats::OnPixelFormat<
        GetReadAndWriteMethods,
        std::pair<ReadPixelFunction *, WritePixelFunction *>
      >(memory.PixelFormat)
    );
    this->readPixel = readAndWriteFunctions.first;
    this->writePixel = readAndWriteFunctions.second;
  }

  // ------------------------------------------------------------------------------------------- //

#if defined(GO_AWAY)
  /// <summary>Returns the pixel at the iterator's current position</summary>
  const PixelIterator::Pixel PixelIterator::operator *() {
    if(this->x >= this->memory.Width) {
      throw std::out_of_range("Pixel access iterator is not on a valid position");
    }

    // TODO: Read pixel from bitmap
    return Pixel();
  }
#endif

  // ------------------------------------------------------------------------------------------- //
/*
  void RgbPixelIterator::Update(const Pixel &pixel) {
    void *currentPixelAddress = GetAddress();


  }
*/
  // ------------------------------------------------------------------------------------------- //

#if defined(GO_AWAY)
  float PixelIterator::GetLuminance() const {
    // TODO: Read pixel and determine luminance
  }
#endif

  // ------------------------------------------------------------------------------------------- //

#if defined(THIS_DOES_NOT_MATCH_EXPECTATIONS)
  /// <summary>Updates a pixel in the bitmap to the specified color</summary>
  /// <param name="pixel">
  ///   Pixel that will be placed at the iterator's current position
  /// </param>
  /// <returns>The pixel accessor at its current position</returns>
  PixelIterator &PixelIterator::operator =(const PixelIterator::Pixel &pixel) {
    if(this->x >= this->memory.Width) {
      throw std::out_of_range("Pixel access iterator is not on a valid position");
    }

    // TODO: Assign pixel to bitmap

    return *this;
  }
#endif

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels
