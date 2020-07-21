#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

namespace Nuclex { namespace Pixels { namespace ColorModels {

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

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels

#endif // NUCLEX_PIXELS_COLORMODELS_COLORMODELCONVERTER_H
