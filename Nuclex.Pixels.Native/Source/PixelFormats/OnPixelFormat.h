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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_ONPIXELFORMAT_H
#define NUCLEX_PIXELS_PIXELFORMATS_ONPIXELFORMAT_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/PixelFormat.h"

#include <utility> // for std::forward
#include <stdexcept> // for std::invalid_argument

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Invokes a functor specialized on a runtime-specific pixel format</summary>
  /// <typeparam name="TFunctor">
  ///   Template functor that will be specialized on the pixel format
  /// </typeparam>
  /// <typeparam name="TResult">
  ///   Type of the result that will be returned by the functor
  /// </typeparam>
  /// <typeparam name="TArguments">
  ///   Additional arguments that will be passed through to the functor
  /// </typeparam>
  /// <param name="pixelFormat">
  ///   Pixel format for which the functor template will be specialized
  /// </param>
  /// <param name="arguments">Additional arguments that will be passed to the functor</param>
  /// <returns>The result returned by the functor</returns>
  /// <remarks>
  ///   <para>
  ///     This method lets you run a templated action on a pixel format whose type will
  ///     only be known at runtime. This allows you to write algorithms with templates
  ///     (i.e. using data from the PixelFormatDescription template) while still having
  ///     the flexibility to use the pixel format from a <see cref="Bitmap" /> for example.
  ///   </para>
  ///   <para>
  ///     The downside is that this will to instantiate the template with all pixel formats
  ///     in the library. To avoid enormous code bloat, either use trivial functors that
  ///     can be inlined (thus resulting in just a few bytes of instructions per pixel format)
  ///     or isolate the smallest possible code you template on the pixel format.
  ///   </para>
  ///   <example>
  ///     <code>
  ///        template&lt;Nuclex::Pixels::PixelFormat TPixelFormat&gt;
  ///        class CheckIfEndianFlipped {
  ///          public: bool operator()() const {
  ///            return Nuclex::Pixels::PixelFormats::IsFlippedFormat&lt;TPixelFormat&gt;
  ///          }
  ///        };
  ///
  ///        void test() {
  ///          Nuclex::Pixels::PixelFormat myFormat = Nuclex::Pixels::PixelFormat::R16_Unsigned;
  ///          bool isEndianFlipped = OnPixelFormat&lt;CheckIfEndianFlipped, bool&gt;(myFormat);
  ///        }
  ///      </code>
  ///    </example>
  /// </remarks>
  template<
    template<PixelFormat> typename TFunctor, typename TResult, typename... TArguments
  >
  TResult OnPixelFormat(PixelFormat pixelFormat, TArguments... arguments) {
    switch(pixelFormat) {

      case PixelFormat::R8_Unsigned: {
        return TFunctor<PixelFormat::R8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R16_Unsigned_Native16: {
        return TFunctor<PixelFormat::R16_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R16_Float_Native16: {
        return TFunctor<PixelFormat::R16_Float_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R32_Float_Native32: {
        return TFunctor<PixelFormat::R32_Float_Native32>()(std::forward<TArguments>(arguments)...);
      }

      case PixelFormat::A8_Unsigned: {
        return TFunctor<PixelFormat::A8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::A16_Unsigned_Native16: {
        return TFunctor<PixelFormat::A16_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::A16_Float_Native16: {
        return TFunctor<PixelFormat::A16_Float_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::A32_Float_Native32: {
        return TFunctor<PixelFormat::A32_Float_Native32>()(std::forward<TArguments>(arguments)...);
      }

      case PixelFormat::R8_G8_Unsigned: {
        return TFunctor<PixelFormat::R8_G8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R16_G16_Unsigned_Native16: {
        return TFunctor<PixelFormat::R16_G16_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R16_G16_Float_Native16: {
        return TFunctor<PixelFormat::R16_G16_Float_Native16>()(std::forward<TArguments>(arguments)...);
      }

      case PixelFormat::R8_A8_Unsigned: {
        return TFunctor<PixelFormat::R8_A8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R16_A16_Unsigned_Native16: {
        return TFunctor<PixelFormat::R16_A16_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }

      case PixelFormat::R5_G6_B5_Unsigned_Native16: {
        return TFunctor<PixelFormat::R5_G6_B5_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::R8_G8_B8_Unsigned: {
        return TFunctor<PixelFormat::R8_G8_B8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      /*
      case PixelFormat::R8_G8_B8_Signed: {
        return TFunctor<PixelFormat::R8_G8_B8_Signed>()(std::forward<TArguments>(arguments)...);
      }
      */

      case PixelFormat::B5_G6_R5_Unsigned_Native16: {
        return TFunctor<PixelFormat::B5_G6_R5_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }
      case PixelFormat::B8_G8_R8_Unsigned: {
        return TFunctor<PixelFormat::B8_G8_R8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      /*
      case PixelFormat::B8_G8_R8_Signed: {
        return TFunctor<PixelFormat::B8_G8_R8_Signed>()(std::forward<TArguments>(arguments)...);
      }
      */

      // --- insert others here in order ---

      case PixelFormat::R8_G8_B8_A8_Unsigned: {
        return TFunctor<PixelFormat::R8_G8_B8_A8_Unsigned>()(std::forward<TArguments>(arguments)...);
      }
      /*
      case PixelFormat::R8_G8_B8_A8_Signed: {
        return TFunctor<PixelFormat::R8_G8_B8_A8_Signed>()(std::forward<TArguments>(arguments)...);
      }
      */

      case PixelFormat::R16_G16_B16_A16_Unsigned_Native16: {
        return TFunctor<PixelFormat::R16_G16_B16_A16_Unsigned_Native16>()(std::forward<TArguments>(arguments)...);
      }

      case PixelFormat::R32_G32_B32_A32_Float_Native32: {
        return TFunctor<PixelFormat::R32_G32_B32_A32_Float_Native32>()(std::forward<TArguments>(arguments)...);
      }

      default: { throw std::invalid_argument(u8"Unknown pixel format specified"); }

    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_ONPIXELFORMAT_H
