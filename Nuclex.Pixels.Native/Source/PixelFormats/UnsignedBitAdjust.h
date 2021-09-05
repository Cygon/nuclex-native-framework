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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_UNSIGNEDBITADJUST_H
#define NUCLEX_PIXELS_PIXELFORMATS_UNSIGNEDBITADJUST_H

#include "Nuclex/Pixels/Config.h"
#include "BitShift.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Adjust a color channel's number of bits</summary>
  /// <typeparam name="FromBits">Bit count of the origin color channel</typeparam>
  /// <typeparam name="ToBits">Bit count the color channel will be converted to</typeparam>
  /// <remarks>
  ///   <para>
  ///     Adjusting the number of bits in a color channel is not an entirely trivial
  ///     problem. Consider the following:
  ///   </para>
  ///   <para>
  ///     <code>std::size_t eightBit = (fourBit &lt;&lt 4);</code>
  ///   </para>
  ///   <para>
  ///     This kind of expansion would cap the expanded value's maximum intensity
  ///     ot 0xF0 (240 instead of 255). Alternative methods like multiplying instead
  ///     of bit-shifting or even converting to float will be slow. This class
  ///     offers a few verified methods that produce the correct result fast.
  ///   </para>
  ///   <para>
  ///     IMPORTANT: For the output, garbage bits before and after the correct result
  ///     can be generated. If the result does not happen to be exactly the size of
  ///     a type, an extra AND operation with a bit mask of the output range is needed.
  ///   </para>
  /// <remarks>
  template<std::size_t FromBits, std::size_t ToBits>
  class UnsignedBitAdjuster;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Keeps the number of bits the same</summary>
  /// <typeparam name="FromAndToBits">The number of bits both channels have</typeparam>
  template<std::size_t FromAndToBits>
  class UnsignedBitAdjuster<FromAndToBits, FromAndToBits> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 4 to 5</summary>
  template<>
  class UnsignedBitAdjuster<4, 5> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 1>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 3>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 4 to 6</summary>
  template<>
  class UnsignedBitAdjuster<4, 6> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 2>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 4 to 8</summary>
  template<>
  class UnsignedBitAdjuster<4, 8> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 4>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 0>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 4 to 10</summary>
  template<>
  class UnsignedBitAdjuster<4, 10> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 6>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 2>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 4 to 16</summary>
  template<>
  class UnsignedBitAdjuster<4, 16> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      TPixel originalTwice = BitShift<-4>(original) | original;
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 8>(originalTwice) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 0>(originalTwice)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 5 to 4</summary>
  template<>
  class UnsignedBitAdjuster<5, 4> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 1>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 5 to 6</summary>
  template<>
  class UnsignedBitAdjuster<5, 6> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 1>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 4>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 5 to 8</summary>
  template<>
  class UnsignedBitAdjuster<5, 8> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 3>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 5 to 10</summary>
  template<>
  class UnsignedBitAdjuster<5, 10> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 5>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 0>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 5 to 16</summary>
  template<>
  class UnsignedBitAdjuster<5, 16> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      TPixel originalTwice = BitShift<-5>(original) | original;
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 6>(originalTwice) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 4>(originalTwice)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 6 to 4</summary>
  template<>
  class UnsignedBitAdjuster<6, 4> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 6 to 5</summary>
  template<>
  class UnsignedBitAdjuster<6, 5> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 1>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 6 to 8</summary>
  template<>
  class UnsignedBitAdjuster<6, 8> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 2>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 4>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 6 to 10</summary>
  template<>
  class UnsignedBitAdjuster<6, 10> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 4>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 6 to 16</summary>
  template<>
  class UnsignedBitAdjuster<6, 16> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 10>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 4>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 8 to 4</summary>
  template<>
  class UnsignedBitAdjuster<8, 4> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 4>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 8 to 5</summary>
  template<>
  class UnsignedBitAdjuster<8, 5> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 3>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 8 to 6</summary>
  template<>
  class UnsignedBitAdjuster<8, 6> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 8 to 10</summary>
  template<>
  class UnsignedBitAdjuster<8, 10> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 2>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 6>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 8 to 16</summary>
  template<>
  class UnsignedBitAdjuster<8, 16> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <typeparam name="SourceToTargetOffset">
    ///   Position of the bits in the result relative to the input value
    /// </typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 8>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 0>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 10 to 4</summary>
  template<>
  class UnsignedBitAdjuster<10, 4> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 6>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 10 to 5</summary>
  template<>
  class UnsignedBitAdjuster<10, 5> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 5>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 10 to 6</summary>
  template<>
  class UnsignedBitAdjuster<10, 6> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 4>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 10 to 8</summary>
  template<>
  class UnsignedBitAdjuster<10, 8> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 2>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Expands a color channel's number of bits from 10 to 16</summary>
  template<>
  class UnsignedBitAdjuster<10, 16> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return (
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex - 6>(original) |
        BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 4>(original)
      );
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 16 to 4</summary>
  template<>
  class UnsignedBitAdjuster<16, 4> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 12>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 16 to 5</summary>
  template<>
  class UnsignedBitAdjuster<16, 5> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 11>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 16 to 6</summary>
  template<>
  class UnsignedBitAdjuster<16, 6> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 10>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 16 to 8</summary>
  template<>
  class UnsignedBitAdjuster<16, 8> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 8>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Narrows a color channel's number of bits from 16 to 10</summary>
  template<>
  class UnsignedBitAdjuster<16, 10> {
    /// <summary>Widens or narrows a color channel's bits</summary>
    /// <typeparam name="TLowestSourceBitIndex">Lowest bit in the source channel</typeparam>
    /// <typeparam name="TLowestTargetBitIndex">Lowest bit in the target channel</typeparam>
    /// <typeparam name="TPixel">Type used to store the pixel</typeparam>
    /// <param name="original">Original color value that will be adjusted</param>
    /// <returns>The adjusted color value</returns>
    public: template<int TLowestSourceBitIndex, int TLowestTargetBitIndex, typename TPixel>
    NUCLEX_PIXELS_ALWAYS_INLINE static constexpr TPixel Adjust(TPixel original) {
      return BitShift<TLowestSourceBitIndex - TLowestTargetBitIndex + 6>(original);
    }
  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_UNSIGNEDBITADJUST_H
