#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2020 Nuclex Development Labs

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

#ifndef NUCLEX_STORAGE_XML_XMLBLOBWRITER_IMPL_H
#define NUCLEX_STORAGE_XML_XMLBLOBWRITER_IMPL_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Xml/XmlBlobWriter.h"
#include "Nuclex/Storage/Blob.h"

#include <stdexcept> // for std::runtime_error

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes data in the XML format</summary>
  /// <remarks>
  ///   <para>
  ///     Convention: All Append...() methods leave their last (or only) line open.
  ///     This allows the caller to decide whether to continue appending on the same
  ///     line or whether to follow up with a line break and indentation increase to
  ///     split the current element into multiple lines.
  ///   </para>
  /// </remarks>
  class XmlBlobWriter::Impl {

    /// <summary>Number of space characters by which to indent</summary>
    public: static const std::size_t IndentationWidth = 2;

    /// <summary>Target width in which to keep the XML document</summary>
    public: static const std::size_t TargetColumns = 100;

    /// <summary>Whitespace characters</summary>
    public: static const std::string Whitespace;

    /// <summary>Initializes a new XML writer writing into a blob</summary>
    /// <param name="blob">Blob the XML writer will write into</param>
    public: Impl(Blob &blob);
    /// <summary>Destroys the XML reader</summary>
    public: ~Impl();

    /// <summary>Retrieves the current indentation level</summary>
    /// <returns>The current indentation level written text begins at</returns>
    public: std::size_t GetIndentationLevel() const { return this->indentationLevel; }

    /// <summary>Checks whether an XML element with content fits on a single line</summary>
    /// <param name="elementName">Name of the element carrying the content</param>
    /// <param name="contentLength">Length of the content the XML element will carry</param>
    /// <returns>True if the XML element with content fits in a single single</returns>
    public: bool IsElementShort(
      const std::string &elementName, const std::size_t contentLength
    ) {
      std::size_t length = this->indentationLevel;
      length += 1 + elementName.length() + 1;
      length += contentLength;
      length += 2 + elementName.length() + 1;

      return (length < TargetColumns);
    }

    /// <summary>Checks whether an XML element with content fits on a single line</summary>
    /// <param name="commentLength">Length of the comment that will be checked</param>
    /// <returns>True if the comments fits in a single single</returns>
    public: bool IsCommentShort(const std::size_t commentLength) {
      std::size_t length = this->indentationLevel;
      length += 5 + commentLength + 4;

      return (length < TargetColumns);
    }

    /// <summary>Adds an attribute to the currently opened element</summary>
    /// <param name="name">Name of the attribue that will be added</param>
    public: void AddAttribute(const std::string &name) {
      this->attributes.push_back(NameValuePair(name, std::string()));
      this->attributesLength += 1 + name.length() + 3;
    }

    /// <summary>Sets the value of the most recently added attribute</summary>
    /// <param name="value">Value the most recently added attribute will have</param>
    public: void SetAttributeValue(const std::string &value) {
      NameValuePair &lastAttribute = this->attributes.back();
      this->attributesLength -= lastAttribute.second.length();
      lastAttribute.second = value;
      this->attributesLength += lastAttribute.second.length();
    }

    /// <summary>Resets the attribue list</summary>
    public: void ClearAttributes() {
      this->attributes.clear();
      this->attributesLength = 0;
    }

    /// <summary>Appends the XML version and encoding declaration to the buffer</summary>
    /// <param name="encoding">Encoding the XML document will use</param>
    public: void AppendDeclaration(const std::string &encoding = "utf-8");

    /// <summary>Appends an opening XML element to the buffer</summary>
    /// <param name="name">Name of the opening element that will be appended</param>
    public: void AppendElementOpening(const std::string &name);

    /// <summary>Appends a closing XML element to the buffer</summary>
    /// <param name="name">Name of the Closing element that will be appended</param>
    public: void AppendElementClosing(const std::string &name);

    /// <summary>Appends an empty XML element to the buffer</summary>
    /// <param name="name">Name of the empty element that will be appended</param>
    public: void AppendElement(const std::string &name);

    /// <summary>Appends the opening sequence for a comment</summary>
    public: void AppendCommentOpening();

    /// <summary>Appends the closing sequence for a comment</summary>
    public: void AppendCommentClosing();

    /// <summary>Appends a single-line comment</summary>
    /// <param name="comment">Comment that will be appended</param>
    public: void AppendComment(const std::string &comment);

    /// <summary>Appends text to the current XML element or comment</summary>
    /// <param name="text">Text that will be appended to the XML element or comment</param>
    public: void AppendText(const std::string &text);

    /// <summary>Appends an attribute to the buffer</summary>
    /// <param name="name">Name of the attribute that will be appended</param>
    /// <param name="value">Value of the attribue that will be appended</param>
    public: void AppendAttribute(const std::string &name, const std::string &value) {
      this->buffer.push_back(' ');
      this->buffer.insert(this->buffer.end(), name.begin(), name.end());
      this->buffer.push_back('=');
      this->buffer.push_back('"');
      this->buffer.insert(this->buffer.end(), value.begin(), value.end());
      this->buffer.push_back('"');
    }

    /// <summary>Appends the specified string to the writer's line buffer</summary>
    /// <param name="text">Text that will be appended to the line buffer</param>
    public: void Append(const std::string &text) {
      this->buffer.insert(this->buffer.end(), text.begin(), text.end());
    }

    /// <summary>Flushes the line buffer into the blob</summary>
    public: void FlushAndKeepIndentation() {
      appendReturnAndFlush();

      this->buffer.resize(this->indentationLevel);
    }

    /// <summary>
    ///   Flushes the line buffer into the blob and increases the indentation level
    /// </summary>
    public: void FlushAndIncreaseIndentation() {
      appendReturnAndFlush();

      std::size_t oldIndentationLevel = this->indentationLevel;
      this->indentationLevel += IndentationWidth;

      this->buffer.resize(this->indentationLevel);
      for(std::size_t index = oldIndentationLevel; index < this->indentationLevel; ++index) {
        this->buffer[index] = ' ';
      }
    }

    /// <summary>
    ///   Flushes the line buffer into the blob and decreases the indentation level
    /// </summary>
    public: void FlushAndDecreaseIndentation() {
      appendReturnAndFlush();

      if(this->indentationLevel == 0) {
        throw std::runtime_error("Tried to decrease indentation beyond zero");
      }

      this->indentationLevel -= IndentationWidth;
      this->buffer.resize(this->indentationLevel);
    }

    /// <summary>Appends a line break and flushes the line without deleting it</summary>
    /// <remarks>
    ///   For internal use by the other flush methods. This methods doesn't clear
    ///   the buffer and repeatedly calling it would append multiple line breaks.
    /// </remarks>
    private: void appendReturnAndFlush() {
      std::size_t bufferSize = this->buffer.size();
      if(bufferSize == this->indentationLevel) {

        // If the buffer is empty, write a blank line instead of an indented blank line
        // because trailing space characters a f-ugly.
        this->blob.WriteAt(this->location, "\n", 1);
        ++this->location;

      } else {

        // Append a line break to increase the XML file's readability
        this->buffer.push_back('\n');
        ++bufferSize;

        // Write the contents of the buffer with the line break into the blob
        this->blob.WriteAt(this->location, &this->buffer[0], this->buffer.size());
        this->location += bufferSize;

      }
    }

    private: Impl(const Impl &impl);
    private: Impl &operator =(const Impl &impl);

    /// <summary>Combines a name with a value stored as a string</summary>
    typedef std::pair<std::string, std::string> NameValuePair;

    /// <summary>Blob the XML writer is writing to</summary>
    private: Blob &blob;
    /// <summary>Location of the writer within the blob</summary>
    private: std::uint64_t location;

    /// <summary>Attributes in the current XML element</summary>
    private: std::vector<NameValuePair> attributes;
    /// <summary>Total length of the attributes in the unwritten XML element</summary>
    private: std::size_t attributesLength;

    /// <summary>Buffer in which lines are prepared before writing them</summary>
    private: std::vector<char> buffer;
    /// <summary>Current indentation level of the XML writer</summary>
    private: std::size_t indentationLevel;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLBLOBWRITER_IMPL_H
