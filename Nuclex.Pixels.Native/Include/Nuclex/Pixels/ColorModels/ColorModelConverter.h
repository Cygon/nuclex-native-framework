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

#ifndef NUCLEX_PIXELS_COLORMODELS_COLORMODELCONVERTER_H
#define NUCLEX_PIXELS_COLORMODELS_COLORMODELCONVERTER_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/ColorModels/RgbColor.h"
#include "Nuclex/Pixels/ColorModels/HsvColor.h"
#include "Nuclex/Pixels/ColorModels/HslColor.h"
#include "Nuclex/Pixels/ColorModels/YuvColor.h"

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>System definiting the color rectangle for the U+V parts of YUV</summary>
  enum class YuvColorSystem {

    /// <summary>BT.470 also known as SDTV, barely covering common colors</summary>
    /// <remarks>
    ///   Used by old analog systems like PAL and NTSC.
    /// </remarks>
    Bt470,

    /// <summary>BT.709 also known as HDTV with limited green and blue tones</summary>
    /// <remarks>
    ///   This covers about 35% of the colors perceptible by the human eye (CIE 1931).
    ///   At this time, it appears to be the most common format and most YUV or YCbCr
    ///   conversion functions you can find on the net seem to use BT.709 constants
    ///   (with BT.470 a close second).
    /// </remarks>
    Bt709,

    /// <summary>BT.2020 also known as UHDTV and able to represent most colors</summary>
    /// <remarks>
    ///   <para>
    ///     This covers about 76% of the colors perceptible by the human eye (CIE 1931).
    ///     Confusingly, there are claims this specification is digital (as in, quantized)
    ///     only, but the RGB conversion formula is of course present in the specification
    ///     as pure math with fractions in real numbers.
    ///   </para>
    ///   <para>
    ///     Still, the specification talks about YCbCr so maybe ITU wants YUV to mean
    ///     analoguously transmitted (rather than &quot;not quantized&quot;) and bury
    ///     the whole term. Which means my YuvColor class should be YCbCrColor still :/
    /// </remarks>
    Bt2020

    // There's a BT.2100 specification that seems to define an even larger color space.

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Converts colors between different color models</summary>
  class ColorModelConverter {

    /// <summary>Converts an RGB color value to an HSV color value</summary>
    /// <param name="color">RGB color that will be converted</param>
    /// <returns>The equivalent HSV color</returns>
    public: NUCLEX_PIXELS_API static HsvColor HsvFromRgb(const RgbColor &color);

    /// <summary>Converts an HSV color value to an RGB color value</summary>
    /// <param name="color">HSV color that will be converted</param>
    /// <returns>The equivalent RGB color</returns>
    public: NUCLEX_PIXELS_API static RgbColor RgbFromHsv(const HsvColor &color);

    /// <summary>Converts an RGB color value to an HSL color value</summary>
    /// <param name="color">RGB color that will be converted</param>
    /// <returns>The equivalent HSL color</returns>
    public: NUCLEX_PIXELS_API static HslColor HslFromRgb(const RgbColor &color);

    /// <summary>Converts an HSL color value to an RGB color value</summary>
    /// <param name="color">HSL color that will be converted</param>
    /// <returns>The equivalent RGB color</returns>
    public: NUCLEX_PIXELS_API static RgbColor RgbFromHsl(const HslColor &color);

    /// <summary>Converts an HSV color value to an HSL color value</summary>
    /// <param name="color">HSV color that will be converted</param>
    /// <returns>The equivalent HSL color</returns>
    public: NUCLEX_PIXELS_API static HslColor HslFromHsv(const HsvColor &color);

    /// <summary>Converts an HSL color value to an HSV color value</summary>
    /// <param name="color">HSL color that will be converted</param>
    /// <returns>The equivalent HSV color</returns>
    public: NUCLEX_PIXELS_API static HsvColor HsvFromHsl(const HslColor &color);

    /// <summary>Converts an RGB color value to an YUV color value</summary>
    /// <param name="color">RGB color that will be converted (gamma-corrected)</param>
    /// <param name="colorSystem">What color ranged are mapped into the U/V plane</param>
    /// <returns>The equivalent YUV color in the specified color system</returns>
    public: NUCLEX_PIXELS_API static YuvColor YuvFromRgb(
      const RgbColor &color, YuvColorSystem colorSystem = YuvColorSystem::Bt709
    );

    /// <summary>Converts an YUV color value to an RGB color value</summary>
    /// <param name="color">YUV color that will be converted</param>
    /// <param name="colorSystem">What color ranged are mapped into the U/V plane</param>
    /// <returns>The equivalent RGB color (gamma-corrected)</returns>
    public: NUCLEX_PIXELS_API static RgbColor RgbFromYuv(
      const YuvColor &color, YuvColorSystem colorSystem = YuvColorSystem::Bt709
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels

#endif // NUCLEX_PIXELS_COLORMODELS_COLORMODELCONVERTER_H
