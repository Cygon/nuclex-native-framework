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

#ifndef NUCLEX_STORAGE_XML_XMLWRITER_H
#define NUCLEX_STORAGE_XML_XMLWRITER_H

#include "Nuclex/Storage/Config.h"
#include "Nuclex/Storage/Writer.h"
#include "Nuclex/Storage/Xml/XmlBinaryFormat.h"

#include <string>
#include <cstdint>
#include <memory>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Writes data using the XML format</summary>
  /// <remarks>
  ///   <para>
  ///     This interface extends the <see cref="BinaryWriter" /> with features for controlling
  ///     the generation of XML elements, attributes and comments. It is suitable both for
  ///     populating an XML DOM tree and for generating an XML output stream without seeking.
  ///   </para>
  ///   <example>
  ///     <code>
  ///       XmlBlobWriter blobWriter(std::make_shared&lt;MemoryBlob&gt;());
  ///
  ///       XmlWriter &writer = blobWriter;
  ///
  ///       writer.WriteDeclaration();
  ///       { XmlWriter::ElementScope(writer, "Scene");
  ///         writer.SetAttributeValue("Culling", "Grid");
  ///
  ///         writer.WriteComment("Something to populate the scene with");
  ///         { XmlWriter::ElementScope(writer, "Entity");
  ///           writer.Write(123.456f);
  ///         }
  ///       }
  ///     </code>
  ///   </example>
  /// </remarks>
  class XmlWriter : public Writer {

    #pragma region class AttributeScope

    /// <summary>Automatically enters an attribute for the lifetime of the scope</summary>
    public: class AttributeScope {

      /// <summary>Initializes a new attribute scope using the specified XML writer</summary>
      /// <param name="writer">XML writer the attribute scope will use</param>
      /// <param name="attributeName">Name of the attribute that will be written</param>
      public: NUCLEX_STORAGE_API AttributeScope(XmlWriter &writer, const std::string &attributeName) :
        writer(writer) {
        this->writer.BeginAttribute(attributeName);
      }

      /// <summary>Initializes a new attribute scope using the specified XML writer</summary>
      /// <param name="writer">XML writer the attribute scope will use</param>
      /// <param name="attributeName">Name of the attribute that will be written</param>
      public: NUCLEX_STORAGE_API AttributeScope(
        const std::shared_ptr<XmlWriter> &writer, const std::string &attributeName
      ) :
        writer(*writer.get()) {
        this->writer.BeginAttribute(attributeName);
      }

      /// <summary>Destroys the attribute scope and ends writing the attribute</summary>
      public: NUCLEX_STORAGE_API ~AttributeScope() {
        this->writer.EndAttribute();
      }

      private: AttributeScope(const AttributeScope &);
      private: AttributeScope &operator =(const AttributeScope &);

      /// <summary>XML writer in which the scope will create an attribute</summary>
      private: XmlWriter &writer;

    };

    #pragma endregion // class AttributeScope

    #pragma region class ElementScope

    /// <summary>Automatically enters an element for the lifetime of the scope</summary>
    public: class ElementScope {

      /// <summary>Initializes a new element scope using the specified XML writer</summary>
      /// <param name="writer">XML writer the element scope will use</param>
      /// <param name="elementName">Name of the element that will be written</param>
      public: NUCLEX_STORAGE_API ElementScope(XmlWriter &writer, const std::string &elementName) :
        writer(writer) {
        this->writer.BeginElement(elementName);
      }

      /// <summary>Initializes a new element scope using the specified XML writer</summary>
      /// <param name="writer">XML writer the element scope will use</param>
      /// <param name="elementName">Name of the element that will be written</param>
      public: NUCLEX_STORAGE_API ElementScope(
        const std::shared_ptr<XmlWriter> &writer, const std::string &elementName
      ) :
        writer(*writer.get()) {
        this->writer.BeginElement(elementName);
      }

      /// <summary>Destroys the element scope and ends writing the element</summary>
      public: NUCLEX_STORAGE_API ~ElementScope() {
        this->writer.EndElement();
      }

      private: ElementScope(const ElementScope &);
      private: ElementScope &operator =(const ElementScope &);

      /// <summary>XML writer in which the scope will create an element</summary>
      private: XmlWriter &writer;

    };

    #pragma endregion // class ElementScope

    #pragma region class ElementScope

    /// <summary>Automatically enters a comment for the lifetime of the scope</summary>
    public: class CommentScope {

      /// <summary>Initializes a new comment scope using the specified XML writer</summary>
      /// <param name="writer">XML writer the comment scope will use</param>
      public: NUCLEX_STORAGE_API CommentScope(XmlWriter &writer) :
        writer(writer) {
        this->writer.BeginComment();
      }

      /// <summary>Initializes a new comment scope using the specified XML writer</summary>
      /// <param name="writer">XML writer the comment scope will use</param>
      public: NUCLEX_STORAGE_API CommentScope(const std::shared_ptr<XmlWriter> &writer) :
        writer(*writer.get()) {
        this->writer.BeginComment();
      }

      /// <summary>Destroys the comment scope and ends writing the comment</summary>
      public: NUCLEX_STORAGE_API ~CommentScope() {
        this->writer.EndComment();
      }

      private: CommentScope(const CommentScope &);
      private: CommentScope &operator =(const CommentScope &);

      /// <summary>XML writer in which the scope will create a comment</summary>
      private: XmlWriter &writer;

    };

    #pragma endregion // class CommentScope

    /// <summary>Destroys the XML writer</summary>
    public: NUCLEX_STORAGE_API virtual ~XmlWriter() {}

    /// <summary>Retrieves the currently selected binary data format</summary>
    /// <returns>The format in which binary data will be written</returns>
    public: virtual XmlBinaryFormat GetBinaryFormat() const = 0;

    /// <summary>Selects the binary data format to use for writing binary data</summary>
    /// <param name="binaryFormat">Format in which binary data will be written</param>
    public: virtual void SetBinaryFormat(XmlBinaryFormat binaryFormat) = 0;

    /// <summary>
    ///   Writes the XML declaration, containing the version of the XML standard and
    ///   the encoding used
    /// </summary>
    public: virtual void WriteDeclaration(const std::string &encoding = "utf-8") = 0;

    /// <summary>Opens an XML element</summary>
    /// <param name="elementName">Name of the XML element that will be opened</param>
    /// <remarks>
    ///   All values written after this will end up inside the XML element. Consider using
    ///   the ElementScope helper to ensure there's no mismatch between Open/Close calls and
    ///   to simplify your code.
    /// </remarks>
    public: virtual void BeginElement(const std::string &elementName) = 0;

    /// <summary>Closes the current XML element</summary>
    public: virtual void EndElement() = 0;

    /// <summary>Begins an XML comment</summary>
    /// <remarks>
    ///   All values written after this will end up as individual lines inside the XML
    ///   comment. Consider using the CommentScope helper to ensure there's no mismatch
    ///   between Open/Close calls and to simplify your code.
    /// </remarks>
    public: virtual void BeginComment() = 0;

    /// <summary>Ends the current XML comment</summary>
    public: virtual void EndComment() = 0;

    /// <summary>Writes a comment into the XML plaintext</summary>
    /// <param name="comment">Comment that will be written</param>
    public: NUCLEX_STORAGE_API void WriteComment(const std::string &comment) {
      CommentScope scope(*this);
      Write(comment);
    }

    /// <summary>Opens an XML attribute in the current element</summary>
    /// <param name="attributeName">Name of the attribute that will be opened</param>
    /// <remarks>
    ///   All values written after this will end up inside the XML attribute. It is unusual
    ///   to store more than one value in an XML attribute and separate attributes should
    ///   be given preference. Consider using the AttributeScope helper to ensure there's
    ///   no mismatch between Open/Close calls and to simplify your code.
    /// </remarks>
    public: virtual void BeginAttribute(const std::string &attributeName) = 0;

    /// <summary>Closes the current XML attribute</summary>
    public: virtual void EndAttribute() = 0;

    /// <summary>Assigns the value of an attribute in the current element</summary>
    /// <param name="attributeName">Name of the attribue whose value will be assigned</param>
    /// <param name="value">Value that will be assigned to the attribute</param>
    public: template <typename TValue> void SetAttributeValue(
      const std::string &attributeName, const TValue &value
    ) {
      AttributeScope scope(*this, attributeName);
      Write(value);
    }

    // Unhide the overloaded Read() methods in the base class
    // See http://www.parashift.com/c++-faq-lite/strange-inheritance.html#faq-23.9
    using Writer::Write;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLWRITER_H
