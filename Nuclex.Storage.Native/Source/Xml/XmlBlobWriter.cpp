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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_STORAGE_SOURCE 1

#include "Nuclex/Storage/Xml/XmlBlobWriter.h"
#include "Nuclex/Storage/Blob.h"
#include "XmlBlobWriter.Impl.h"

#include "Nuclex/Support/Text/StringConverter.h"
#include "Nuclex/Support/Text/LexicalCast.h"

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  XmlBlobWriter::XmlBlobWriter(const std::shared_ptr<Blob> &blob) :
    binaryFormat(XmlBinaryFormat::Base64),
    blob(blob),
    impl(new Impl(*blob.get())),
    deferredToken(DeferredToken::None),
    isInAttribute(false),
    isInComment(false) {}

  // ------------------------------------------------------------------------------------------- //

  XmlBlobWriter::~XmlBlobWriter() {
    this->impl->FlushAndKeepIndentation();
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::WriteDeclaration(const std::string &encoding /* = "utf-8" */) {
    if(this->elementNames.size() > 0) {
      throw std::runtime_error("XML declaration must be the first element that is written");
    }

    this->impl->AppendDeclaration(encoding);

    this->impl->FlushAndKeepIndentation();
    this->impl->FlushAndKeepIndentation();
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::BeginElement(const std::string &elementName) {
    if(this->isInAttribute || this->isInComment) {
      throw std::logic_error("Cannot start an element inside a comment or attribute");
    }

    lineBreakOuterElement();

    // Record the element, but don't write it yet. We don't know if it will become
    // and empty element or contain content. The user is also free to add attributes.
    this->elementNames.push(elementName);
    this->deferredToken = DeferredToken::ElementOpening;
    this->impl->ClearAttributes();
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::EndElement() {
    if(this->isInAttribute || this->isInComment) {
      throw std::logic_error("Cannot end an element inside a comment or attribute");
    }

    switch(this->deferredToken) {

      // The closed element didn't have its opening written yet, so it is empty and
      // can be turned into a self-closing element.
      case DeferredToken::ElementOpening: {
        this->impl->AppendElement(this->elementNames.top());

        break;
      }

      // The closed element was a candidate for single-line content. Since we now know
      // nothing more is going to be added to it, turn it into a single-line content element
      case DeferredToken::ElementOpeningWithContent: {
        this->impl->Append(this->content);
        this->content.clear();
        this->impl->AppendElementClosing(this->elementNames.top());

        break;
      }

      // The buffer is still containing the last line of the element's contents.
      // Now we know the element is being closed, so decrease the indentation and write
      // the closing element.
      case DeferredToken::ElementChildren: {
        this->impl->FlushAndDecreaseIndentation();
        this->impl->AppendElementClosing(this->elementNames.top());

        break;
      }

    }

    this->elementNames.pop();
    this->deferredToken = DeferredToken::ElementChildren;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::BeginComment() {
    this->isInComment = true;

    lineBreakOuterElement();

    this->impl->AppendCommentOpening();
    this->deferredToken = DeferredToken::CommentOpening;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::EndComment() {
    this->isInComment = false;

    switch(this->deferredToken) {

      // The closed element didn't have its opening written yet, so it is empty and
      // can be turned into a self-closing element.
      case DeferredToken::CommentOpening: {
        this->impl->AppendCommentClosing();

        break;
      }

      // The closed element was a candidate for single-line content. Since we now know
      // nothing more is going to be added to it, turn it into a single-line content element
      case DeferredToken::CommentOpeningWithText: {
        this->impl->Append(this->content);
        this->content.clear();
        this->impl->AppendCommentClosing();

        break;
      }

      // The buffer is still containing the last line of the element's contents.
      // Now we know the element is being closed, so decrease the indentation and write
      // the closing element.
      case DeferredToken::CommentText: {
        this->impl->FlushAndDecreaseIndentation();
        this->impl->AppendCommentClosing();

        break;
      }

    }

    this->deferredToken = DeferredToken::ElementChildren;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::BeginAttribute(const std::string &attributeName) {
    this->impl->AddAttribute(attributeName);
    this->isInAttribute = true;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::EndAttribute() {
    this->isInAttribute = false;
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(bool value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::uint8_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::int8_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::uint16_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::int16_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::uint32_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::int32_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::uint64_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(std::int64_t value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(float value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(double value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::lexical_cast<std::string>(value));
    } else {
      writeData(Nuclex::Support::Text::lexical_cast<std::string>(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(const std::string &value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(value);
    } else if(this->isInComment) {
      writeComment(value);
    } else {
      writeData(value);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(const std::wstring &value) {
    if(this->isInAttribute) {
      this->impl->SetAttributeValue(
        Nuclex::Support::Text::StringConverter::Utf8FromWide(value)
      );
    } else if(this->isInComment) {
      writeComment(Nuclex::Support::Text::StringConverter::Utf8FromWide(value));
    } else {
      writeData(Nuclex::Support::Text::StringConverter::Utf8FromWide(value));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::Write(const void *buffer, std::size_t byteCount) {
    throw std::runtime_error("Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::lineBreakOuterElement() {
    switch(this->deferredToken) {

      // There's an element opening queued that we haven't written yet. Now we know
      // that it's going to have content (us), so write an opening tag
      case DeferredToken::ElementOpening: {
        this->impl->AppendElementOpening(this->elementNames.top());
        this->impl->FlushAndIncreaseIndentation();

        break;
      }

      // There's an element opening queued with single-line content that we haven't
      // written yet. Now we know it needs to be split into multiple lines!
      case DeferredToken::ElementOpeningWithContent: {
        this->impl->FlushAndIncreaseIndentation();

        this->impl->Append(this->content);
        this->content.clear();
        this->impl->FlushAndKeepIndentation();

        break;
      }

      // We're just one of many children of the outer element. Open the new element
      // in a new line
      case DeferredToken::ElementChildren: {
        this->impl->FlushAndKeepIndentation();

        break;
      }

    }
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::closeElement() {
  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::writeComment(const std::string &comment) {
    std::size_t startIndex = comment.find_first_not_of(Impl::Whitespace);
    std::size_t endIndex = comment.find_last_not_of(Impl::Whitespace);

    std::size_t length;
    if(endIndex >= startIndex) {
      length = endIndex - startIndex + 1;
    } else {
      length = 0;
    }
    
    switch(this->deferredToken) {

      // This won't be called unless we're in a comment, so all element cases
      // do not need to be handled here!

      case DeferredToken::CommentOpening: {
        if(this->impl->IsCommentShort(length)) {
          if(endIndex >= startIndex) {
            this->content.assign(comment, startIndex, length);
          } else {
            this->content.clear();
          }
          this->deferredToken = DeferredToken::CommentOpeningWithText;
        } else {
          this->impl->FlushAndIncreaseIndentation();
          this->impl->AppendText(comment);
          this->deferredToken = DeferredToken::CommentText;
        }

        break;
      }

      case DeferredToken::CommentOpeningWithText: {
        this->impl->FlushAndIncreaseIndentation();
        this->impl->Append(this->content);
        this->content.clear();

        this->impl->FlushAndKeepIndentation();
        this->impl->AppendText(comment);
        this->deferredToken = DeferredToken::CommentText;

        break;
      }

      case DeferredToken::CommentText: {
        this->impl->FlushAndKeepIndentation();
        this->impl->AppendText(comment);

        break;
      }

      default: {
        throw std::logic_error(
          "Implementation error: XmlBlobWriter::writeComment() called outside of a comment"
        );
      }

    }

  }

  // ------------------------------------------------------------------------------------------- //

  void XmlBlobWriter::writeData(const std::string &text) {
    std::size_t startIndex = text.find_first_not_of(Impl::Whitespace);
    std::size_t endIndex = text.find_last_not_of(Impl::Whitespace);

    std::size_t length;
    if(endIndex >= startIndex) {
      length = endIndex - startIndex + 1;
    } else {
      length = 0;
    }

    switch(this->deferredToken) {

      case DeferredToken::ElementOpening: {
        this->impl->AppendElementOpening(this->elementNames.top());

        if((length == 0) || this->impl->IsElementShort(this->elementNames.top(), length)) {
          if(endIndex >= startIndex) {
            this->content.assign(text, startIndex, length);
          } else {
            this->content.clear();
          }
          this->deferredToken = DeferredToken::ElementOpeningWithContent;
        } else {
          this->impl->FlushAndIncreaseIndentation();
          this->impl->AppendText(text);
          this->deferredToken = DeferredToken::ElementChildren;
        }

        break;
      }

      case DeferredToken::ElementOpeningWithContent: {
        this->impl->FlushAndIncreaseIndentation();
        this->impl->Append(this->content);
        this->content.clear();

        this->impl->FlushAndKeepIndentation();
        this->impl->AppendText(text);
        this->deferredToken = DeferredToken::ElementChildren;

        break;
      }

      case DeferredToken::ElementChildren: {
        this->impl->FlushAndKeepIndentation();
        this->impl->AppendText(text);

        break;
      }

    }
    
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml
