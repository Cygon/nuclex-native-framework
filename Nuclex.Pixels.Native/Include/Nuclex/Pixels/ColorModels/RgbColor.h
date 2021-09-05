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

#ifndef NUCLEX_PIXELS_COLORMODELS_RGBCOLOR_H
#define NUCLEX_PIXELS_COLORMODELS_RGBCOLOR_H

#include "Nuclex/Pixels/Config.h"

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a color value using the RGB color model</summary>
  struct NUCLEX_PIXELS_TYPE RgbColor {

    /// <summary>Normalized (0.0 .. 1.0) amount of red in the color</summary>
    public: float Red;
    /// <summary>Normalized (0.0 .. 1.0) amount of green in the color</summary>
    public: float Green;
    /// <summary>Normalized (0.0 .. 1.0) amount of blue in the color</summary>
    public: float Blue;
    /// <summary>Normalized (0.0 .. 1.0) opacity</summary>
    /// <remarks>
    ///   0.0 is completely transparent (invisible),
    ///   1.0 is fully opaque (everything behind is obscured)
    /// </remarks>
    public: float Alpha;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels

#endif // NUCLEX_PIXELS_COLORMODELS_RGBCOLOR_H
