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

#include "Nuclex/Pixels/Scaling/BitmapScaler.h"
#include "Nuclex/Pixels/Errors/PixelFormatError.h"

#include "Nuclex/Pixels/PixelFormats/PixelFormatConverter.h"
#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using nearest-neighbor sampling</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  void resampleNearest(
    const Nuclex::Pixels::BitmapMemory &source, const Nuclex::Pixels::BitmapMemory &target
  ) {
    (void)source;
    (void)target;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using Catmull-Rom interpolation</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  void resampleCatmullRom(
    const Nuclex::Pixels::BitmapMemory &source, const Nuclex::Pixels::BitmapMemory &target
  ) {
    (void)source;
    (void)target;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Resamples an image using a 3-lobe Lanczos filter</summary>
  /// <param name="source">Description of the memory block holding the source bitmap</param>
  /// <param name="target">Description of the memory block holding the target bitmap</param>
  void resampleLanczos3(
    const Nuclex::Pixels::BitmapMemory &source, const Nuclex::Pixels::BitmapMemory &target
  ) {
    (void)source;
    (void)target;
    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Scaling {

  // ------------------------------------------------------------------------------------------- //

  void BitmapScaler::Rescale(
    const Bitmap &source, Bitmap &target, ResamplingMethod method
  ) {
    switch(method) {
      case ResamplingMethod::Nearest: {
        resampleNearest(source.Access(), target.Access());
        break;
      }
      case ResamplingMethod::CatmullRom: {
        resampleCatmullRom(source.Access(), target.Access());
        break;
      }
      case ResamplingMethod::Lanczos3: {
        resampleLanczos3(source.Access(), target.Access());
        break;
      }
      case ResamplingMethod::AvirSmooth:
      case ResamplingMethod::Avir:
      case ResamplingMethod::AvirSharp: {
        rescaleAvir(source, target, method);
        break;
      }
      default: {
        throw std::runtime_error(u8"Invalid resampling method specified");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap BitmapScaler::Rescale(
    const Bitmap &source, const Size &newResolution, ResamplingMethod method
  ) {
    Bitmap result(newResolution.Width, newResolution.Height, source.GetPixelFormat());
    Rescale(source, result, method);
    return result;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Scaling
