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

#ifndef NUCLEX_STORAGE_XML_XMLBLOBWRITER_H
#define NUCLEX_STORAGE_XML_XMLBLOBWRITER_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Xml/XmlWriter.h"

#include <vector>
#include <stack>

namespace Nuclex { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  class Blob;

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Storage

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes data in the XML format</summary>
  class XmlBlobWriter : public XmlWriter {
  
    /// <summary>Initializes a new XML writer writing into a blob</summary>
    /// <param name="blob">Blob the XML writer will write into</param>
    public: NUCLEX_STORAGE_API XmlBlobWriter(const std::shared_ptr<Blob> &blob);
    /// <summary>Destroys the XML reader</summary>
    public: NUCLEX_STORAGE_API ~XmlBlobWriter();

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

    /// <summary>
    ///   Writes the XML declaration, containing the version of the XML standard and
    ///   the encoding used
    /// </summary>
    public: NUCLEX_STORAGE_API void WriteDeclaration(const std::string &encoding = "utf-8");

    /// <summary>Opens an XML element</summary>
    /// <param name="elementName">Name of the XML element that will be opened</param>
    /// <remarks>
    ///   All values written after this will end up inside the XML element. Consider using
    ///   the ElementScope helper to ensure there's no mismatch between Open/Close calls and
    ///   to simplify your code.
    /// </remarks>
    public: NUCLEX_STORAGE_API void BeginElement(const std::string &elementName);

    /// <summary>Closes the current XML element</summary>
    public: NUCLEX_STORAGE_API void EndElement();

    /// <summary>Begins an XML comment</summary>
    /// <remarks>
    ///   All values written after this will end up as individual lines inside the XML
    ///   comment. Consider using the CommentScope helper to ensure there's no mismatch
    ///   between Open/Close calls and to simplify your code.
    /// </remarks>
    public: NUCLEX_STORAGE_API void BeginComment();

    /// <summary>Ends the current XML comment</summary>
    public: NUCLEX_STORAGE_API void EndComment();

    /// <summary>Opens an XML attribute in the current element</summary>
    /// <param name="attributeName">Name of the attribute that will be opened</param>
    /// <remarks>
    ///   All values written after this will end up inside the XML attribute. It is unusual
    ///   to store more than one value in an XML attribute and separate attributes should
    ///   be given preference. Consider using the AttributeScope helper to ensure there's
    ///   no mismatch between Open/Close calls and to simplify your code.
    /// </remarks>
    public: NUCLEX_STORAGE_API void BeginAttribute(const std::string &attributeName);

    /// <summary>Closes the current XML attribute</summary>
    public: NUCLEX_STORAGE_API void EndAttribute();

    /// <summary>Writes a boolean into the stream</summary>
    /// <param name="value">Boolean that will be written</param>
    public: NUCLEX_STORAGE_API void Write(bool value);

    /// <summary>Writes an unsigned 8 bit integer into the stream</summary>
    /// <param name="value">8 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint8_t value);

    /// <summary>Writes a signed 8 bit integer into the stream</summary>
    /// <param name="value">8 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int8_t value);

    /// <summary>Writes an unsigned 16 bit integer into the stream</summary>
    /// <param name="value">16 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint16_t value);

    /// <summary>Writes a signed 16 bit integer into the stream</summary>
    /// <param name="value">16 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int16_t value);

    /// <summary>Writes an unsigned 32 bit integer into the stream</summary>
    /// <param name="value">32 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint32_t value);

    /// <summary>Writes a signed 32 bit integer into the stream</summary>
    /// <param name="value">32 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int32_t value);

    /// <summary>Writes an unsigned 64 bit integer into the stream</summary>
    /// <param name="value">64 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::uint64_t value);

    /// <summary>Writes a signed 64 bit integer into the stream</summary>
    /// <param name="value">64 bit integer that will be written</param>
    public: NUCLEX_STORAGE_API void Write(std::int64_t value);

    /// <summary>Writes a floating point value into the stream</summary>
    /// <param name="value">Floating point value that will be written</param>
    public: NUCLEX_STORAGE_API void Write(float value);

    /// <summary>Writes a double precision floating point value into the stream</summary>
    /// <param name="value">Double precision floating point value that will be written</param>
    public: NUCLEX_STORAGE_API void Write(double value);

    /// <summary>Writes a string into the stream</summary>
    /// <param name="value">String that will be written</param>
    public: NUCLEX_STORAGE_API void Write(const std::string &value);

    /// <summary>Writes a unicode string into the stream</summary>
    /// <param name="value">Unicode string that will be written</param>
    public: NUCLEX_STORAGE_API void Write(const std::wstring &value);

    /// <summary>Writes a chunk of bytes into the stream</summary>
    /// <param name="buffer">Buffer the bytes that will be written</param>
    /// <param name="byteCount">Number of bytes to write</param>
    public: NUCLEX_STORAGE_API void Write(const void *buffer, std::size_t byteCount);

    // Unhide the overloaded Read() methods in the base class
    // See http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.9
    using XmlWriter::Write;

    /// <summary>Applies a line break to the element the writer is currently in</summary>
    private: void lineBreakOuterElement();

    /// <summary>Closes the element the writer is currently in</summary>
    private: void closeElement();

    /// <summary>Appends a comment to the current element</summary>
    /// <param name="comment">Comment that will be appended</param>
    private: void writeComment(const std::string &comment);

    /// <summary>Appends content to the current element</summary>
    /// <param name="text">Content that will be appended</param>
    private: void writeData(const std::string &text);

    /// <summary>Stores private implementation details</summary>
    private: class Impl;

    #pragma region enum DeferredToken

    /// <summary>Which kind of token the XML blob writer is still waiting to write</summary>
    public: enum class DeferredToken {

      /// <summary>Nothing is waiting to be written</summary>
      None,

      /// <summary>An element opening is waiting to be written</summary>
      /// <remarks>
      ///   <para>
      ///     In this case, the element name has been pushed onto our <see cref="elementNames" />
      ///     stack, but the element opening itself wasn't written at all yet because we don't
      ///     know yet whether it will be an empty element, a single-line data element or
      ///     a multi-line element with children.
      ///   <para>
      ///   <para>
      ///     Buffer contents: whitespace up to the element start. We already flushed
      ///     the outer element's line because only back then did we know whether this is
      ///     its first child (increase indentation) or the nth (keep indentation).
      ///   </para>
      /// </remarks>
      ElementOpening,

      /// <summary>An element's single-line data is waiting to be written</summary>
      /// <remarks>
      ///   <para>
      ///     We've written the element opening, but are waiting on the data in
      ///     the <see cref="content" /> field. Currently the data seems like it would
      ///     allow of a single-line element, but more data or children might still be
      ///     added to the element.
      ///   </para>
      ///   <para>
      ///     Buffer contents: The element opening tag without <see cref="content" />.
      ///     If the element is closed, the content and closing tag should be appended,
      ///     otherwise a line break and indentation increase should follow.
      ///   </para>
      /// </remarks>
      ElementOpeningWithContent,

      /// <summary>We're in an element after its first child has been written</summary>
      /// <remarks>
      ///   <para>
      ///     After something has been written that didn't allow for a single-line element
      ///     (a comment, content that's too long for one line or a child element), we're
      ///     in this state that allows other children to be appended or the element to be
      ///     closed regularly with the closing tag on its own line.
      ///   </para>
      ///   <para>
      ///     Buffer contents: the last line of whatever content the element is carrying.
      ///     If more content follows, an indentation-keeping flush should be used,
      ///     otherwise indentation should be decreased and the closing tag written.
      ///   </para>
      /// </remarks>
      ElementChildren,

      /// <summary>A comment opening is waiting to be written</summary>
      /// <remarks>
      ///   <para>
      ///      A comment was started, but it isn't clear yet whether this will be an empty
      ///      comment, a single-line comment or a multi-line comment.
      ///   </para>
      ///   <para>
      ///     Buffer contents: whitespace up to the comment start. We already flushed
      ///     the owning element's line because only back then did we know whether this is
      ///     its first child (increase indentation) or the nth (keep indentation).
      ///   </para>
      /// </remarks>
      CommentOpening,

      /// <summary>A comment's single-line text is waiting to be written</summary>
      /// <remarks>
      ///   <para>
      ///     We've written the comment opening, but are waiting on the text in
      ///     the <see cref="content" /> field. Currently the text seems like it would
      ///     allow of a single-line comment, but more text might be added to the comment.
      ///   </para>
      ///   <para>
      ///     Buffer contents: The comment opening tag without <see cref="content" />.
      ///     If the comment is closed, the text and comment closing should be appended,
      ///     otherwise a line break and indentation increase should follow.
      ///   </para>
      /// </remarks>
      CommentOpeningWithText,

      /// <summary>We're in a comment after its first line has been written</summary>
      /// <remarks>
      ///   <para>
      ///     The comment was turned into a multi-line comment and we've written everything
      ///     up to the final line, because we don't know what if the comment will end or
      ///     if more text will be written into it.
      ///   </para>
      ///   <para>
      ///     Buffer contents: the last line of the text the comment is carrying.
      ///     If more text follows, an indentation-keeping flush should be used,
      ///     otherwise indentation should be decreased and the comment closing written.
      ///   </para>
      /// </remarks>
      CommentText

    };

    #pragma endregion // enum DeferredToken

    /// <summary>Blob the XML writer is writing to</summary>
    private: std::shared_ptr<Blob> blob;
    /// <summary>Implementation details</summary>
    private: std::unique_ptr<Impl> impl;
    /// <summary>Format in which binary data will be written</summary>
    private: XmlBinaryFormat binaryFormat;

    /// <summary>Names of currently opened XML elements</summary>
    private: std::stack<std::string> elementNames;
    /// <summary>Token that still needs to be written</summary>
    private: DeferredToken deferredToken;
    /// <summary>Whether the XML writer is currently writing an attribute</summary>
    private: bool isInAttribute;
    /// <summary>Whether the XML writer is currently writing a comment</summary>
    private: bool isInComment;

    /// <summary>Comment or element content being written</summary>
    private: std::string content;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLBLOBWRITER_H
