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

  /// <summary>Describes the R8 G8 unsigned pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R8_G8_Unsigned> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::UnsignedInteger
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // We use the color channel offsets instead
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint16_t PixelType;

#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 8> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 8, 8> Channel2;
#else
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 8, 8> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 0, 8> Channel2;
#endif
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R16 G16 unsigned pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R16_G16_Unsigned_Native16> {

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
    public: typedef ColorChannelDescription<0, 0, 16> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 16, 16> Channel2;
#else
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 16, 16> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 0, 16> Channel2;
#endif
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R16 G16 float pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R16_G16_Float_Native16> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::FloatingPoint
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // We use the color channel offsets instead
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint32_t PixelType;

#if defined(NUCLEX_PIXELS_LITTLE_ENDIAN)
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 16> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 16, 16> Channel2;
#else
    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 16, 16> Channel1;
    /// <summary>Bits occupied by the green color channel</summary>
    public: typedef ColorChannelDescription<1, 0, 16> Channel2;
#endif
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
