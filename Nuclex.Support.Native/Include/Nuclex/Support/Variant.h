#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_VARIANT_H
#define NUCLEX_SUPPORT_VARIANT_H

#include "Config.h"
#include "Any.h"
#include "VariantType.h"

#include <cstdint>
#include <string>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps a value of an arbitrary type</summary>
  /// <remarks>
  ///   Whatever you do, only use this to interface with scripting languages or where
  ///   it really makes sense. C++ is not a dynamically typed language and a template-based
  ///   design is always a better choice. If you need to design interfaces that pass
  ///   changing types to internal implementations, consider
  ///   the <see cref="Any" /> class instead.
  /// </remarks>
  class Variant {

    /// <summary>Initializes a new, empty variant</summary>
    public: NUCLEX_SUPPORT_API Variant() : type(VariantType::Empty) {}

    /// <summary>Initializes a variant to a boolean value</summary>
    /// <param name="booleanValue">Boolean value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(bool booleanValue) :
      type(VariantType::Boolean), booleanValue(booleanValue) {}

    /// <summary>Initializes a variant to an unsigned 8 bit integer value</summary>
    /// <param name="uint8Value">Unsigned 8 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::uint8_t uint8Value) :
      type(VariantType::Uint8), uint8Value(uint8Value) {}

    /// <summary>Initializes a variant to a signed 8 bit integer value</summary>
    /// <param name="int8Value">Signed 8 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::int8_t int8Value) :
      type(VariantType::Int8), int8Value(int8Value) {}

    /// <summary>Initializes a variant to an unsigned 16 bit integer value</summary>
    /// <param name="uint16Value">Unsigned 16 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::uint16_t uint16Value) :
      type(VariantType::Uint16), uint16Value(uint16Value) {}

    /// <summary>Initializes a variant to a signed 16 bit integer value</summary>
    /// <param name="int16Value">Signed 16 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::int16_t int16Value) :
      type(VariantType::Int16), int16Value(int16Value) {}

    /// <summary>Initializes a variant to an unsigned 32 bit integer value</summary>
    /// <param name="uint32Value">Unsigned 32 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::uint32_t uint32Value) :
      type(VariantType::Uint32), uint32Value(uint32Value) {}

    /// <summary>Initializes a variant to a signed 32 bit integer value</summary>
    /// <param name="int32Value">Signed 32 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::int32_t int32Value) :
      type(VariantType::Int32), int32Value(int32Value) {}

    /// <summary>Initializes a variant to an unsigned 64 bit integer value</summary>
    /// <param name="uint64Value">Unsigned 64 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::uint64_t uint64Value) :
      type(VariantType::Uint64), uint64Value(uint64Value) {}

    /// <summary>Initializes a variant to a signed 64 bit integer value</summary>
    /// <param name="int64Value">Signed 64 bit integer value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(std::int64_t int64Value) :
      type(VariantType::Int64), int64Value(int64Value) {}

    /// <summary>Initializes a variant to a floating point value</summary>
    /// <param name="floatValue">Floating point value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(float floatValue) :
      type(VariantType::Float), floatValue(floatValue) {}

    /// <summary>Initializes a variant to a double precision floating point value</summary>
    /// <param name="doubleValue">
    ///   Double precision floating point value the variant will hold
    /// </param>
    public: NUCLEX_SUPPORT_API Variant(double doubleValue) :
      type(VariantType::Double), doubleValue(doubleValue) {}

    /// <summary>Initializes a variant to hold a string</summary>
    /// <param name="stringValue">String that the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(const std::string &stringValue) :
      type(VariantType::String) {
      new(this->stringValueBytes) std::string(stringValue);
    }

    /// <summary>Initializes a variant to hold a wide string</summary>
    /// <param name="wstringValue">Wide string that the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(const std::wstring &wstringValue) :
      type(VariantType::WString) {
      new(this->wstringValueBytes) std::wstring(wstringValue);
    }

    /// <summary>Initializes a variant to hold an opaquely typed value</summary>
    /// <param name="anyValue">Opaquely typed value the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(const Any &anyValue) :
      type(VariantType::Any) {
      new(this->anyValueBytes) Any(anyValue);
    }

    /// <summary>Initializes a variant to hold a void pointer</summary>
    /// <param name="pointerValue">Pointer that the variant will hold</param>
    public: NUCLEX_SUPPORT_API Variant(void *pointerValue) :
      type(VariantType::VoidPointer), pointerValue(pointerValue) {}

    /// <summary>Initializes the variant as a copy of another variant</summary>
    /// <param name="other">Other variant that will be copied</param>
    public: NUCLEX_SUPPORT_API Variant(const Variant &other);

    /// <summary>Initializes the variant by taking over the contents of another</summary>
    /// <param name="other">Other variant that will be taken over</param>
    public: NUCLEX_SUPPORT_API Variant(Variant &&other);

    /// <summary>Destroys the variant and frees any memory used</summary>
    public: NUCLEX_SUPPORT_API ~Variant() {
      free();
    }

    /// <summary>Checks whether the variant is currently not holding a value</summary>
    /// <returns>True if the variant is empty</returns>
    public: NUCLEX_SUPPORT_API bool IsEmpty() const {
      return (this->type == VariantType::Empty);
    }

    /// <summary>Returns the value held by the variant as a boolean</summary>
    /// <returns>The variant's value as a boolean</returns>
    /// <remarks>
    ///   Integer or floating point values will be true if they are any value other than
    ///   zero, strings will be lexically casted and objects will be true if they are
    ///   not null pointers.
    /// </remarks>
    public: NUCLEX_SUPPORT_API bool ToBoolean() const;

    /// <summary>Returns the value held by the variant as an unsigned 8 bit integer</summary>
    /// <returns>The variant's value as an unsigned 8 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::uint8_t ToUint8() const;

    /// <summary>Returns the value held by the variant as a signed 8 bit integer</summary>
    /// <returns>The variasnt's value as a signed 8 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::int8_t ToInt8() const;

    /// <summary>Returns the value held by the variant as an unsigned 16 bit integer</summary>
    /// <returns>The variant's value as an unsigned 16 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::uint16_t ToUint16() const;

    /// <summary>Returns the value held by the variant as a signed 16 bit integer</summary>
    /// <returns>The variasnt's value as a signed 16 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::int16_t ToInt16() const;

    /// <summary>Returns the value held by the variant as an unsigned 32 bit integer</summary>
    /// <returns>The variant's value as an unsigned 32 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::uint32_t ToUint32() const;

    /// <summary>Returns the value held by the variant as a signed 32 bit integer</summary>
    /// <returns>The variasnt's value as a signed 32 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::int32_t ToInt32() const;

    /// <summary>Returns the value held by the variant as an unsigned 64 bit integer</summary>
    /// <returns>The variant's value as an unsigned 64 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::uint64_t ToUint64() const;

    /// <summary>Returns the value held by the variant as a signed 64 bit integer</summary>
    /// <returns>The variasnt's value as a signed 64 bit integer</returns>
    public: NUCLEX_SUPPORT_API std::int64_t ToInt64() const;

    /// <summary>Returns the value held by the variant as a floating point value</summary>
    /// <returns>The variasnt's value as a floating point value</returns>
    public: NUCLEX_SUPPORT_API float ToFloat() const;

    /// <summary>
    ///   Returns the value held by the variant as a double precision floating point value
    /// </summary>
    /// <returns>The variasnt's value as a double precision floating point value</returns>
    public: NUCLEX_SUPPORT_API double ToDouble() const;

    /// <summary>Returns the value held by the variant as a string</summary>
    /// <returns>The variasnt's value as a string</returns>
    public: NUCLEX_SUPPORT_API std::string ToString() const;

    /// <summary>Returns the value held by the variant as a wide string</summary>
    /// <returns>The variasnt's value as a wide string</returns>
    public: NUCLEX_SUPPORT_API std::wstring ToWString() const;

    /// <summary>Returns the value held by the variant as an opaquely typed value</summary>
    /// <returns>The variasnt's value as an opaquely typed value</returns>
    public: NUCLEX_SUPPORT_API Any ToAny() const;

    /// <summary>Returns the value held by the variant as a void pointer</summary>
    /// <returns>The variasnt's value as a void pointer</returns>
    public: NUCLEX_SUPPORT_API void *ToVoidPointer() const;

    /// <summary>Checks whether the variant is holding a numeric value</summary>
    /// <returns>True if the variant is holding a numeric value</returns>
    public: NUCLEX_SUPPORT_API bool IsNumber() const {
      switch(this->type) {
        case VariantType::Uint8:
        case VariantType::Int8:
        case VariantType::Uint16:
        case VariantType::Int16:
        case VariantType::Uint32:
        case VariantType::Int32:
        case VariantType::Uint64:
        case VariantType::Int64:
        case VariantType::Float:
        case VariantType::Double: {
          return true;
        }
        default: { return false; }
      }
    }

    /// <summary>Checks whether the variant is holding a string</summary>
    /// <returns>True if the variant is holding a string</returns>
    public: NUCLEX_SUPPORT_API bool IsString() const {
      switch(this->type) {
        case VariantType::String:
        case VariantType::WString: {
          return true;
        }
        default: { return false; }
      }
    }

    /// <summary>Retrieves the type that is currently stored by the variant</summary>
    /// <returns>The type stored by the variant</returns>
    public: NUCLEX_SUPPORT_API VariantType GetType() const {
      return this->type;
    }

    /// <summary>Assigns a boolean value to the variant</summary>
    /// <param name="newValue">Boolean that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(bool newValue) {
      free();
      this->booleanValue = newValue;
      this->type = VariantType::Boolean;
      return *this;
    }

    /// <summary>Assigns an unsigned 8 bit integer value to the variant</summary>
    /// <param name="newValue">Unsigned 8 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::uint8_t newValue) {
      free();
      this->uint8Value = newValue;
      this->type = VariantType::Uint8;
      return *this;
    }

    /// <summary>Assigns a signed 8 bit integer value to the variant</summary>
    /// <param name="newValue">Signed 8 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::int8_t newValue) {
      free();
      this->int8Value = newValue;
      this->type = VariantType::Int8;
      return *this;
    }

    /// <summary>Assigns an unsigned 16 bit integer value to the variant</summary>
    /// <param name="newValue">Unsigned 16 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::uint16_t newValue) {
      free();
      this->uint16Value = newValue;
      this->type = VariantType::Uint16;
      return *this;
    }

    /// <summary>Assigns a signed 16 bit integer value to the variant</summary>
    /// <param name="newValue">Signed 16 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::int16_t newValue) {
      free();
      this->int16Value = newValue;
      this->type = VariantType::Int16;
      return *this;
    }

    /// <summary>Assigns an unsigned 32 bit integer value to the variant</summary>
    /// <param name="newValue">Unsigned 32 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::uint32_t newValue) {
      free();
      this->uint32Value = newValue;
      this->type = VariantType::Uint32;
      return *this;
    }

    /// <summary>Assigns a signed 32 bit integer value to the variant</summary>
    /// <param name="newValue">Signed 32 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::int32_t newValue) {
      free();
      this->int32Value = newValue;
      this->type = VariantType::Int32;
      return *this;
    }

    /// <summary>Assigns an unsigned 64 bit integer value to the variant</summary>
    /// <param name="newValue">Unsigned 64 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::uint64_t newValue) {
      free();
      this->uint64Value = newValue;
      this->type = VariantType::Uint64;
      return *this;
    }

    /// <summary>Assigns a signed 64 bit integer value to the variant</summary>
    /// <param name="newValue">Signed 64 bit integer value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(std::int64_t newValue) {
      free();
      this->int64Value = newValue;
      this->type = VariantType::Int64;
      return *this;
    }

    /// <summary>Assigns a floating point value to the variant</summary>
    /// <param name="newValue">Floating point value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(float newValue) {
      free();
      this->floatValue = newValue;
      this->type = VariantType::Float;
      return *this;
    }

    /// <summary>Assigns a double precision floating point value to the variant</summary>
    /// <param name="newValue">
    ///   Double precision floating point value that will be assigned
    /// </param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(double newValue) {
      free();
      this->doubleValue = newValue;
      this->type = VariantType::Double;
      return *this;
    }

    /// <summary>Assigns a string to the variant</summary>
    /// <param name="newValue">String that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(const std::string &newValue) {
      free();
      new(this->stringValueBytes) std::string(newValue);
      this->type = VariantType::String;
      return *this;
    }

    /// <summary>Assigns a wide string to the variant</summary>
    /// <param name="newValue">Wide string that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(const std::wstring &newValue) {
      free();
      new(this->wstringValueBytes) std::wstring(newValue);
      this->type = VariantType::WString;
      return *this;
    }

    /// <summary>Assigns an opaquely typed value to the variant</summary>
    /// <param name="newValue">Opaquely types value that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(const Any &newValue) {
      free();
      new(this->anyValueBytes) Any(newValue);
      this->type = VariantType::Any;
      return *this;
    }

    /// <summary>Assigns a pointer to the variant</summary>
    /// <param name="newValue">Pointer that will be assigned</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(void *newValue) {
      free();
      this->pointerValue = newValue;
      this->type = VariantType::VoidPointer;
      return *this;
    }

    /// <summary>Assigns a variant to hold the same value as another variant</summary>
    /// <param name="other">Other variant whose value will be copied</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(const Variant &other);

    /// <summary>Assigns a variant to hold the same value as another variant</summary>
    /// <param name="other">Other variant whose value will be copied</param>
    /// <returns>The variant itself</returns>
    public: NUCLEX_SUPPORT_API Variant &operator =(Variant &&other);

    /// <summary>Frees all memory used by the variant</summary>
    private: void free() {
      switch(this->type) {
        case VariantType::String: {
          reinterpret_cast<std::string *>(this->stringValueBytes)->~basic_string();
          break;
        }
        case VariantType::WString: {
          reinterpret_cast<std::wstring *>(this->wstringValueBytes)->~basic_string();
          break;
        }
        case VariantType::Any: {
          reinterpret_cast<Any *>(this->anyValueBytes)->~Any();
          break;
        }
        default: {} // Avoids compiler warnings about unhandled enum members
      }
    }

    /// <summary>Type of value that the variant is holding</summary>
    private: VariantType type;
    /// <summary>Value held by the variant</summary>
    private: union {
      /// <summary>Boolean value, if the variant is holding that type</summary>
      bool booleanValue;
      /// <summary>Unsigned 8 bit integer value, if the variant is holding that type</summary>
      std::uint8_t uint8Value;
      /// <summary>Signed 8 bit integer value, if the variant is holding that type</summary>
      std::int8_t int8Value;
      /// <summary>Unsigned 16 bit integer value, if the variant is holding that type</summary>
      std::uint16_t uint16Value;
      /// <summary>Signed 16 bit integer value, if the variant is holding that type</summary>
      std::int16_t int16Value;
      /// <summary>Unsigned 32 bit integer value, if the variant is holding that type</summary>
      std::uint32_t uint32Value;
      /// <summary>Signed 32 bit integer value, if the variant is holding that type</summary>
      std::int32_t int32Value;
      /// <summary>Unsigned 64 bit integer value, if the variant is holding that type</summary>
      std::uint64_t uint64Value;
      /// <summary>Signed 64 bit integer value, if the variant is holding that type</summary>
      std::int64_t int64Value;
      /// <summary>Floating point value, if the variant is holding that type</summary>
      float floatValue;
      /// <summary>
      ///   Double precision floating point value, if the variant is holding that type
      /// </summary>
      double doubleValue;
      /// <summary>String value, if the variant is holding that type</summary>
      std::uint8_t stringValueBytes[sizeof(std::string)];
      /// <summary>Wide string value, if the variant is holding that type</summary>
      std::uint8_t wstringValueBytes[sizeof(std::wstring)];
      /// <summary>Opaque value of an arbitrary type, if the variant is holding that</summary>
      std::uint8_t anyValueBytes[sizeof(Any)];
      /// <summary>Void pointer value, if the variant is holding that type</summary>
      void *pointerValue;
    };

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_VARIANT_H
