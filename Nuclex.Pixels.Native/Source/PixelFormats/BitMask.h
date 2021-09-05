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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_BITMASK_H
#define NUCLEX_PIXELS_PIXELFORMATS_BITMASK_H

#include "Nuclex/Pixels/Config.h"
#include "BitShift.h"

#include <cstddef> // for std::side_z

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Creates a bit mask for the specified bit range</summary>
  /// <typeparam name="TUnsigned">Integer type the bit mask will be stored in</typeparam>
  /// <typeparam name="LowestBitIndex">Index of the lowest bit set in the mask</typeparam>
  /// <typeparam name="BitCount">Number of bits that will be set in the mask</typeparam>
  /// <remarks>
  ///   <para>
  ///     Example: create an std::uint16_t with the second, third and fourth bits set:
  ///   </para>
  ///   <para>
  ///     <code>
  ///       // Lowest set bit is 1 (zero-based indexed), set 3 bits starting there
  ///       const std::uint16_t myMask = BitMask<std::uint16_t, 1, 3>;
  ///     </code>
  ///   </para>
  ///   <para>
  ///     Example: create an std::uint64_t with the upper half of its bits set
  ///   </para>
  ///   <para>
  ///     <code>
  ///       // Bits 0-31 (inclusive, )
  ///       const std::uint64_t myMask = BitMask<std::uint64>t, 31, 32>;
  ///     </code>
  ///   </para>
  /// </remarks>
  template<typename TUnsigned, std::size_t LowestBitIndex, std::size_t BitCount>
  constexpr TUnsigned BitMask = (
    BitShift<-static_cast<int>(LowestBitIndex + BitCount)>(static_cast<TUnsigned>(-1)) ^
    BitShift<-static_cast<int>(LowestBitIndex)>(static_cast<TUnsigned>(-1))
  );

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats

#endif // NUCLEX_PIXELS_PIXELFORMATS_BITMASK_H
