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

#ifndef NUCLEX_PIXELS_COLORMODELS_YUVCOLOR_H
#define NUCLEX_PIXELS_COLORMODELS_YUVCOLOR_H

#include "Nuclex/Pixels/Config.h"

namespace Nuclex { namespace Pixels { namespace ColorModels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes a color value using the YUV or Y'UV color model</summary>
  /// <remarks>
  ///   <para>
  ///     This format originated back in the black-and-white TV days, where first only
  ///     luminance (brightness) was transmitted and, to keep backwards compatibility,
  ///     chrominance (color) was latter added as a separate signal that encoded
  ///     U+V coordinates into a 2D color space.
  ///   </para>
  ///   <para>
  ///     As it turned out, this format was also useful for digital signals because
  ///     the human eye is more sensitive to brightness than to color, so separating
  ///     luminance from chroma meant you could use lower-resolution chroma (i.e. only
  ///     store it for every second or fourth pixel and interpolate the ones inbetween.
  ///   </para>
  ///   <para>
  ///     When made <em>digital</em> by quantizing, this format is called YCbCr (storing
  ///     Y, U and V as bytes either from 0..255 (&qout;full swing&quot;) or 16..235
  ///     (studio swing). However, this structure stores analogous values with U and V
  ///     components ranging from -1.0 .. +1.0, so this is actual YUV, not YCbCr.
  ///   </para>
  ///   <para>
  ///     (Or perhaps not, ITU appears to define YUV as &quot;analogue TV&quot; and
  ///     even non-quantized math in BT.2020 refers to the numbers as YCbCr...)
  ///   </para>
  /// </remarks>
  struct YuvColor {

    /// <summary>Normalized (0.0 .. 1.0) luminance (brightness)</summary>
    /// <remarks>
    ///   This channel can either be linear luminance (usually shortened Y) or
    ///   gamma-corrected (perceptual) luma (usually shortened Y').
    /// </remarks>
    public: float Y;
    /// <summary>Normalized (-0.5 .. +0.5) U coordinate on the color plane</summary>
    public: float U;
    /// <summary>Normalized (-0.5 .. +0.5) V coordinate on the color plane</summary>
    public: float V;
    /// <summary>Normalized (0.0 .. 1.0) opacity</summary>
    /// <remarks>
    ///   0.0 is completely transparent (invisible),
    ///   1.0 is fully opaque (everything behind is obscured)
    /// </remarks>
    public: float Alpha;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::ColorModels

#endif // NUCLEX_PIXELS_COLORMODELS_YUVCOLOR_H
