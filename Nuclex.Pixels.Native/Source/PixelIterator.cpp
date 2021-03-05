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

#include "Nuclex/Pixels/PixelIterator.h"
#include <cassert>

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::moveLeft(std::size_t pixelCount) {

    // Does the iterator stay on the same line?
    if(this->x >= pixelCount) {
      this->x -= pixelCount;
      this->current -= this->bytesPerPixel * pixelCount;
    } else { // Nope, we're going up one or more lines

      // Figure out the number of lines we have to go up and the new position
      std::size_t newX, newY;
      {
        std::size_t pixelCountBeyondLine = pixelCount - this->x;
        std::size_t lineCount = pixelCountBeyondLine / this->memory.Width;
        pixelCountBeyondLine %= this->memory.Width;
        
        if(pixelCountBeyondLine > 0) {
          ++lineCount;
          newX = this->memory.Width - pixelCountBeyondLine;
        } else {
          newX = 0;
        }

        assert(
          "Cannot move iterator before the first pixel" &&
          (this->y >= lineCount)
        );

        newY = this->y - lineCount;
      }

      this->x = newX;
      this->y = newY;
      this->current = static_cast<std::uint8_t *>(this->memory.Pixels);
      this->current += this->memory.Stride * newY;
      this->current += newX * this->bytesPerPixel;
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::moveRight(std::size_t pixelCount) {

    // Does the move stay in the same line? Normal, branch-predicted case.
    std::size_t pixelsRemainingPlusOne = this->memory.Width - this->x;
    if(pixelCount < pixelsRemainingPlusOne) {
      this->x += pixelCount;
      this->current += this->bytesPerPixel * pixelCount;
    } else { // Nope we're advancing one or multiple lines

      // Figure out the number of lines we have to go down and the new position
      std::size_t newX, newY;
      {
        std::size_t pixelCountBeyondLine = (pixelCount - pixelsRemainingPlusOne);
        std::size_t lineCountMinusOne = (pixelCountBeyondLine / this->memory.Width);

        newX = pixelCountBeyondLine % this->memory.Width;
        //newX = pixelCountBeyondLine - (lineCountMinusOne * this->memory.Width);
        newY = this->y + lineCountMinusOne + 1;
      }

      // Does the position stay inside the bitmap?
      if(newY < this->memory.Height) {
        this->x = newX;
        this->y = newY;
        this->current = static_cast<std::uint8_t *>(this->memory.Pixels);
        this->current += this->memory.Stride * newY;
        this->current += newX * this->bytesPerPixel;
      } else if((newY == this->memory.Height) && (newX == 0)) { // 'end' position?
        this->x = this->memory.Width;
        this->y = this->memory.Height;
        this->current = static_cast<std::uint8_t *>(this->memory.Pixels);
        this->current += this->memory.Stride * this->y;
        this->current += this->memory.Width * this->bytesPerPixel;
      } else { // Out of bounds!
        using namespace std;
        assert(
          "Cannot move iterator beyond the end position" &&
          (newY < this->memory.Height) && (newX < this->memory.Width)
        );
      }
    }

  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::moveUp(std::size_t lineCount) {

    // Would this go above the top of the image?
    {
      using namespace std;
      assert(
        "Cannot move iterator before the first line" &&
        (this->y >= lineCount)
      );
    }

    // Adjust the position
    this->y -= lineCount;
    this->current -= this->memory.Stride * lineCount;

  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::moveDown(std::size_t lineCount) {

    // Would this go beyond the bottom of the image?
    std::size_t newY = this->y + lineCount;
    {
      using namespace std;
      assert(
        "Cannot move iterator beyond the last line" &&
        (newY < this->memory.Height)
      );
    }

    // Adjust the position
    this->y = newY;
    this->current += this->memory.Stride * lineCount;

  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::enforceNotAtEndPosition() const {
    using namespace std; // Some C++ std libs had assert in std, some didn't :-/
    assert(
      "Iterator must lie on a valid position and before the end position" &&
      (this->x < this->memory.Width) && (this->y < this->memory.Height)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::enforceStrideLargerThanLineByteCount() const {
    using namespace std; // Some C++ std libs had assert in std, some didn't :-/
    assert(
      "Stride must be greater or equal to the byte count of one bitmap line" &&
      (static_cast<std::size_t>(std::abs(this->memory.Stride)) >= (this->memory.Width * this->bytesPerPixel))
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::enforceIteratorCanRetreat() const {
    using namespace std; // Some C++ std libs had assert in std, some didn't :-/
    assert(
      "Iterator must be at a position that has a preceding position" &&
      (this->current > static_cast<const std::uint8_t *>(this->memory.Pixels))
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::enforceIteratorCanAdvance() const {
    using namespace std; // Some C++ std libs had assert in std, some didn't :-/
    assert(
      "Iterator must be at the end position or have a succeeding position" &&
      ((this->y * this->memory.Width + this->x) < (this->memory.Width * this->memory.Height))
    );
  }

  // ------------------------------------------------------------------------------------------- //

  void PixelIterator::enforceAtEndPosition(std::size_t checkX, std::size_t checkY) const {
    (void)checkX;
    (void)checkY;

    using namespace std; // Some C++ std libs had assert in std, some didn't :-/
    assert(
      "Cannot move iterator beyond the end position" &&
      (checkX == this->memory.Width) && (checkY == this->memory.Height)
    );
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels
