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

#ifndef NUCLEX_PIXELS_SCALING_RESAMPLINGMETHOD_H
#define NUCLEX_PIXELS_SCALING_RESAMPLINGMETHOD_H

#include "Nuclex/Pixels/Config.h"

namespace Nuclex { namespace Pixels { namespace Scaling {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Methods that can be used to scale bitmaps to different resolutions</summary>
  enum class ResamplingMethod {

    /// <summary>Cheap but fast method that picks the nearest source pixel</summary>
    /// <remarks>
    ///   Old school method. The only reason to use this is probably when you want to
    ///   see how bad the resized image could have looked.
    /// </remarks>
    Nearest, // < not implemented yet

    /// <summary>Uses the Catmull-Rom curve to interpolate surrounding pixels</summary>
    /// <remarks>
    ///   This one is popular for movie and game footage upscaling because it looks less
    ///   blurry than bilinear filtering and still hides aliasing artifacts.
    /// </remarks>
    CatmullRom, // < not implemented yet

    /// <summary>Lanczos filter resampling using the embedded AVIR library code</summary>
    /// <remarks>
    ///   This is a popular filter for image upscaling because it emulates the theoretically
    ///   optimal sinc filter closely.
    /// </remarks>
    Lanczos3, // < not implemented yet

    /// <summary>Smooth resample method using the embedded AVIR library code</summary>
    /// <remarks>
    ///   This avoids ringing artifacts, but is prone to blurring sharp features in
    ///   the image. Not ideal for text and UI.
    /// </remarks>
    AvirSmooth,

    /// <summary>Balanced resample method using the embedded AVIR library code</summary>
    /// <remarks>
    ///   The default AVIR variant tries to find a good compromise between the risk
    ///   for ringing artifacts and blurry images.
    /// </remarks>
    Avir,

    /// <summary>Sharp resample method using the embedded AVIR library code</summary>
    /// <remarks>
    ///   This can produce ringing artifacts near sharp features (high-contrast edges in
    ///   the image may develop a halo or shadow around them)
    /// </remarks>
    AvirSharp

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Scaling

#endif // NUCLEX_PIXELS_SCALING_RESAMPLINGMETHOD_H
