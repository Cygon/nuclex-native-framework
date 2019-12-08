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

#ifndef NUCLEX_STORAGE_XML_XMLBLOBREADER_H
#define NUCLEX_STORAGE_XML_XMLBLOBREADER_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Xml/XmlReader.h"

#include <memory>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class Blob;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data from a chunk of XML plaintext</summary>
  class XmlBlobReader : public XmlReader {

    /// <summary>Initializes a new XML reader reading out of a blob</summary>
    /// <param name="blob">Blob the XML reader will read out of</param>
    public: NUCLEX_STORAGE_API XmlBlobReader(const std::shared_ptr<const Blob> &blob);
    /// <summary>Destroys the XML reader</summary>
    public: NUCLEX_STORAGE_API virtual ~XmlBlobReader();

    /// <summary>Retrieves the currently selected binary data format</summary>
    /// <returns>The format in which binary data will be read</returns>
    public: NUCLEX_STORAGE_API XmlBinaryFormat GetBinaryFormat() const {
      return this->binaryFormat;
    }

    /// <summary>Selects the binary data format to use for reading binary data</summary>
    /// <param name="newBinaryFormat">Format in which binary data will be read</param>
    public: NUCLEX_STORAGE_API void SetBinaryFormat(XmlBinaryFormat newBinaryFormat) {
      this->binaryFormat = newBinaryFormat;
    }

    /// <summary>Reads from XML plaintext up until the next event is encountered</summary>
    /// <returns>The type of event encountered when parsing</returns>
    public: NUCLEX_STORAGE_API XmlReadEvent Read();

    /// <summary>Retrieves the name of the last element that was entered or exited</summary>
    /// <returns>The name of the last element entered or exited</returns>
    public: NUCLEX_STORAGE_API const std::string &GetElementName() const;

    /// <summary>Counts the number of attributes in the current element</summary>
    /// <returns>The number of attributes present in the current element</summary>
    public: NUCLEX_STORAGE_API std::size_t CountAttributes() const;

    /// <summary>Retrieves the name of the attribute with the specified index</summary>
    /// <param name="index">Index of the attribue whose name will be looked up</param>
    /// <returns>The name of the attribute with the specified index</returns>
    public: NUCLEX_STORAGE_API const std::string &GetAttributeName(std::size_t index) const;

    /// <summary>Try to enter the attribute with the specified name</summary>
    /// <param name="attributeName">Name of the attribute that will be entered</param>
    /// <returns>True if the attribute existed and was entered, otherwise false</returns>
    public: NUCLEX_STORAGE_API bool TryEnterAttribute(const std::string &attributeName);

    /// <summary>Leaves the currently entered attribute again</summary>
    public: NUCLEX_STORAGE_API void LeaveAttribute();

    /// <summary>Reads a boolean from the stream</summary>
    /// <param name="target">Address of a boolean the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(bool &target);

    /// <summary>Reads an unsigned 8 bit integer from the stream</summary>
    /// <param name="target">Address of an 8 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint8_t &target);

    /// <summary>Reads a signed 8 bit integer from the stream</summary>
    /// <param name="target">Address of an 8 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int8_t &target);

    /// <summary>Reads an unsigned 16 bit integer from the stream</summary>
    /// <param name="target">Address of a 16 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint16_t &target);

    /// <summary>Reads a signed 16 bit integer from the stream</summary>
    /// <param name="target">Address of a 16 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int16_t &target);

    /// <summary>Reads an unsigned 32 bit integer from the stream</summary>
    /// <param name="target">Address of a 32 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint32_t &target);

    /// <summary>Reads a signed 32 bit integer from the stream</summary>
    /// <param name="target">Address of a 32 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int32_t &target);

    /// <summary>Reads an unsigned 64 bit integer from the stream</summary>
    /// <param name="target">Address of a 64 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::uint64_t &target);

    /// <summary>Reads a signed 64 bit integer from the stream</summary>
    /// <param name="target">Address of a 64 bit integer the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::int64_t &target);

    /// <summary>Reads a floating point value from the stream</summary>
    /// <param name="target">Address of a floating point value that will be read into</param>
    public: NUCLEX_STORAGE_API void Read(float &target);

    /// <summary>Reads a double precision floating point value from the stream</summary>
    /// <param name="target">
    ///   Address of a double precision floating point value that will be read into
    /// </param>
    public: NUCLEX_STORAGE_API void Read(double &target);

    /// <summary>Reads a string from the stream</summary>
    /// <param name="target">Address of a string the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::string &target);

    /// <summary>Reads a unicode string from the stream</summary>
    /// <param name="target">Address of a unicode string the value will be read into</param>
    public: NUCLEX_STORAGE_API void Read(std::wstring &target);

    /// <summary>Reads a chunk of bytes from the stream</summary>
    /// <param name="buffer">Buffer the bytes will be read into</param>
    /// <param name="byteCount">Number of bytes that will be read from the stream</param>
    public: NUCLEX_STORAGE_API void Read(void *buffer, std::size_t byteCount);

    // Unhide the overloaded Read() methods in the base class
    // See http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.9
    using XmlReader::Read;

    /// <summary>Stores private implementation details</summary>
    private: class Impl;

    /// <summary>The blob being parsed</summary>
    private: std::shared_ptr<const Blob> blob;
    /// <summary>Implementation details</summary>
    private: std::unique_ptr<Impl> impl;

    /// <summary>Binary data format currently used by the XML reader</summary>
    private: XmlBinaryFormat binaryFormat;
    /// <summary>Value of the attribute the reader has entered</summary>
    private: const std::string *enteredAttribute;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLBLOBREADER_H
