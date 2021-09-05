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

#ifndef NUCLEX_PIXELS_COLORMODELS_HSVCOLOR_H
#define NUCLEX_PIXELS_COLORMODELS_HSVCOLOR_H

#include "Nuclex/Pixels/Config.h"

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a color value using the HSV color model</summary>
  struct NUCLEX_PIXELS_TYPE HsvColor {

    /// <summary>Hue value that will result in the color red</summary>
    public: static constexpr float RedHue = (
      0.0f
    );

    /// <summary>Hue value that will result in the color yellow</summary>
    public: static constexpr float YellowHue = (
      1.047197551196597746154214461093167628065723133125035273658314864102605468762069666209344942f
    );

    /// <summary>Hue value that will result in the color green</summary>
    public: static constexpr float GreenHue = (
      2.094395102393195492308428922186335256131446266250070547316629728205210937524139332418689884f
    );

    /// <summary>Hue value that will result in the color cyan</summary>
    public: static constexpr float CyanHue = (
      3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825f
    );

    /// <summary>Hue value that will result in the color blue</summary>
    public: static constexpr float BlueHue = (
      4.188790204786390984616857844372670512262892532500141094633259456410421875048278664837379767f
    );

    /// <summary>Hue value that will result in the color magenta</summary>
    public: static constexpr float MagentaHue = (
      5.235987755982988730771072305465838140328615665625176368291574320513027343810348331046724709f
    );

    /// <summary>
    ///   Index into colors of the visible light spectrum as a loop (0.0 .. 2x PI)
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     Maps the colors of the visible light spectrum into an index from 0.0 to 2x PI
    ///     where 2x PI is identical to 0.0 (so ultraviolet and infrared are imposssible to
    ///     specify, instead blue interpolates back into red).
    ///   </para>
    ///   <para>
    ///     0.00     (0°) = red
    ///     1/3 PI  (60°) = yellow
    ///     2/3 PI (120°) = green
    ///     PI     (180°) = cyan
    ///     4/3 PI (240°) = blue
    ///     5/3 PI (300°) = magenta
    ///   </para>
    /// </remarks>
    public: float Hue;

    /// <summary>How saturated the color is (0.0 .. 1.0)</summary>
    /// <remarks>
    ///   0.0 is monochrome (either black or white depending on value)
    ///   1.0 is only color (either black or full color depending on value)
    /// </remarks>
    public: float Saturation;

    /// <summary>How strongly the color is expressed (0.0 .. 1.0)</summary>
    /// <remarks>
    ///   0.0 is always black
    ///   1.0 is the full color (but not white as in the HSL color model)
    /// </remarks>
    public: float Value;

    /// <summary>Normalized (0.0 .. 1.0) opacity</summary>
    /// <remarks>
    ///   0.0 is completely transparent (invisible),
    ///   1.0 is fully opaque (everything behind is obscured)
    /// </remarks>
    public: float Alpha;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels

#endif // NUCLEX_PIXELS_COLORMODELS_HSVCOLOR_H
