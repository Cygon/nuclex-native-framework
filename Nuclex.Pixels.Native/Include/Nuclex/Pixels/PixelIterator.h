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

#ifndef NUCLEX_PIXELS_PIXELITERATOR_H
#define NUCLEX_PIXELS_PIXELITERATOR_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/BitmapMemory.h"

#include <cstdint> // for std::ptrdiff_t
#include <iterator> // for std::random_access_iterator_tag

namespace Nuclex { namespace Pixels {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Allows an offset in lines to be passed to pixel accessors</summary>
  class NUCLEX_PIXELS_TYPE Lines {

    /// <summary>Initializes a new line offset</summary>
    /// <param name="offset">Number of lines the pixel accessor should advance</param>
    public: NUCLEX_PIXELS_API explicit Lines(std::ptrdiff_t offset) :
      offset(offset) {}

    /// <summary>Retrieves the vertical offset to advance</summary>
    /// <returns>The vertical offset the pixel accessor should advance</returns>
    public: NUCLEX_PIXELS_API std::ptrdiff_t GetOffset() const { return this->offset; }

    /// <summary>Number of lines the pixel accessor should advance</summary>
    private: std::ptrdiff_t offset;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Moves through an image's pixels and allows them to be accessed</summary>
  /// <remarks>
  ///   <para>
  ///     The pixel iterator is a convenient way to process and alter pixels in an image
  ///     without having to do expensive bounds checks and address calculations per pixel.
  ///     It follows the design of standard iterators and can be used to access each pixel
  ///     in a bitmap like so:
  ///   </para>
  ///   <para>
  ///     <example>
  ///       <code>
  ///         PixelIterator end = PixelIterator::GetEnd(myBitmapMemory);
  ///         for(PixelIterator current(myBitmapMemory); current != end; ++current) {
  ///           void *pixelAddress = *current; // do something with the pixel
  ///         }
  ///       </code>
  ///     </example>
  ///   </para>
  ///   <para>
  ///     Performance-wise, the pixel iterator is a bit slower than hand-coding pixel accesses,
  ///     but not by much. For example, when stepping through an image's pixels like above,
  ///     only a single clipping check is performed when it is advanced and the address is
  ///     simply incremented instead of being recalculated.
  ///   </para>
  ///   <para>
  ///     By design, the pixel access iterator only requires a <see cref="BitmapMemory" />
  ///     instance to work on, so that it can be used on various targets, including locked
  ///     textures of 3D rendering APIs or bitmaps in various windowing systems.
  ///   </para>
  ///   <para>
  ///     In addition to being a random access iterator (that allows jumping multiple pixels
  ///     and using the array indexing operator), the pixel access iterator can also move
  ///     vertically or go to a specific position:
  ///   </para>
  ///   <para>
  ///     <example>
  ///       <code>
  ///         PixelIterator current(myBitmapMemory)
  ///         current.MoveTo(256, 256); // Jump to this X, Y position
  ///
  ///         do {
  ///           current += Lines(1); // Go down 1 pixel
  ///         } while(current.GetY() < (myBitmapMemory.Height)) {
  ///       </code>
  ///     </example>
  ///   </para>
  /// </remarks>
  class NUCLEX_PIXELS_TYPE PixelIterator {

    /// <summary>Type that results when the distance of two iterators is calculated</summary>
    public: typedef std::ptrdiff_t difference_type;

    /// <summary>Type of the values this iterator iterates over</summary>
    public: typedef void *value_type;

    /// <summary>Reference to an element addressed by this iterator</summary>
    public: typedef void *&reference;

    /// <summary>Type of pointer this iterator emulates</summary>
    public: typedef void **pointer;

    /// <summary>Which type of iterator this is</summary>
    public: typedef std::random_access_iterator_tag iterator_category;

    /// <summary>Builds the begin iterator for a pixel accessor to the specified memory</summary>
    /// <param name="memory">Bitmap memory for which a begin iterator will be built</param>
    /// <returns>The begin iterator for the specified bitmap memory</returns>
    public: NUCLEX_PIXELS_API static PixelIterator GetBegin(const BitmapMemory &memory) {
      return PixelIterator(memory);
    }

    /// <summary>Builds the end iterator for a pixel accessor to the specified memory</summary>
    /// <param name="memory">Bitmap memory for which an end iterator will be built</param>
    /// <returns>The end iterator for the specified bitmap memory</returns>
    public: NUCLEX_PIXELS_API static PixelIterator GetEnd(const BitmapMemory &memory) {
      return PixelIterator(memory) += (memory.Width * memory.Height);
    }

    /// <summary>Initializes a new pixel accessor for the specified bitmap memory</summary>
    /// <param name="memory">Bitmap memory the pixel accessor will work on</param>
    public: NUCLEX_PIXELS_API PixelIterator(const BitmapMemory &memory) :
      memory(memory),
      x(0),
      y(0),
      current(reinterpret_cast<std::uint8_t *>(memory.Pixels)),
      bytesPerPixel(CountBitsPerPixel(memory.PixelFormat) / 8) {
      enforceStrideLargerThanLineByteCount();
    }

    /// <summary>Moves the pixel access iterator to the specified position</summary>
    /// <param name="x">X coordinate the pixel access iterator will be moved to</param>
    /// <param name="y">Y coordinate the pixel access iterator will be moved to</param>
    public: NUCLEX_PIXELS_API void MoveTo(std::size_t newX, std::size_t newY) {
      if((newX >= this->memory.Width) || (newY >= this->memory.Height)) {
        // It is valid to place the iterator *exactly* at one past the last pixel,
        // so we only fail if the position is out of bounds in any other way.
        enforceAtEndPosition(newX, newY);
      }

      this->x = newX;
      this->y = newY;
      this->current = static_cast<std::uint8_t *>(memory.Pixels);
      this->current += newY * memory.Stride;
      this->current += newX * this->bytesPerPixel;
    }

    /// <summary>Moves the pixel accessor vertically in negative direction</summary>
    /// <param name="lines">Number of lines the pixel accessor will be moved back</param>
    /// <returns>The pixel accessor</returns>
    public: NUCLEX_PIXELS_API PixelIterator &operator -=(const Lines &lines) {
      if(lines.GetOffset() < 0) {
        moveDown(static_cast<std::size_t>(-lines.GetOffset()));
      } else {
        moveUp(static_cast<std::size_t>(lines.GetOffset()));
      }

      return *this;
    }

    /// <summary>Moves the pixel accessor vertically in positive direction</summary>
    /// <param name="lines">Number of lines the pixel accessor will be moved forward</param>
    /// <returns>The pixel accessor</returns>
    public: NUCLEX_PIXELS_API PixelIterator &operator +=(const Lines &lines) {
      if(lines.GetOffset() < 0) {
        moveUp(static_cast<std::size_t>(-lines.GetOffset()));
      } else {
        moveDown(static_cast<std::size_t>(lines.GetOffset()));
      }

      return *this;
    }

    /// <summary>Moves the pixel accessor horizontally in negative direction</summary>
    /// <param name="offset">Number of pixels the pixel accessor will be moved back</param>
    /// <returns>The pixel accessor</returns>
    public: NUCLEX_PIXELS_API PixelIterator &operator -=(
      PixelIterator::difference_type offset
    ) {
      if(offset < 0) {
        moveRight(static_cast<std::size_t>(-offset));
      } else {
        moveLeft(static_cast<std::size_t>(offset));
      }

      return *this;
    }

    /// <summary>Moves the pixel accessor horizontally in positive direction</summary>
    /// <param name="offset">Number of pixels the pixel accessor will be moved forward</param>
    /// <returns>The pixel accessor</returns>
    public: NUCLEX_PIXELS_API PixelIterator &operator +=(
      PixelIterator::difference_type offset
    ) {
      if(offset < 0) {
        moveLeft(static_cast<std::size_t>(-offset));
      } else {
        moveRight(static_cast<std::size_t>(offset));
      }

      return *this;
    }

    /// <summary>Moves the pixel accessor to the previous pixel</summary>
    /// <returns>The pixel accessor</returns>
    public: NUCLEX_PIXELS_API PixelIterator &operator --() {

      // Are we somewhere in the middle of a line? (normal, branch-predicted case)
      if(this->x > 0) {
        this->current -= this->bytesPerPixel;
        --this->x;
      } else { // Or have we reached the left image border?
        enforceIteratorCanRetreat(); // Checks and triggers an assertion if needed

        this->x = this->memory.Width - 1;
        this->current -= this->memory.Stride - (this->x * this->bytesPerPixel);
        --this->y;
      }

      return *this;

    }

    /// <summary>Moves the pixel accessor to the next pixel</summary>
    /// <returns>The pixel accessor</returns>
    public: NUCLEX_PIXELS_API PixelIterator &operator ++() {

      // Will we still be within the image area? (normal, branch-predicted case)
      if(this->x + 1 < this->memory.Width) {
        this->current += this->bytesPerPixel;
        ++this->x;
      } else { // We're hitting the right image border

        // Can we go forward by one line?
        if(this->y + 1 < this->memory.Height) {
          this->current += this->memory.Stride - (this->x * this->bytesPerPixel);
          this->x = 0;
          ++this->y;
        } else { // We're on the last line already
          enforceIteratorCanAdvance(); // Checks and triggers an assertion if needed
          ++this->x; // Go to special iterator end position (or commence undefined behavior)
        }

      }

      return *this;

    }

    /// <summary>Returns the memory address of the iterator's current position</summary>
    /// <returns>The memory address the pixel iterator is currently at</returns>
    public: NUCLEX_PIXELS_API void *operator *() {
      enforceNotAtEndPosition();
      return this->current;
    }

    /// <summary>Returns the memory address of the iterator's current position</summary>
    /// <returns>The memory address the pixel iterator is currently at</returns>
    public: NUCLEX_PIXELS_API const void *operator *() const {
      enforceNotAtEndPosition();
      return this->current;
    }

    /// <summary>Looks up the current X coordinate of the pixel accessor</summary>
    /// <returns>The X coordinate of the pixel currently being accessed</returns>
    public: NUCLEX_PIXELS_API std::size_t GetX() const {
      enforceNotAtEndPosition();
      return this->x;
    }

    /// <summary>Looks up the current Y coordinate of the pixel accessor</summary>
    /// <returns>The Y coordinate of the pixel currently being accessed</returns>
    public: NUCLEX_PIXELS_API std::size_t GetY() const {
      enforceNotAtEndPosition();
      return this->y;
    }

    /// <summary>Returns a pointer to the current pixel</summary>
    /// <returns>A pointer to the current pixel</returns>
    public: NUCLEX_PIXELS_API const void *GetAddress() const {
      enforceNotAtEndPosition();
      return this->current;
    }

    /// <summary>Returns a pointer to the current pixel</summary>
    /// <returns>A pointer to the current pixel</returns>
    public: NUCLEX_PIXELS_API void *GetAddress() {
      enforceNotAtEndPosition();
      return this->current;
    }

    /// <summary>Checks whether another pixel accessor is at the same position</summary>
    /// <param name="other">Other pixel accessor that will be compared</param>
    /// <returns>True if the other pixel accessor is at the same position</returns>
    public: NUCLEX_PIXELS_API bool operator ==(const PixelIterator &other) const {
      return (this->current == other.current);
    }

    /// <summary>Checks whether another pixel accessor is at a different position</summary>
    /// <param name="other">Other pixel accessor that will be compared</param>
    /// <returns>True if the other pixel accessor is at a different position</returns>
    public: NUCLEX_PIXELS_API bool operator !=(const PixelIterator &other) const {
      return (this->current != other.current);
    }

    /// <summary>Moves the pixel iterator a number of pixels to the left</summary>
    /// <param name="pixelCount">Number of pixels the pixel iterator will be moved</param>
    private: void moveLeft(std::size_t pixelCount);

    /// <summary>Moves the pixel iterator a number of pixels to the right</summary>
    /// <param name="pixelCount">Number of pixels the pixel iterator will be moved</param>
    private: void moveRight(std::size_t pixelCount);

    /// <summary>Moves the pixel iterator a number of lines upwards</summary>
    /// <param name="lineCount">Number of lines the pixel iterator will be moved</param>
    private: void moveUp(std::size_t lineCount);

    /// <summary>Moves the pixel iterator a number of lines downwards</summary>
    /// <param name="lineCount">Number of lines the pixel iterator will be moved</param>
    private: void moveDown(std::size_t lineCount);

    /// <summary>Fails an assertion if the iterator is at its end position</summary>
    private: void enforceNotAtEndPosition() const;

    /// <summary>
    ///   Fails an assertion if the stride value is less than the number of bytes in
    ///   a line of the bitmap
    /// </summary>
    private: void enforceStrideLargerThanLineByteCount() const;

    /// <summary>
    ///   Fails an assertion if the iterator is at a position from where it can't retreat
    /// </summary>
    private: void enforceIteratorCanRetreat() const;

    /// <summary>
    ///   Fails an assertion if the iterator is at a position from where it can't advance
    /// </summary>
    private: void enforceIteratorCanAdvance() const;

    /// <summary>
    ///   Fails an assertion if the iterator is not at the end position
    /// </summary>
    /// <param name="checkX">X coordinate that will be checked</param>
    /// <param name="checkY">Y coordinate that will be checked</param>
    private: void enforceAtEndPosition(std::size_t checkX, std::size_t checkY) const;

    /// <summary>Bitmap memory the pixel iterator is accessing</summary>
    private: BitmapMemory memory;
    /// <summary>Current X coordinate of the iterator</summary>
    private: std::size_t x;
    /// <summary>Current Y coordinate of the iterator</summary>
    private: std::size_t y;
    /// <summary>Current address the pixel iterator is at</summary>
    private: std::uint8_t *current;
    /// <summary>Number of bytes in a single pixel</summary>
    private: std::size_t bytesPerPixel;

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Pixels

#endif // NUCLEX_PIXELS_PIXELITERATOR_H
