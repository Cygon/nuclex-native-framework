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

#ifndef NUCLEX_PIXELS_STORAGE_OPTIONALBITMAP_H
#define NUCLEX_PIXELS_STORAGE_OPTIONALBITMAP_H

#include "Nuclex/Pixels/Config.h"
#include "Nuclex/Pixels/Bitmap.h"

#include <cstdint>
#include <utility>
#include <stdexcept>

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wrapper to pass a bitmap as an optional return value</summary>
  /// <remarks>
  ///   This is the equivalent of returning std::optional&lt;Bitmap&gt; in C++17. It either
  ///   transmits a Bitmap instance or it has no value.
  class OptionalBitmap {

    /// <summary>Initializes an empty optional bitmap wrapper</summary>
    public: NUCLEX_PIXELS_API OptionalBitmap() :
      hasValue(false) {}
    
    /// <summary>
    ///   Initializes an optional bitmap wrapper taking over the specified bitmap
    /// </summary> 
    /// <param name="bitmap">Bitmap the wrapper will carry</param>
    public: NUCLEX_PIXELS_API OptionalBitmap(Bitmap &&bitmap) :
      hasValue(true) {
      new(this->bitmapMemory) Bitmap(std::move(bitmap));
    }

    /// <summary>Frees all memory used by the instance</summary>
    public: NUCLEX_PIXELS_API ~OptionalBitmap() {
      if(this->hasValue) {
        reinterpret_cast<Bitmap *>(this->bitmapMemory)->~Bitmap();
      }
    }

    /// <summary>Initializes a new optional bitmap wrapper as a copy</summary>
    /// <param name="other">Other bitmap wrapper that will be copied</param>
    public: NUCLEX_PIXELS_API OptionalBitmap(const OptionalBitmap &other) :
      hasValue(other.hasValue) {
      new(this->bitmapMemory) Bitmap(
        *reinterpret_cast<const Bitmap *>(other.bitmapMemory)
      );
    }

    /// <summary>Initializes a new optional bitmap wrapper taking over another</summary>
    /// <param name="other">Other bitmap wrapper whose contents will be taken over</param>
    public: NUCLEX_PIXELS_API OptionalBitmap(OptionalBitmap &&other) :
      hasValue(other.hasValue) {

      // If the other wrapper held a bitmap, take it over. We know that the Bitmap
      // class doesn't hold any pointers to its own members, so it's safe to move
      // without invoking its move constructor.
      if(other.hasValue) {
        for(std::size_t index = 0; index < sizeof(Bitmap); ++index) {
          this->bitmapMemory[index] = other.bitmapMemory[index];
        }
        other.hasValue = false;
      }
    }

    /// <summary>Assigns the contents of another optional bitmap wrapper to this</summary>
    /// <parma name="other">Other bitmap wrapper that will be assigned</param>
    /// <returns>A reference to this bitmap wrapper</returns>
    public: NUCLEX_PIXELS_API OptionalBitmap &operator =(const OptionalBitmap &other) {
      if(this->hasValue) {
        reinterpret_cast<Bitmap *>(this->bitmapMemory)->~Bitmap();
      }

      this->hasValue = other.hasValue;
      if(other.hasValue) {
        new(this->bitmapMemory) Bitmap(
          *reinterpret_cast<const Bitmap *>(other.bitmapMemory)
        );
      }

      return *this;
    }

    /// <summary>Assigns the contents of another optional bitmap wrapper to this</summary>
    /// <parma name="other">Other bitmap wrapper that will be assigned</param>
    /// <returns>A reference to this bitmap wrapper</returns>
    public: NUCLEX_PIXELS_API OptionalBitmap &operator =(OptionalBitmap &&other) {
      if(this->hasValue) {
        reinterpret_cast<Bitmap *>(this->bitmapMemory)->~Bitmap();
      }

      this->hasValue = other.hasValue;

      // If the other wrapper held a bitmap, take it over. We know that the Bitmap
      // class doesn't hold any pointers to its own members, so it's safe to move
      // without invoking its move constructor.
      if(other.hasValue) {
        for(std::size_t index = 0; index < sizeof(Bitmap); ++index) {
          this->bitmapMemory[index] = other.bitmapMemory[index];
        }
        other.hasValue = false;
      }

      return *this;
    }

    /// <summary>Checks whether the bitmap wrapper is carrying a bitmap</summary>
    /// <returns>True if the wrapper carries a bitmap, false otherwise</returns>
    public: NUCLEX_PIXELS_API bool HasValue() const { return this->hasValue; }

    /// <summary>Takes the bitmap out of the wrapper</summary>
    /// <returnsThe bitmap stored in the wrapper</returns>
    public: NUCLEX_PIXELS_API Bitmap &&Take() {
      if(!this->hasValue) {
        throw std::logic_error("Cannot take Bitmap, OptionalBitmap is empty");
      }
      this->hasValue = false; // skips destructor call for bitmapMemory. This is okay.
      return std::move(*reinterpret_cast<Bitmap *>(this->bitmapMemory));
    }

    /// <summary>Whether the optional bitmap is currently carrying a value</summary>
    public: bool hasValue;
    /// <summary>Holds the bitmap carried by the wrapper, if any</summary>
    public: std::uint8_t bitmapMemory[sizeof(Bitmap)];

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage

#endif // NUCLEX_PIXELS_STORAGE_OPTIONALBITMAP_H
