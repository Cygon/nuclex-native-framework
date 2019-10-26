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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Variant.h"
#include "Nuclex/Support/Text/Lexical.h"
#include "Nuclex/Support/Text/StringConverter.h"

#include <stdexcept>

namespace {

  /// Message used in the exception thrown when the variant is of an unknown type
  std::string InvalidVariantTypeExceptionMessage("Invalid variant type");

} // anonymous namespace

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  Variant::Variant(const Variant &other) :
    type(other.type) {

    switch(other.type) {
      case VariantType::Empty: { break; } // Nothing more to do
      case VariantType::Boolean: { this->booleanValue = other.booleanValue; break; }
      case VariantType::Uint8: { this->uint8Value = other.uint8Value; break; }
      case VariantType::Int8: { this->int8Value = other.int8Value; break; }
      case VariantType::Uint16: { this->uint16Value = other.uint16Value; break; }
      case VariantType::Int16: { this->int16Value = other.int16Value; break; }
      case VariantType::Uint32: { this->uint32Value = other.uint32Value; break; }
      case VariantType::Int32: { this->int32Value = other.int32Value; break; }
      case VariantType::Uint64: { this->uint64Value = other.uint64Value; break; }
      case VariantType::Int64: { this->int64Value = other.int64Value; break; }
      case VariantType::Float: { this->floatValue = other.floatValue; break; }
      case VariantType::Double: { this->doubleValue = other.doubleValue; break; }
      case VariantType::String: {
        new(this->stringValueBytes) std::string(
          *reinterpret_cast<const std::string *>(other.stringValueBytes)
        );
        break;
      }
      case VariantType::WString: {
        new(this->wstringValueBytes) std::wstring(
          *reinterpret_cast<const std::wstring *>(other.wstringValueBytes)
        );
        break;
      }
      case VariantType::Any: {
        new(this->anyValueBytes) Any(
          *reinterpret_cast<const Any *>(other.anyValueBytes)
        );
        break;
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Variant::Variant(Variant &&other) :
    type(other.type) {

    switch(other.type) {
      case VariantType::Empty: { break; } // Nothing more to do
      case VariantType::Boolean: { this->booleanValue = other.booleanValue; break; }
      case VariantType::Uint8: { this->uint8Value = other.uint8Value; break; }
      case VariantType::Int8: { this->int8Value = other.int8Value; break; }
      case VariantType::Uint16: { this->uint16Value = other.uint16Value; break; }
      case VariantType::Int16: { this->int16Value = other.int16Value; break; }
      case VariantType::Uint32: { this->uint32Value = other.uint32Value; break; }
      case VariantType::Int32: { this->int32Value = other.int32Value; break; }
      case VariantType::Uint64: { this->uint64Value = other.uint64Value; break; }
      case VariantType::Int64: { this->int64Value = other.int64Value; break; }
      case VariantType::Float: { this->floatValue = other.floatValue; break; }
      case VariantType::Double: { this->doubleValue = other.doubleValue; break; }
      case VariantType::String: {
        std::string &&otherString = std::move(
          *reinterpret_cast<std::string *>(other.stringValueBytes)
        );
        new(this->stringValueBytes) std::string(std::move(otherString));
        otherString.~basic_string(); // move doesn't guarantee that destructor can be omitted
        break;
      }
      case VariantType::WString: {
        std::wstring &&otherString = std::move(
          *reinterpret_cast<std::wstring *>(other.wstringValueBytes)
        );
        new(this->wstringValueBytes) std::wstring(std::move(otherString));
        otherString.~basic_string();
        break;
      }
      case VariantType::Any: {
        Any &&otherAny = std::move(*reinterpret_cast<Any *>(other.anyValueBytes));
        new(this->anyValueBytes) Any(std::move(otherAny));
        break;
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }

    // We could just not call the destructor on the other variant's strings or anys,
    // letting their clear() method do it on assignment or destruction, but then
    // the other variant would still claim to hold a string or any when it is in
    // fact holding a potentially destroyed string or any. So we destroy and clear.
    other.type = VariantType::Empty;
  }

  // ------------------------------------------------------------------------------------------- //

  bool Variant::ToBoolean() const {
    switch(this->type) {
      case VariantType::Empty: { return false; }
      case VariantType::Boolean: { return this->booleanValue; }
      case VariantType::Uint8: { return this->uint8Value != 0; }
      case VariantType::Int8: { return this->int8Value != 0; }
      case VariantType::Uint16: { return this->uint16Value != 0; }
      case VariantType::Int16: { return this->int16Value != 0; }
      case VariantType::Uint32: { return this->uint32Value != 0; }
      case VariantType::Int32: { return this->int32Value != 0; }
      case VariantType::Uint64: { return this->uint64Value != 0; }
      case VariantType::Int64: { return this->int64Value != 0; }
      case VariantType::Float: { return this->floatValue != 0.0f; }
      case VariantType::Double: { return this->doubleValue != 0.0; }
      case VariantType::String: {
        return Text::lexical_cast<bool>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<bool>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return true; }
      case VariantType::VoidPointer: { return this->pointerValue != nullptr; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint8_t Variant::ToUint8() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return this->uint8Value; }
      case VariantType::Int8: { return static_cast<std::uint8_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::uint8_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::uint8_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::uint8_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::uint8_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::uint8_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::uint8_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint8_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint8_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint8_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return static_cast<std::uint8_t>(
          Text::wlexical_cast<int>(
            *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
          )
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::uint8_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int8_t Variant::ToInt8() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int8_t>(this->uint8Value); }
      case VariantType::Int8: { return this->int8Value; }
      case VariantType::Uint16: { return static_cast<std::int8_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::int8_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::int8_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::int8_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::int8_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::int8_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::int8_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int8_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int8_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return static_cast<std::int8_t>(
          Text::wlexical_cast<int>(
            *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
          )
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::int8_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
          );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint16_t Variant::ToUint16() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::uint16_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::uint16_t>(this->int8Value); }
      case VariantType::Uint16: { return this->uint16Value; }
      case VariantType::Int16: { return static_cast<std::uint16_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::uint16_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::uint16_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::uint16_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::uint16_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint16_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint16_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint16_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::uint16_t>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::uint16_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int16_t Variant::ToInt16() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int16_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::int16_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::int16_t>(this->uint16Value); }
      case VariantType::Int16: { return this->int16Value; }
      case VariantType::Uint32: { return static_cast<std::int16_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::int16_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::int16_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::int16_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::int16_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int16_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int16_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::int16_t>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::int16_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint32_t Variant::ToUint32() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::uint32_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::uint32_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::uint32_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::uint32_t>(this->int16Value); }
      case VariantType::Uint32: { return this->uint32Value; }
      case VariantType::Int32: { return static_cast<std::uint32_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::uint32_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::uint32_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint32_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint32_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint32_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::uint32_t>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::uint32_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int32_t Variant::ToInt32() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int32_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::int32_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::int32_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::int32_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::int32_t>(this->uint32Value); }
      case VariantType::Int32: { return this->int32Value; }
      case VariantType::Uint64: { return static_cast<std::int32_t>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<std::int32_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::int32_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int32_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int32_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::int32_t>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::int32_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::uint64_t Variant::ToUint64() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::uint64_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::uint64_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::uint64_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::uint64_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::uint64_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::uint64_t>(this->int32Value); }
      case VariantType::Uint64: { return this->uint64Value; }
      case VariantType::Int64: { return static_cast<std::uint64_t>(this->int64Value); }
      case VariantType::Float: { return static_cast<std::uint64_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::uint64_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::uint64_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::uint64_t>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::uint64_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::int64_t Variant::ToInt64() const {
    switch(this->type) {
      case VariantType::Empty: { return 0; }
      case VariantType::Boolean: { return this->booleanValue ? 1 : 0; }
      case VariantType::Uint8: { return static_cast<std::int64_t>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<std::int64_t>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<std::int64_t>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<std::int64_t>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<std::int64_t>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<std::int64_t>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<std::int64_t>(this->uint64Value); }
      case VariantType::Int64: { return this->int64Value; }
      case VariantType::Float: { return static_cast<std::int64_t>(this->floatValue); }
      case VariantType::Double: { return static_cast<std::int64_t>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<std::int64_t>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<std::int64_t>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<std::int64_t>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  float Variant::ToFloat() const {
    switch(this->type) {
      case VariantType::Empty: { return 0.0f; }
      case VariantType::Boolean: { return this->booleanValue ? 1.0f : 0.0f; }
      case VariantType::Uint8: { return static_cast<float>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<float>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<float>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<float>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<float>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<float>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<float>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<float>(this->int64Value); }
      case VariantType::Float: { return this->floatValue; }
      case VariantType::Double: { return static_cast<float>(this->doubleValue); }
      case VariantType::String: {
        return Text::lexical_cast<float>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<float>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<float>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  double Variant::ToDouble() const {
    switch(this->type) {
      case VariantType::Empty: { return 0.0; }
      case VariantType::Boolean: { return this->booleanValue ? 1.0 : 0.0; }
      case VariantType::Uint8: { return static_cast<double>(this->uint8Value); }
      case VariantType::Int8: { return static_cast<double>(this->int8Value); }
      case VariantType::Uint16: { return static_cast<double>(this->uint16Value); }
      case VariantType::Int16: { return static_cast<double>(this->int16Value); }
      case VariantType::Uint32: { return static_cast<double>(this->uint32Value); }
      case VariantType::Int32: { return static_cast<double>(this->int32Value); }
      case VariantType::Uint64: { return static_cast<double>(this->uint64Value); }
      case VariantType::Int64: { return static_cast<double>(this->int64Value); }
      case VariantType::Float: { return static_cast<double>(this->floatValue); }
      case VariantType::Double: { return this->doubleValue; }
      case VariantType::String: {
        return Text::lexical_cast<double>(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return Text::wlexical_cast<double>(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return 0; }
      case VariantType::VoidPointer: {
        return static_cast<double>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::string Variant::ToString() const {
    static std::string emptyString;
    static std::string trueString("1", 1);
    static std::string falseString("0", 1);

    switch(this->type) {
      case VariantType::Empty: { return emptyString; }
      case VariantType::Boolean: { return this->booleanValue ? trueString : falseString; }
      case VariantType::Uint8: {
        return Text::lexical_cast<std::string>(static_cast<std::uint16_t>(this->uint8Value));
      }
      case VariantType::Int8: {
        return Text::lexical_cast<std::string>(static_cast<std::int16_t>(this->int8Value));
      }
      case VariantType::Uint16: { return Text::lexical_cast<std::string>(this->uint16Value); }
      case VariantType::Int16: { return Text::lexical_cast<std::string>(this->int16Value); }
      case VariantType::Uint32: { return Text::lexical_cast<std::string>(this->uint32Value); }
      case VariantType::Int32: { return Text::lexical_cast<std::string>(this->int32Value); }
      case VariantType::Uint64: { return Text::lexical_cast<std::string>(this->uint64Value); }
      case VariantType::Int64: { return Text::lexical_cast<std::string>(this->int64Value); }
      case VariantType::Float: { return Text::lexical_cast<std::string>(this->floatValue); }
      case VariantType::Double: { return Text::lexical_cast<std::string>(this->doubleValue); }
      case VariantType::String: {
        return *reinterpret_cast<const std::string *>(this->stringValueBytes);
      }
      case VariantType::WString: {
        return Text::StringConverter::Utf8FromWide(
          *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
        );
      }
      case VariantType::Any: { return emptyString; }
      case VariantType::VoidPointer: {
        return Text::lexical_cast<std::string>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::wstring Variant::ToWString() const {
    static std::wstring emptyString;
    static std::wstring trueString(L"1", 1);
    static std::wstring falseString(L"0", 1);

    switch(this->type) {
      case VariantType::Empty: { return emptyString; }
      case VariantType::Boolean: { return this->booleanValue ? trueString : falseString; }
      case VariantType::Uint8: { return Text::wlexical_cast<std::wstring>(this->uint8Value); }
      case VariantType::Int8: { return Text::wlexical_cast<std::wstring>(this->int8Value); }
      case VariantType::Uint16: { return Text::wlexical_cast<std::wstring>(this->uint16Value); }
      case VariantType::Int16: { return Text::wlexical_cast<std::wstring>(this->int16Value); }
      case VariantType::Uint32: { return Text::wlexical_cast<std::wstring>(this->uint32Value); }
      case VariantType::Int32: { return Text::wlexical_cast<std::wstring>(this->int32Value); }
      case VariantType::Uint64: { return Text::wlexical_cast<std::wstring>(this->uint64Value); }
      case VariantType::Int64: { return Text::wlexical_cast<std::wstring>(this->int64Value); }
      case VariantType::Float: { return Text::wlexical_cast<std::wstring>(this->floatValue); }
      case VariantType::Double: { return Text::wlexical_cast<std::wstring>(this->doubleValue); }
      case VariantType::String: {
        return Text::StringConverter::WideFromUtf8(
          *reinterpret_cast<const std::string *>(this->stringValueBytes)
        );
      }
      case VariantType::WString: {
        return *reinterpret_cast<const std::wstring *>(this->wstringValueBytes);
      }
      case VariantType::Any: { return emptyString; }
      case VariantType::VoidPointer: {
        return Text::wlexical_cast<std::wstring>(
          reinterpret_cast<std::uintptr_t>(this->pointerValue)
        );
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Any Variant::ToAny() const {
    switch(this->type) {
      case VariantType::Empty: { return Any(nullptr); }
      case VariantType::Boolean: { return Any(this->booleanValue); }
      case VariantType::Uint8: { return Any(this->uint8Value); }
      case VariantType::Int8: { return Any(this->int8Value); }
      case VariantType::Uint16: { return Any(this->uint16Value); }
      case VariantType::Int16: { return Any(this->int16Value); }
      case VariantType::Uint32: { return Any(this->uint32Value); }
      case VariantType::Int32: { return Any(this->int32Value); }
      case VariantType::Uint64: { return Any(this->uint64Value); }
      case VariantType::Int64: { return Any(this->int64Value); }
      case VariantType::Float: { return Any(this->floatValue); }
      case VariantType::Double: { return Any(this->doubleValue); }
      case VariantType::String: {
        return Any(*reinterpret_cast<const std::string *>(this->stringValueBytes));
      }
      case VariantType::WString: {
        return Any(*reinterpret_cast<const std::wstring *>(this->wstringValueBytes));
      }
      case VariantType::Any: {
        return *reinterpret_cast<const Any *>(this->anyValueBytes);
      }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void *Variant::ToVoidPointer() const {
    switch(this->type) {
      case VariantType::Empty: { return nullptr; }
      case VariantType::Boolean: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->booleanValue));
      }
      case VariantType::Uint8: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->uint8Value));
      }
      case VariantType::Int8: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->int8Value));
      }
      case VariantType::Uint16: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->uint16Value));
      }
      case VariantType::Int16: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->int16Value));
      }
      case VariantType::Uint32: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->uint32Value));
      }
      case VariantType::Int32: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->int32Value));
      }
      case VariantType::Uint64: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->uint64Value));
      }
      case VariantType::Int64: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->int64Value));
      }
      case VariantType::Float: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->floatValue));
      }
      case VariantType::Double: {
        return reinterpret_cast<void *>(static_cast<std::uintptr_t>(this->doubleValue));
      }
      case VariantType::String: {
        return reinterpret_cast<void *>(
          Text::lexical_cast<std::uintptr_t>(
            *reinterpret_cast<const std::string *>(this->stringValueBytes)
          )
        );
      }
      case VariantType::WString: {
        return reinterpret_cast<void *>(
          Text::wlexical_cast<std::uintptr_t>(
            *reinterpret_cast<const std::wstring *>(this->wstringValueBytes)
          )
        );
      }
      case VariantType::Any: {
        return nullptr;
      }
      case VariantType::VoidPointer: { return this->pointerValue; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Variant &Variant::operator =(const Variant &other) {
    free();

    // Change type to 'empty' temporarily in case the copy throws an exception
    this->type = VariantType::Empty;

    switch(other.type) {
      case VariantType::Empty: { return *this; } // Nothing more to do
      case VariantType::Boolean: { this->booleanValue = other.booleanValue; break; }
      case VariantType::Uint8: { this->uint8Value = other.uint8Value; break; }
      case VariantType::Int8: { this->int8Value = other.int8Value; break; }
      case VariantType::Uint16: { this->uint16Value = other.uint16Value; break; }
      case VariantType::Int16: { this->int16Value = other.int16Value; break; }
      case VariantType::Uint32: { this->uint32Value = other.uint32Value; break; }
      case VariantType::Int32: { this->int32Value = other.int32Value; break; }
      case VariantType::Uint64: { this->uint64Value = other.uint64Value; break; }
      case VariantType::Int64: { this->int64Value = other.int64Value; break; }
      case VariantType::Float: { this->floatValue = other.floatValue; break; }
      case VariantType::Double: { this->doubleValue = other.doubleValue; break; }
      case VariantType::String: {
        new(this->stringValueBytes) std::string(
          *reinterpret_cast<const std::string *>(other.stringValueBytes)
        );
        break;
      }
      case VariantType::WString: {
        new(this->wstringValueBytes) std::wstring(
          *reinterpret_cast<const std::wstring *>(other.wstringValueBytes)
        );
        break;
      }
      case VariantType::Any: {
        new(this->anyValueBytes) Any(
          *reinterpret_cast<const Any *>(other.anyValueBytes)
        );
        break;
      }
      case VariantType::VoidPointer: { this->pointerValue = other.pointerValue; break; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }

    this->type = other.type;

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

  Variant &Variant::operator =(Variant &&other) {
    if(this == &other) {
      return *this;
    }

    free();

    // Change type to 'empty' temporarily in case the move throws an exception
    this->type = VariantType::Empty;

    switch(other.type) {
      case VariantType::Empty: { return *this; } // Nothing more to do
      case VariantType::Boolean: { this->booleanValue = other.booleanValue; break; }
      case VariantType::Uint8: { this->uint8Value = other.uint8Value; break; }
      case VariantType::Int8: { this->int8Value = other.int8Value; break; }
      case VariantType::Uint16: { this->uint16Value = other.uint16Value; break; }
      case VariantType::Int16: { this->int16Value = other.int16Value; break; }
      case VariantType::Uint32: { this->uint32Value = other.uint32Value; break; }
      case VariantType::Int32: { this->int32Value = other.int32Value; break; }
      case VariantType::Uint64: { this->uint64Value = other.uint64Value; break; }
      case VariantType::Int64: { this->int64Value = other.int64Value; break; }
      case VariantType::Float: { this->floatValue = other.floatValue; break; }
      case VariantType::Double: { this->doubleValue = other.doubleValue; break; }
      case VariantType::String: {
        std::string &&otherString = std::move(
          *reinterpret_cast<std::string *>(other.stringValueBytes)
        );
        new(this->stringValueBytes) std::string(std::move(otherString));
        otherString.~basic_string();
        break;
      }
      case VariantType::WString: {
        std::wstring &&otherString = std::move(
          *reinterpret_cast<std::wstring *>(other.stringValueBytes)
        );
        new(this->stringValueBytes) std::wstring(std::move(otherString));
        otherString.~basic_string();
        break;
      }
      case VariantType::Any: {
        Any &&otherAny = std::move(*reinterpret_cast<Any *>(other.anyValueBytes));
        new(this->anyValueBytes) Any(std::move(otherAny));
        otherAny.~Any();
        break;
      }
      case VariantType::VoidPointer: { this->pointerValue = other.pointerValue; break; }
      default: { throw std::runtime_error(InvalidVariantTypeExceptionMessage); }
    }

    this->type = other.type;

    // We could just not call the destructor on the other variant's strings or anys,
    // letting their clear() method do it on assignment or destruction, but then
    // the other variant would still claim to hold a string or any when it is in
    // fact holding a potentially destroyed string or any. So we destroy and clear.
    other.type = VariantType::Empty;

    return *this;
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
