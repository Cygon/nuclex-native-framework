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

#include "JpegBitmapCodec.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)

#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"
#include "Nuclex/Pixels/Errors/WrongSizeError.h"

#include "Nuclex/Support/ScopeGuard.h"

#include "LibJpegHelpers.h"

#include <cassert> // for assert()
#include <algorithm>

#include <jpeglib.h>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Ignores diagnostic messages from jpeglib</summary>
  /// <param name="cinfo">Jpeg common info containing the diagnostic message</param>
  void discardJpegMessage(struct ::jpeg_common_struct *cinfo) { (void)cinfo; }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Handles an error inside libjpeg</summary>
  /// <param name="info">Main structure containing all libjpeg configuration</param>
  /// <remarks>
  ///   <para>
  ///     libjpeg is a C library, but its error handling scheme expects this function to
  ///     never return (either it calls abort() or longjmp()). To allow this, all memory
  ///     libjpeg allocates must be tracked in the jpeg_common_struct and there must be no
  ///     open ends on the stack when the error handler is called.
  ///   </para>
  ///   <para>
  ///     This gives us all the guarantees we need to fire a C++ exception right through
  ///     libjpeg back to our original call site.
  ///   </para>
  /// </remarks>
  void handleJpegError(struct ::jpeg_common_struct *cinfo) {
    if(cinfo != nullptr) {
      if(cinfo->err != nullptr) {
        const ::jpeg_error_mgr &errorManager = *cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*errorManager.format_message)(cinfo, buffer);

        throw Nuclex::Pixels::Errors::FileFormatError(buffer);
      }
    }

    throw Nuclex::Pixels::Errors::FileFormatError(u8"Error occurred in libjpeg");
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interpolates between a minimum and maximum value</summary>
  /// <typeparam name="TValue">
  ///   Type of value that will be interpolated, assumed to be  an integer
  /// </typeparam>
  /// <param name="t">Interpolation point between 0.0 .. 1.0</param>
  /// <param name="min">Minimum value, will be returned at <paramref name="t" /> 0.0</param>
  /// <param name="max">Maximum value, will be returned at <paramref name="t" /> 1.0</param>
  /// <returns>The interpolated value</returns>
  template<typename TValue>
  TValue lerpInclusive(float t, TValue min, TValue max) {
    float interpolated = static_cast<float>(max - min) * t + 0.5f;
    return min + static_cast<TValue>(interpolated);
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage { namespace Jpeg {

  // ------------------------------------------------------------------------------------------- //

  void JpegBitmapCodec::Save(
    const Bitmap &bitmap, VirtualFile &target,
    float compressionEffortHint /* = 0.75f */, float outputQualityHint /* = 0.95f */
  ) const {
    (void)bitmap;
    (void)target;
    (void)compressionEffortHint;
    (void)outputQualityHint;

    {
      ::jpeg_compress_struct commonInfo;

      // Set up a custom error manager that throws exceptions rather than exit()
      struct ::jpeg_error_mgr errorManager;
      ::jpeg_std_error(&errorManager);
      errorManager.error_exit = &handleJpegError;
      errorManager.output_message = &discardJpegMessage;
      commonInfo.err = &errorManager;

      ::jpeg_create_compress(&commonInfo);
      ON_SCOPE_EXIT {
        ::jpeg_destroy_compress(&commonInfo);
      };

      const BitmapMemory &memory = bitmap.Access();
      commonInfo.image_width = memory.Width;
      commonInfo.image_height = memory.Height;
      commonInfo.input_components = 3;
      commonInfo.in_color_space = JCS_RGB;

      {
        // Initialize all compression settings to their defaults
        ::jpeg_set_defaults(&commonInfo);

        // Override the image quality
        ::boolean forceBaseline = TRUE;
        int quality = lerpInclusive(outputQualityHint, 0, 100);
        ::jpeg_set_quality(&commonInfo, quality, forceBaseline);
      }

      {
        // Set up a custom data source that reads from a virtual file
        JpegWriteEnvironment virtualFileTarget(target);
        commonInfo.dest = &virtualFileTarget;
      }


    }

    throw std::runtime_error(u8"Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

}}}} // namespace Nuclex::Pixels::Storage::Jpeg

#endif // defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
