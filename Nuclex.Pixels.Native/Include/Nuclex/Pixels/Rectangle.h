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

#ifndef NUCLEX_PIXELS_RECTANGLE_H
#define NUCLEX_PIXELS_RECTANGLE_H

#include "Nuclex/Pixels/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>2D rectangle using pixel coordinates</summary>
  struct NUCLEX_PIXELS_TYPE Rectangle {

    /// <summary>Constructs a new rectangle from a minimum and maximum point</summary>
    /// <param name="minX">X coordinate of the rectangle's left side</param>
    /// <param name="minY">Y coordinate of the rectangle's lower side</param>
    /// <param name="maxX">X coordinate of the rectangle's right side</param>
    /// <param name="maxY">Y coordinate of the rectangle's upper side</param>
    /// <remarks>
    ///   <para>
    ///     As a convention, the maximum (far) point is exclusive. Pixels in which
    ///     one or both coordinates match with the maximum coordinates specified here
    ///     are considered to be outside of the rectangle (i.e. a rectange (0,0)-(0,0)
    ///     is empty and contains nothing, while (0,0)-(1,1) contains exactly one pixel).
    ///   </para>
    ///   <para>
    ///     This library does not make any assumptions about whether you Y-axis is
    ///     counting upwards or downwards, but documentation will treat the Y-axis as
    ///     extending towards the top (the upside-down Y-axis common in 2D graphics
    ///     is an artifact of ancient computer/TV design).
    ///   </para>
    /// </remarks>
    public: NUCLEX_PIXELS_API static Rectangle FromMinAndMax(
      std::size_t minX, std::size_t minY,
      std::size_t maxX, std::size_t maxY
    ) {
      return Rectangle(minX, minY, maxX, maxY);
    }

    /// <summary>Constructs a new rectangle from a position and a size</summary>
    /// <param name="x">X coordinate of the rectangle's minor corner</param>
    /// <param name="x">Y coordinate of the rectangle's minor corner</param>
    /// <param name="width">Total width of the rectangle</param>
    /// <param name="height">Total height of the rectangle</param>
    public: NUCLEX_PIXELS_API static Rectangle FromPositionAndSize(
      std::size_t x, std::size_t y,
      std::size_t width, std::size_t height
    ) {
      return Rectangle(x, y, x + width, y + height);
    }

    /// <summary>Initializes a new rectangle size</summary>
    /// <param name="width">Width of the rectangle in pixels</param>
    /// <param name="height">Height of the rectangle in pixels</param>
    public: NUCLEX_PIXELS_API Rectangle(
      std::size_t minX, std::size_t minY,
      std::size_t maxX, std::size_t maxY
    ) :
      MinX(minX),
      MinY(minY),
      MaxX(maxX),
      MaxY(maxY) {}

    /// <summary>X coordinate of the rectangle's left side</summary>
    public: std::size_t MinX;
    /// <summary>X coordinate of the rectangle's lower side</summary>
    public: std::size_t MinY;
    /// <summary>Y coordinate of the rectangle's right side</summary>
    public: std::size_t MaxX;
    /// <summary>Y coordinate of the rectangle's upper side</summary>
    public: std::size_t MaxY;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_SIZE_H
