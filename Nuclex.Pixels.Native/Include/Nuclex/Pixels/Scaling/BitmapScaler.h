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

#ifndef NUCLEX_PIXELS_SCALING_BITMAPSCALER_H
#define NUCLEX_PIXELS_SCALING_BITMAPSCALER_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/Bitmap.h"
#include "Nuclex/Pixels/Scaling/ResamplingMethod.h"

namespace Nuclex { namespace Pixels { namespace Scaling {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Scales bitmaps to different resolutions</summary>
  class NUCLEX_PIXELS_TYPE BitmapScaler {

    /// <summary>Resamples the source bitmap to fill the destination bitmap</summary>
    /// <param name="source">Source bitmap that will be resampled to a new size</param>
    /// <param name="target">Target bitmap which the resampling operation will fill</param>
    /// <param name="method">Resampling method that will be used adjust the size</param>
    public: NUCLEX_PIXELS_API static void Rescale(
      const Bitmap &source, Bitmap &target, ResamplingMethod method
    );

    /// <summary>Resamples the source bitmap to the specified resolution</summary>
    /// <param name="source">Source bitmap that will be resampled to a new size</param>
    /// <param name="newResolution">Resolution the bitmap will be resized to</param>
    /// <param name="method">Resampling method that will be used adjust the size</param>
    /// <returns>A resampled version of the input bitmap with the specified resolution</returns>
    public: NUCLEX_PIXELS_API static Bitmap Rescale(
      const Bitmap &source, const Size &newResolution, ResamplingMethod method
    );

    // TODO: Add special fast method for resampling a bitmap to half its size
    //   Useful for generating mip maps in software.

    /// <summary>Resamples the source bitmap to fill the destination bitmap</summary>
    /// <param name="source">Source bitmap that will be resampled to a new size</param>
    /// <param name="target">Target bitmap which the resampling operation will fill</param>
    /// <param name="method">Resampling method that will be used adjust the size</param>
    private: static void rescaleAvir(
      const Bitmap &source, Bitmap &target, ResamplingMethod method
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Scaling

#endif // NUCLEX_PIXELS_SCALING_BITMAPSCALER_H
