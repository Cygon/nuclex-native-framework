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

#include "Nuclex/Pixels/PixelFormats/PixelFormatQuery.h"

#include "ChannelHelpers.h"
#include "PixelFormatDescription.h"
#include "OnPixelFormat.h"

#include <tuple>

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format has a red color channel</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfRedChannelPresent {
    /// <summary>Returns whether the pixel format has a red color channel</summary>
    /// <returns>True if the pixel format has a red color channel</returns>
    public: bool operator()() const {
      return !std::is_same<
        typename Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat>::Channel1,
        std::nullptr_t
      >::value;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format has a green color channel</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfGreenChannelPresent {
    /// <summary>Returns whether the pixel format has a green color channel</summary>
    /// <returns>True if the pixel format has a green color channel</returns>
    public: bool operator()() const {
      return !std::is_same<
        typename Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat>::Channel2,
        std::nullptr_t
      >::value;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format has a blue color channel</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfBlueChannelPresent {
    /// <summary>Returns whether the pixel format has a blue color channel</summary>
    /// <returns>True if the pixel format has a blue color channel</returns>
    public: bool operator()() const {
      return !std::is_same<
        typename Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat>::Channel3,
        std::nullptr_t
      >::value;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format has a alpha channel</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfAlphaChannelPresent {
    /// <summary>Returns whether the pixel format has an alpha channel</summary>
    /// <returns>True if the pixel format has a alpha channel</returns>
    public: bool operator()() const {
      return !std::is_same<
        typename Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat>::Channel4,
        std::nullptr_t
      >::value;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format uses signed channels</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfSignedFormat {
    /// <summary>Returns whether the pixel format uses signed channels</summary>
    /// <returns>True if the pixel format uses signed integers or floats</returns>
    public: bool operator()() const {
      return Nuclex::Pixels::PixelFormats::IsSignedFormat<TPixelFormat>;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format uses floating point channels</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfFloatFormat {
    /// <summary>Returns whether the pixel format uses floating point channels</summary>
    /// <returns>True if the pixel format uses floating point channels</returns>
    public: bool operator()() const {
      return Nuclex::Pixels::PixelFormats::IsFloatFormat<TPixelFormat>;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>
  ///   Checks whether all channels in the specified pixel format are byte-aligned
  /// </summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfChannelsByteAligned {
    /// <summary>Returns whether the pixel format uses byte-aligned channels</summary>
    /// <returns>True if the pixel format uses byte-aligned channels</returns>
    public: bool operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;

      bool allByteAligned = true;

      if constexpr(!std::is_same<typename Description::Channel1, std::nullptr_t>::value) {
        allByteAligned &= ((Description::Channel1::LowestBitIndex % 8) == 0);
        allByteAligned &= ((Description::Channel1::BitCount % 8) == 0);
      }
      if constexpr(!std::is_same<typename Description::Channel2, std::nullptr_t>::value) {
        allByteAligned &= ((Description::Channel2::LowestBitIndex % 8) == 0);
        allByteAligned &= ((Description::Channel2::BitCount % 8) == 0);
      }
      if constexpr(!std::is_same<typename Description::Channel3, std::nullptr_t>::value) {
        allByteAligned &= ((Description::Channel3::LowestBitIndex % 8) == 0);
        allByteAligned &= ((Description::Channel3::BitCount % 8) == 0);
      }
      if constexpr(!std::is_same<typename Description::Channel4, std::nullptr_t>::value) {
        allByteAligned &= ((Description::Channel4::LowestBitIndex % 8) == 0);
        allByteAligned &= ((Description::Channel4::BitCount % 8) == 0);
      }

      return allByteAligned;
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Checks whether the specified pixel format requires endian-flipping</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class CheckIfEndianFlippingRequired {
    /// <summary>Returns whether the pixel format requires endian-flipping</summary>
    /// <returns>True if the pixel format required endian-flipping</returns>
    public: bool operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      return false;
      //return (Description::EndianFlip != nuclex::Pixels::PixelFormats::EndianFlipMode::None);
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the index of the lowest red bit in the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchLowestRedBitIndex {
    /// <summary>Returns the index of the lowest red bit in the pixel format</summary>
    /// <returns>The index of the lowest red bit in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel1, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel1::LowestBitIndex;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the index of the lowest green bit in the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchLowestGreenBitIndex {
    /// <summary>Returns the index of the lowest green bit in the pixel format</summary>
    /// <returns>The index of the lowest green bit in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel2, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel2::LowestBitIndex;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the index of the lowest blue bit in the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchLowestBlueBitIndex {
    /// <summary>Returns the index of the lowest blue bit in the pixel format</summary>
    /// <returns>The index of the lowest blue bit in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel3, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel3::LowestBitIndex;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves the index of the lowest alpha bit in the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchLowestAlphaBitIndex {
    /// <summary>Returns the index of the lowest alpha bit in the pixel format</summary>
    /// <returns>The index of the lowest alpha bit in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel4, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel4::LowestBitIndex;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of red bits used by the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchRedBitCount {
    /// <summary>Returns the number of red bits used by the pixel format</summary>
    /// <returns>The number of bits used for the red channel in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel1, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel1::BitCount;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of green bits used by the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchGreenBitCount {
    /// <summary>Returns the number of green bits used by the pixel format</summary>
    /// <returns>The number of bits used for the green channel in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel2, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel2::BitCount;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of blue bits used by the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchBlueBitCount {
    /// <summary>Returns the number of blue bits used by the pixel format</summary>
    /// <returns>The number of bits used for the blue channel in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel3, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel3::BitCount;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of alpha bits used by the pixel format</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchAlphaBitCount {
    /// <summary>Returns the number of alpha bits used by the pixel format</summary>
    /// <returns>The number of bits used for the alpha channel in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;
      if constexpr(std::is_same<typename Description::Channel4, std::nullptr_t>::value) {
        return std::size_t(-1);
      } else {
        return Description::Channel4::BitCount;
      }
    }
  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Counts the number of bits used by the pixel format's widest channel</summary>
  /// <typeparam name="TPixelFormat">Pixel format that will be checked</typeparam>
  template<Nuclex::Pixels::PixelFormat TPixelFormat>
  class FetchWidestChannelBitCount {
    /// <summary>Returns the number of bits used by the pixel format's widest channel</summary>
    /// <returns>The number of bits used for the widest channel in the pixel format</returns>
    public: std::size_t operator()() const {
      typedef Nuclex::Pixels::PixelFormats::PixelFormatDescription<TPixelFormat> Description;

      std::size_t widestColorChannelBitCount = 0;

      if constexpr(!std::is_same<typename Description::Channel1, std::nullptr_t>::value) {
        widestColorChannelBitCount = Description::Channel1::BitCount;
      }
      if constexpr(!std::is_same<typename Description::Channel2, std::nullptr_t>::value) {
        if(Description::Channel2::BitCount > widestColorChannelBitCount) {
          widestColorChannelBitCount = Description::Channel2::BitCount;
        }
      }
      if constexpr(!std::is_same<typename Description::Channel3, std::nullptr_t>::value) {
        if(Description::Channel3::BitCount > widestColorChannelBitCount) {
          widestColorChannelBitCount = Description::Channel3::BitCount;
        }
      }
      if constexpr(!std::is_same<typename Description::Channel4, std::nullptr_t>::value) {
        if(Description::Channel4::BitCount > widestColorChannelBitCount) {
          widestColorChannelBitCount = Description::Channel4::BitCount;
        }
      }

      return widestColorChannelBitCount;
    }
  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::HasRedChannel(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfRedChannelPresent, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::HasGreenChannel(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfGreenChannelPresent, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::HasBlueChannel(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfBlueChannelPresent, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::HasAlphaChannel(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfAlphaChannelPresent, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::IsSignedFormat(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfSignedFormat, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::IsFloatFormat(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfFloatFormat, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::HasDifferentlySizedChannels(PixelFormat pixelFormat) {
    return (
      (pixelFormat == PixelFormat::A2_B10_G10_R10_Unsigned_Native32) ||
      (pixelFormat == PixelFormat::A2_R10_G10_B10_Unsigned_Native32) ||
      (pixelFormat == PixelFormat::B5_G6_R5_Unsigned_Native16) ||
      (pixelFormat == PixelFormat::R5_G6_B5_Unsigned_Native16)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::AreAllChannelsByteAligned(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfChannelsByteAligned, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  bool PixelFormatQuery::RequiresEndianFlip(PixelFormat pixelFormat) {
    return OnPixelFormat<CheckIfEndianFlippingRequired, bool>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::GetLowestRedBitIndex(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchLowestRedBitIndex, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::GetLowestGreenBitIndex(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchLowestGreenBitIndex, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::GetLowestBlueBitIndex(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchLowestBlueBitIndex, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::GetLowestAlphaBitIndex(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchLowestAlphaBitIndex, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::CountRedBits(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchRedBitCount, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::CountGreenBits(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchGreenBitCount, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::CountBlueBits(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchBlueBitCount, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::size_t> PixelFormatQuery::CountAlphaBits(PixelFormat pixelFormat) {
    std::size_t index = OnPixelFormat<FetchAlphaBitCount, std::size_t>(pixelFormat);
    if(index == std::size_t(-1)) {
      return std::optional<std::size_t>();
    } else {
      return index;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t PixelFormatQuery::CountWidestChannelBits(PixelFormat pixelFormat) {
    return OnPixelFormat<FetchWidestChannelBitCount, std::size_t>(pixelFormat);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
