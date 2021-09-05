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

  /// <summary>Describes the R8 unsigned pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R8_Unsigned> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::UnsignedInteger
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint8_t PixelType;

    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 8> Channel1;
    /// <summary>Unused green color channel</summary>
    public: typedef std::nullptr_t Channel2;
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R16 unsigned pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R16_Unsigned_Native16> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::UnsignedInteger
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // this is a "native endian" format!
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint16_t PixelType;

    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 16> Channel1;
    /// <summary>Unused green color channel</summary>
    public: typedef std::nullptr_t Channel2;
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R16 floating point pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R16_Float_Native16> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::FloatingPoint
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // this is a "native endian" format!
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint16_t PixelType;

    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 16> Channel1;
    /// <summary>Unused green color channel</summary>
    public: typedef std::nullptr_t Channel2;
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Describes the R32 floating point pixel format</summary>
  template<>
  struct PixelFormatDescription<PixelFormat::R32_Float_Native32> {

    /// <summary>Data format of the individual color channels</summary>
    public: static constexpr enum PixelFormatDataType DataType = (
      PixelFormatDataType::FloatingPoint
    );

    /// <summary>What kind of endian flip this format needs</param>
    public: static constexpr EndianFlipOperation EndianFlip = (
      EndianFlipOperation::None // this is a "native endian" format!
    );

    /// <summary>Integral type that can hold a whole pixel</summary>
    public: typedef std::uint32_t PixelType;

    /// <summary>Bits occupied by the red color channel</summary>
    public: typedef ColorChannelDescription<0, 0, 32> Channel1;
    /// <summary>Unused green color channel</summary>
    public: typedef std::nullptr_t Channel2;
    /// <summary>Unused blue color channel</summary>
    public: typedef std::nullptr_t Channel3;
    /// <summary>Unused alpha channel</summary>
    public: typedef std::nullptr_t Channel4;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::PixelFormats
