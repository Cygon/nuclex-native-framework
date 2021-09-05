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

#ifndef NUCLEX_PIXELS_PIXELFORMATS_PIXELFORMATDESCRIPTION_H
#error This file is intended to be included through PixelFormatDescription.h
#endif

namespace Nuclex { namespace Pixels { namespace PixelFormats {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R8 G8 B8 A8 unsigned pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R8_G8_B8_A8_Unsigned> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::UnsignedInteger
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // We use the color channel offsets instead
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint32_t PixelType;

#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 8> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 8, 8> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 16, 8> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 24, 8> Channel4;
#else
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 24, 8> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 16, 8> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 8, 8> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 0, 8> Channel4;
#endif

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R8 G8 B8 A8 signed pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R8_G8_B8_A8_Signed> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::SignedInteger
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // We use the color channel offsets instead
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint32_t PixelType;

#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 8> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 8, 8> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 16, 8> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 24, 8> Channel4;
#else
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 24, 8> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 16, 8> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 8, 8> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 0, 8> Channel4;
#endif

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R16 G16 B16 A16 unsigned pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R16_G16_B16_A16_Unsigned_Native16> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::UnsignedInteger
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // this is a "native endian" format!
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint64_t PixelType;

#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 16> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 16, 16> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 32, 16> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 48, 16> Channel4;
#else
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 48, 16> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 32, 16> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 16, 16> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 0, 16> Channel4;
#endif

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R32 G32 B32 A32 floating point pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R32_G32_B32_A32_Float_Native32> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::FloatingPoint
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // this is a "native endian" format!
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef uint128_t PixelType;

    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 32> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 32, 32> Channel2;
    /// <summary>Bits occupied by the blue color channel</summary>
    public: typedef ColorChannelDescription<2, 64, 32> Channel3;
    /// <summary>Bits occupied by the alpha channel</summary>
    public: typedef ColorChannelDescription<3, 96, 32> Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
