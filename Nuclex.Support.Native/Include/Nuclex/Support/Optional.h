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

#ifndef NUCLEX_SUPPORT_OPTIONAL_H
#define NUCLEX_SUPPORT_OPTIONAL_H

#include "Nuclex/Support/Config.h"

#if !defined(NUCLEX_SUPPORT_SOURCE)
  #warning Nuclex::Support::Optional has been deprecated in favor of C++17 std::optional
#endif

#include <type_traits>
#include <stdexcept>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores either a value or nothing, allowing optional values on the stack</summary>
  /// <typeparam name="TValue">Type of value the optional will contain</typeparam>
  /// <remarks>
  ///   This library targets C++14, where std::optional hadn't been introduced yet.
  ///   If you are targeting C++17 or later, there is no need to use this class.
  /// </remarks>
  template<typename TValue>
  class Optional {

    /// <summary>An <see cref="Optional" /> instance that is empty</summary>
    public: NUCLEX_SUPPORT_API const static Optional Empty;

    /// <summary>Initializes a new optional not holding a value</summary>
    public: NUCLEX_SUPPORT_API Optional() :
      carriesValue(false) {}

    /// <summary>Initializes a new Optional containing the specified value</summary>
    /// <param name="value">Value that will be carried by the optional</param>
    public: NUCLEX_SUPPORT_API Optional(const TValue &value) :
      carriesValue(true) {
      new(this->valueMemory) TValue(value);
    }

    /// <summary>Initializes a new Optional containing the specified value</summary>
    /// <param name="value">Value that will be carried by the optional</param>
    public: NUCLEX_SUPPORT_API Optional(TValue &&value) :
      carriesValue(true) {
      new(this->valueMemory) TValue(std::move(value));
    }

    /// <summary>Initializes a new optional copying the contents of an existing instance</summary>
    /// <param name="other">Other instance that will be copied</param>
    public: NUCLEX_SUPPORT_API Optional(const Optional &other) :
      carriesValue(other.carriesValue) {
      if(this->carriesValue) {
        new(this->valueMemory) TValue(*reinterpret_cast<const TValue *>(other.valueMemory));
      }
    }

    /// <summary>Initializes a new Optional taking over an existing instance</summary>
    /// <param name="other">Other instance that will be taken over</param>
    public: NUCLEX_SUPPORT_API Optional(Optional &&other) :
      carriesValue(other.carriesValue) {
      if(other.carriesValue) {
        new(this->valueMemory) TValue(
          std::move(*reinterpret_cast<TValue *>(other.valueMemory))
        );

        reinterpret_cast<TValue *>(other.valueMemory)->~TValue();
        other.carriesValue = false;
      }
    }

    /// <summary>Frees all memory used by the instance</summary>
    public: NUCLEX_SUPPORT_API ~Optional() {
      if(this->carriesValue) {
        reinterpret_cast<TValue *>(this->valueMemory)->~TValue();
      }
    }

    /// <summary>Checks whether the Any is currently holding a value</summary>
    /// <returns>True if the Any holds a value, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool HasValue() const {
      return this->carriesValue;
    }

    /// <summary>Destroys the contents of the Any</summary>
    public: NUCLEX_SUPPORT_API void Reset() {
      if(this->carriesValue) {
        reinterpret_cast<TValue *>(this->valueMemory)->~TValue();
        this->carriesValue = false;
      }
    }

    /// <summary>Assigns the contents of another Optional to this instance</summary>
    /// <param name="other">Other Optional whose contents will be assigned to this one</param>
    /// <returns>The current Optional after the value has been assigned</returns>
    public: NUCLEX_SUPPORT_API Optional &operator =(const Optional &other) {
      if(this->carriesValue) {
        reinterpret_cast<TValue *>(this->valueMemory)->~TValue();
      }
      if(other.carriesValue) {
        new(this->valueMemory) TValue(*reinterpret_cast<const TValue *>(other.valueMemory));
      }

      this->carriesValue = other.carriesValue;
      return *this;
    }

    /// <summary>Moves the contents of another Optional to this instance</summary>
    /// <param name="other">Other Optional whose contents will be moved to this one</param>
    /// <returns>The current Optional after the value has been moved</returns>
    public: NUCLEX_SUPPORT_API Optional &operator =(Optional &&other) {
      if(this->carriesValue) {
        reinterpret_cast<TValue *>(this->valueMemory)->~TValue();
      }
      if(other.carriesValue) {
        new(this->valueMemory) TValue(
          std::move(*reinterpret_cast<TValue *>(other.valueMemory))
        );
        reinterpret_cast<TValue *>(other.valueMemory)->~TValue();
        this->carriesValue = true;
        other.carriesValue = false;
      } else{
        this->carriesValue = false;
      }
      return *this;
    }

    /// <summary>Retrieves the value stored in the any</summary>
    /// <typeparam name="TValue">Type of value that will be retrieved from the any</typeparam>
    /// <returns>The value stored by the any</returns>
    public: NUCLEX_SUPPORT_API const TValue &Get() const {
      if(!this->carriesValue) {
        throw std::logic_error(u8"Optional does not contain a value");
      }

      return *reinterpret_cast<TValue *>(this->valueMemory);
    }

    /// <summary>Retrieves the value stored in the any</summary>
    /// <typeparam name="TValue">Type of value that will be retrieved from the any</typeparam>
    /// <returns>The value stored by the any</returns>
    public: NUCLEX_SUPPORT_API TValue &Get() {
      if(!this->carriesValue) {
        throw std::logic_error(u8"Optional does not contain a value");
      }

      return *reinterpret_cast<TValue *>(this->valueMemory);
    }

    /// <summary>Whether the optional container is currently holding a value</summary>
    private: bool carriesValue;
    /// <summary>Memory used to store the contained value, if any</summary>
    private: std::uint8_t valueMemory[sizeof(TValue)];

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_OPTIONAL_H
