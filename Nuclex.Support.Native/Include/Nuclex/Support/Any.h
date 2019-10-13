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

#ifndef NUCLEX_SUPPORT_ANY_H
#define NUCLEX_SUPPORT_ANY_H

#include <typeinfo>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Opaquely wraps a value of an arbitrary type</summary>
  class Any {

    #pragma region struct GenericValueHolder

    /// <summary>Base class to the holder for the value wrapped by the any</summary>
    protected: struct GenericValueHolder {

      /// <summary>Immediately releases all resources owned by the instance</summary>
      public: virtual ~GenericValueHolder() {}

      /// <summary>Creates a clone of the instance, also copying the value</summary>
      /// <returns>The cloned instance</returns>
      public: virtual GenericValueHolder *Clone() const = 0;

      /// <summary>Returns the type stored in the wrapper</summary>
      /// <returns>The type the wrapper is storing</returns>
      public: virtual const std::type_info &GetType() const = 0;

    };

    #pragma endregion // struct GenericValueHolder

    #pragma region struct ValueHolder

    /// <summary>Strongly typed container for the value the any is carrying</summary>
    private: template<typename TValue> struct  ValueHolder : public GenericValueHolder {

      /// <summary>Initializes a new value holder for the specified value</summary>
      /// <param name="value">Value the holder will carry</param>
      public: ValueHolder(const TValue &value) :
        value(value) {}

      /// <summary>Immediately releases all resources owned by the instance</summary>
      public: virtual ~ValueHolder() {}

      /// <summary>Creates a clone of the instance, also copying the value</summary>
      /// <returns>The cloned instance</returns>
      public: ValueHolder *Clone() const override {
        return new ValueHolder<TValue>(this->value);
      }

      /// <summary>Returns the type stored in the wrapper</summary>
      /// <returns>The type the wrapper is storing</returns>
      public: const std::type_info &GetType() const override { return typeid(TValue); }

      /// <summary>Retrieves the value stored in the value holder</summary>
      /// <returns>The value stored in the value holder</returns>
      public: const TValue &Get() const { return this->value; }

      /// <summary>Value that is being carried by the any</summary>
      private: TValue value;

    };

    #pragma endregion // struct ValueHolder

    /// <summary>Initializes a new any containing the specified value</summary>
    /// <param name="value">Value that will be carried by the any</param>
    public: template<typename TValue> Any(const TValue &value) :
      valueHolder(new ValueHolder<TValue>(value)) {}

    /// <summary>Initializes a new any copying the contents of an existing instance</summary>
    /// <param name="other">Other instance that will be copied</param>
    public: Any(const Any &other) :
      valueHolder(other.valueHolder->Clone()) {}

    /// <summary>Frees all memory used by the instance</summary>
    public: ~Any() {
      delete this->valueHolder;
    }

    /// <summary>Assigns the contents of another any to this instance</summary>
    /// <param name="other">Other any whose contents will be assigned to this one</param>
    /// <returns>The current any after the value has been assigned</returns>
    public: Any &operator =(const Any &other) {
      delete this->valueHolder;
      this->valueHolder = nullptr; // In case clone throws
      this->valueHolder = other.valueHolder->Clone();
      return *this;
    }

    /// <summary>Retrieves the value stored in the any</summary>
    /// <typeparam name="TValue">Type of value that will be retrieved from the any</typeparam>
    /// <returns>The value stored by the any</returns>
    public: template<typename TValue> const TValue &Get() const {
      typedef ValueHolder<TValue> TValueHolder;

      const std::type_info &type = typeid(TValue);
      if(type != valueHolder->GetType()) {
        throw std::bad_cast(); // "Type is different from the value stored by the 'Any' instance"
      }

      return static_cast<TValueHolder *>(this->valueHolder)->Get();
    }

    /// <summary>Value holder that carries the value stored in the any</summary>
    private: GenericValueHolder *valueHolder;

  };

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Extracts the value stored in the any</summary>
  /// <typeparam name="TValue">Type of the value that will be retrieved</typeparam>
  /// <param name="any">Any whose stored value will be retrieved</param>
  /// <returns>The value that is stored inside the any</returns>
  template<typename TValue>
  const TValue &any_cast(const Any &any) {
    return any.Get<TValue>();
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_ANY_H
