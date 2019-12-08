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

#ifndef NUCLEX_STORAGE_XML_XMLBLOBREADER_IMPL_H
#define NUCLEX_STORAGE_XML_XMLBLOBREADER_IMPL_H

#include "Nuclex/Storage/Xml/XmlBlobReader.h"
#include "ExpatParser.h"

#include <vector>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data using the XML format</summary>
  class XmlBlobReader::Impl {

    /// <summary>Amount of data handed to the parser at once</summary>
    private: static const std::size_t BufferSize;

    /// <summary>Initializes a new XML blob reader implementation</summary>
    /// <param name="blob">Blob of plaintext XML the XML blog reader will parse</param>
    public: Impl(const Blob &blob);

    /// <summary>Destroys the XML blob reader implementation</summary>
    public: ~Impl();

    /// <summary>Reads from XML plaintext up until the next event is encountered</summary>
    /// <returns>The type of event encountered when parsing</returns>
    public: XmlReadEvent Read();

    /// <summary>Retrieves the name of the last element that was entered or exited</summary>
    /// <returns>The name of the last element entered or exited</returns>
    public: const std::string &GetElementName() const { return this->name; }

    /// <summary>Counts the number of attributes in the current element</summary>
    /// <returns>The number of attributes present in the current element</summary>
    public: std::size_t CountAttributes() const { return this->attributes.size(); }

    /// <summary>Retrieves the text in the element currently entered</summary>
    /// <returns>The text in the element currently entered</returns>
    public: const std::string &GetElementText() const { return this->text; }

    /// <summary>Retrieves the value if the attribute with the specified name</summary>
    /// <param name="attributeName">Name of the attribute whose value will be retrieved</param>
    /// <returns>The value of the requested attribute or null if it doesn't exist</returns>
    public: const std::string *GetAttributeValue(const std::string &attributeName) const {
      for(std::size_t index = 0; index < this->attributes.size(); ++index) {
        if(this->attributes[index].first == attributeName) {
          return &this->attributes[index].second;
        }
      }

      return nullptr;
    }

    /// <summary>Retrieves the name of the attribue with the specified index</summary>
    /// <param name="index">Index of the attribue whose name will be retrieved</param>
    /// <returns>The name of the attribue with the specified index</returns>
    public: const std::string &GetAttributeName(std::size_t index) const {
      return this->attributes.at(index).first;
    }

    /// <summary>Reads the next chunk of data from the blob and parses it</summary>
    /// <returns>The status of the XML parser</returns>    
    private: XML_Status parseNextChunk();

    /// <summary>Called when eXpat encounters the start of an element</summary>
    /// <param name="name">Name of the elment whose start eXpat has encountered</param>
    /// <param name="firstAttribute">Start of the array containing the attribute names</param>
    /// <param name="attributeCount">Number of attributes in the array</param>
    private: void elementStartEncountered(
      const char *name,
      const char **firstAttribute, std::size_t attributeCount
    );

    /// <summary>Called when eXpat encounters the end of an element</summary>
    /// <param name="name">Name of the element whose end eXpat has encountered</param>
    private: void elementEndEncountered(const char *name);

    /// <summary>Calback for when eXpat encounters text between nodes</summary>
    /// <param name="text">Text that eXpat has encountered in the XML stream</param>
    /// <param name="length">Lenght of the encountered text</param>
    private: void textEncountered(const char *text, int length);

    /// <summary>Callback for when eXpat encounters the start of an element</summary>
    /// <param name="impl">this pointer of the implementation details instance</param>
    /// <param name="name">Name of the element whose start eXpat encountered</param>
    /// <param name="attributes">
    ///   The element's attribues in a null-terminated array of name/value pairs
    /// </param>
    private: static void elementStartEncountered(
      void *impl, const char *name, const char **attributes
    );

    /// <summary>Callback for when eXpat encounters the end of an element</summary>
    /// <param name="impl">this pointer of the implementation details instance</param>
    /// <param name="name">Name of the element whose end eXpat encountered</param>
    private: static void elementEndEncountered(void *impl, const char *name);

    /// <summary>Calback for when eXpat encounters text between nodes</summary>
    /// <param name="impl">this pointer of the implementation details instance</param>
    /// <param name="text">Text that eXpat has encountered in the XML stream</param>
    /// <param name="length">Lenght of the encountered text</param>
    private: static void textEncountered(void *impl, const char *text, int length);

    /// <summary>Combines a name with a value stored as a string</summary>
    typedef std::pair<std::string, std::string> NameValuePair;

    /// <summary>The expat parser used to walk through the XML elements</summary>
    private: ExpatParser parser;
    /// <summary>Whether the parser has been suspended</summary>
    private: bool isSuspended;

    /// <summary>From which which the parser reads and processes XML plaintext</summary>
    private: const Blob &blob;
    /// <summary>Length of the blob being parsed</summary>
    private: std::uint64_t blobLength;
    /// <summary>Position of the parser within the blob</summary>
    private: std::uint64_t position;

    /// <summary>Lastmost read event that was encountered</summary>
    private: XmlReadEvent lastReadEvent;
    /// <summary>Whether the parser reported an element end after being suspended</summary>
    private: bool elementEndOutstanding;
    /// <summary>Name of the current element</summary>
    private: std::string name;
    /// <summary>Attributes in the current element</summary>
    private: std::vector<NameValuePair> attributes;
    /// <summary>Name of the current element</summary>
    private: std::string text;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLBLOBREADER_IMPL_H
