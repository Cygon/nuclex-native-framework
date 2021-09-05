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

#ifndef NUCLEX_PIXELS_SIZE_H
#define NUCLEX_PIXELS_SIZE_H

#include "Nuclex/Pixels/Config.h"

#include <cstddef> // for std::size_t

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Size of a 2D rectangle in pixels</summary>
  struct NUCLEX_PIXELS_TYPE Size {

    /// <summary>Initializes a new rectangle size</summary>
    /// <param name="width">Width of the rectangle in pixels</param>
    /// <param name="height">Height of the rectangle in pixels</param>
    public: NUCLEX_PIXELS_API Size(std::size_t width, std::size_t height) :
      Width(width),
      Height(height) {}

    /// <summary>Width of the rectangle in pixels</summary>
    public: std::size_t Width;
    /// <summary>Height of the rectangle in pixels</summary>
    public: std::size_t Height;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_SIZE_H
