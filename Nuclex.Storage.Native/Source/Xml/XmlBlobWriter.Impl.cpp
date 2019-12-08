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
#define NUCLEX_STORAGE_SOURCE 1

#include "XmlBlobWriter.Impl.h"

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  const std::string XmlBlobWriter::Impl::Whitespace = std::string(" \t\r\n");

  // ------------------------------------------------------------------------------------------- //

  XmlBlobWriter::Impl::Impl(Blob &blob) :
    blob(blob),
    location(0),
    attributesLength(0),
    indentationLevel(0) {}

  // ------------------------------------------------------------------------------------------- //

  XmlBlobWriter::Impl::~Impl() {}

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendDeclaration(const std::string &encoding /* = "utf8" */) {
    const static std::string declarationStart("<?xml version=\"1.0\" encoding=\"");
    const static std::string declarationEnd("\" ?>");

    Append(declarationStart);
    Append(encoding);
    Append(declarationEnd);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendElementOpening(const std::string &name) {
    this->buffer.push_back('<');
    this->buffer.insert(this->buffer.end(), name.begin(), name.end());

    std::size_t length = this->indentationLevel + 1 + name.length() + attributesLength + 1;
    if(this->attributes.empty() || (length < TargetColumns)) {

      // Append all attributes after the opener
      for(std::size_t index = 0; index < this->attributes.size(); ++index) {
        AppendAttribute(this->attributes[index].first, this->attributes[index].second);
      }

    } else { // Line with attributes too long, split into multiple lines

      // Begin with a line break so we can line up all attributes below the tag
      FlushAndIncreaseIndentation();

      // Append all attributes, each on its own line. This branch is only entered
      // if there is at least one attribute, so the (unsigned type - 1) is safe.
      std::size_t lastIndex = this->attributes.size() - 1;
      for(std::size_t index = 0; index < this->attributes.size(); ++index) {
        AppendAttribute(this->attributes[index].first, this->attributes[index].second);
        if(index == lastIndex) {
          FlushAndDecreaseIndentation();
        } else {
          FlushAndKeepIndentation();
        }
      }

    }

    this->buffer.push_back('>');
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendElementClosing(const std::string &name) {
    this->buffer.push_back('<');
    this->buffer.push_back('/');
    this->buffer.insert(this->buffer.end(), name.begin(), name.end());
    this->buffer.push_back('>');
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendElement(const std::string &name) {
    this->buffer.push_back('<');
    Append(name);

    std::size_t length = this->indentationLevel + 1 + name.length() + attributesLength + 3;
    if(this->attributes.empty() || (length < TargetColumns)) {

      // Append all attributes after the opener
      for(std::size_t index = 0; index < this->attributes.size(); ++index) {
        AppendAttribute(this->attributes[index].first, this->attributes[index].second);
      }

      this->buffer.push_back(' ');

    } else { // Line with attributes too long, split into multiple lines

      // Begin with a line break so we can line up all attributes below the tag
      FlushAndIncreaseIndentation();

      // Append all attributes, each on its own line. This branch is only entered
      // if there is at least one attribute, so the (unsigned type - 1) is safe.
      std::size_t lastIndex = this->attributes.size() - 1;
      for(std::size_t index = 0; index < this->attributes.size(); ++index) {
        AppendAttribute(this->attributes[index].first, this->attributes[index].second);
        if(index == lastIndex) {
          FlushAndDecreaseIndentation();
        } else {
          FlushAndKeepIndentation();
        }
      }

    }

    this->buffer.push_back('/');
    this->buffer.push_back('>');

    this->attributes.clear();
    this->attributesLength = 0;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendCommentOpening() {
    const static std::string CommentOpeningSquence("<!-- ");

    Append(CommentOpeningSquence);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendCommentClosing() {
    const static std::string CommentClosingSequence(" -->");

    Append(CommentClosingSequence);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendComment(const std::string &comment) {
    const static std::string CommentOpeningSquence("<!-- ");
    const static std::string CommentClosingSequence(" -->");

    Append(CommentOpeningSquence);
    Append(comment);
    Append(CommentClosingSequence);
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Impl::AppendText(const std::string &text) {
    std::size_t targetColumns = TargetColumns - this->indentationLevel - 1;

    std::size_t startIndex = text.find_first_not_of(Impl::Whitespace);
    while(text.length() - startIndex >= targetColumns) {
      std::size_t nextStartIndex;

      // Find the last whitespace character still inside the target column width
      std::size_t lastIndex = text.find_last_of(
        Impl::Whitespace, startIndex + targetColumns
      );

      // None found? Take the first one we can find after the target column width
      if(lastIndex == std::string::npos) {
        lastIndex = text.find_first_of(Impl::Whitespace, startIndex + targetColumns);

        // Still nothing? Take the whole line!
        if(lastIndex == std::string::npos) {
          lastIndex = text.length();
          nextStartIndex = lastIndex;
        } else {
          nextStartIndex = lastIndex + 1;
        }
      } else { // Go back to the first whitespace character if there are multiple
        nextStartIndex = lastIndex + 1;
        lastIndex = text.find_last_not_of(Impl::Whitespace, lastIndex - 1);
      }

      Append(text.substr(startIndex, lastIndex - startIndex + 1));
      
      // Skip any whitespace characters after the line break, if there are multiple
      startIndex = text.find_first_not_of(Impl::Whitespace, nextStartIndex);
      if(startIndex == std::string::npos) {
        break;
      }

      FlushAndKeepIndentation();
    }

    // If the string fits exactly or ends in lots of whitespace, we might end up with
    // no short line at the end.
    if(startIndex == std::string::npos) {
      // No characters left
    } else { // Generate a line from the remaining text
      std::size_t lastIndex = text.find_last_not_of(Impl::Whitespace);
      Append(text.substr(startIndex, lastIndex - startIndex + 1));
    }
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml
