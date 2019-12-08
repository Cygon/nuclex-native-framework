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

#ifndef NUCLEX_STORAGE_XML_XMLREADER_H
#define NUCLEX_STORAGE_XML_XMLREADER_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Reader.h"
#include "Nuclex/Storage/Xml/XmlReadEvent.h"
#include "Nuclex/Storage/Xml/XmlBinaryFormat.h"

#include <memory>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data using the XML format</summary>
  class XmlReader : Reader {

    #pragma region class AttributeScope

    /// <summary>Automatically leaves an attribute when the scope is destroyed</summary>
    public: class AttributeScope {

      /// <summary>Initializes a new attribute scope using the specified XML reader</summary>
      /// <param name="reader">XML reader the attribute scope will use</param>
      public: NUCLEX_STORAGE_API AttributeScope(XmlReader &reader) :
        reader(reader) {}

      /// <summary>Initializes a new attribute scope using the specified XML reader</summary>
      /// <param name="reader">XML reader the attribute scope will use</param>
      public: NUCLEX_STORAGE_API AttributeScope(const std::shared_ptr<XmlReader> &reader) :
        reader(*reader.get()) {}

      /// <summary>Destroys the attribute scope and leaves the attribute again</summary>
      public: NUCLEX_STORAGE_API ~AttributeScope() {
        this->reader.LeaveAttribute();
      }

      private: AttributeScope(const AttributeScope &);
      private: AttributeScope &operator =(const AttributeScope &);

      /// <summary>XML reader in which the scope will leave the current attribute</summary>
      private: XmlReader &reader;

    };

    #pragma endregion // class AttributeScope

    /// <summary>Destroys the XML reader</summary>
    public: NUCLEX_STORAGE_API virtual ~XmlReader() {}

    /// <summary>Retrieves the currently selected binary data format</summary>
    /// <returns>The format in which binary data will be read</returns>
    public: virtual XmlBinaryFormat GetBinaryFormat() const = 0;

    /// <summary>Selects the binary data format to use for reading binary data</summary>
    /// <param name="binaryFormat">Format in which binary data will be read</param>
    public: virtual void SetBinaryFormat(XmlBinaryFormat binaryFormat) = 0;

    /// <summary>Reads from XML plaintext up until the next event is encountered</summary>
    /// <returns>The type of event encountered when parsing</returns>
    public: virtual XmlReadEvent Read() = 0;

    /// <summary>Retrieves the name of the last element that was entered or exited</summary>
    /// <returns>The name of the last element entered or exited</returns>
    public: virtual const std::string &GetElementName() const = 0;

    /// <summary>Counts the number of attributes in the current element</summary>
    /// <returns>The number of attributes present in the current element</summary>
    public: virtual std::size_t CountAttributes() const = 0;

    /// <summary>Retrieves the name of the attribute with the specified index</summary>
    /// <param name="index">Index of the attribue whose name will be looked up</param>
    /// <returns>The name of the attribute with the specified index</returns>
    public: virtual const std::string &GetAttributeName(std::size_t index) const = 0;

    /// <summary>Enters the attribute with the specified name</summary>
    /// <param name="attributeName">Name of the attribute that will be entered</param>
    public: void EnterAttribute(const std::string &attributeName) {
      if(!TryEnterAttribute(attributeName)) {
        throw std::runtime_error(
          std::string("Attribute '") + attributeName + "' doesn't exist"
        );
      }
    }

    /// <summary>Try to enter the attribute with the specified name</summary>
    /// <param name="attributeName">Name of the attribute that will be entered</param>
    /// <returns>True if the attribute existed and was entered, otherwise false</returns>
    public: virtual bool TryEnterAttribute(const std::string &attributeName) = 0;

    /// <summary>Leaves the currently entered attribute again</summary>
    public: virtual void LeaveAttribute() = 0;

    /// <summary>Retrieves the value of an attribute from the current element</summary>
    /// <param name="attributeName">Name of the attribue whose value will be retrieved</param>
    /// <returns>The value of the queried attribute</returns>
    public: template <typename TValue> TValue GetAttributeValue(
      const std::string &attributeName
    ) {
      EnterAttribute(attributeName);
      {
        AttributeScope scope(*this);

        TValue value;
        Read(value);

        return value;
      }
    }

    /// <summary>Retrieves the value of an attribute from the current element</summary>
    /// <param name="attributeName">Name of the attribue whose value will be retrieved</param>
    /// <param name="defaultValue">
    ///   Value that will be returned when the attribue doesn't exist
    /// </param>
    /// <returns>The attribute's default value</returns>
    public: template <typename TValue> TValue GetAttributeValue(
      const std::string &attributeName, const TValue &defaultValue
    ) {
      if(TryEnterAttribute(attributeName)) {
        AttributeScope scope(*this);

        TValue value;
        Read(value);

        return value;
      } else {
        return defaultValue;
      }
    }

    // Unhide the overloaded Read() methods in the base class
    // See http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.9
    using Reader::Read;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLREADER_H
