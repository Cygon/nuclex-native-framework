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

#include "ExpatParser.h"
#include "Nuclex/Storage/Xml/XmlParseError.h"

#include <limits>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  ExpatParser::ExpatParser(const std::string &charset) :
    parser(::XML_ParserCreate(charset.c_str()), &::XML_ParserFree),
    errorCode(XML_ERROR_NONE) {
    if(this->parser.get() == nullptr) {
      throw std::runtime_error("Could not initialize eXpat XML parser");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::string ExpatParser::BuildErrorDescription() const {
    std::string errorDescription;
    {
      const char *errorMessage = ::XML_ErrorString(this->errorCode);
      errorDescription.append(errorMessage);
    }
    errorDescription.append(" at line ");
    errorDescription.append(std::to_string(this->errorLineNumber));
    errorDescription.append(", column ");
    errorDescription.append(std::to_string(this->errorColumnIndex));

    return errorDescription;
  }

  // ------------------------------------------------------------------------------------------- //

  void ExpatParser::ThrowIfErrorRecorded() const {
    if(ErrorEncountered()) {
      std::size_t lineNumber = static_cast<std::size_t>(this->errorLineNumber);
      std::size_t columnIndex = static_cast<std::size_t>(this->errorColumnIndex);
      throw XmlParseError(BuildErrorDescription(), lineNumber, columnIndex);
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void *ExpatParser::GetBuffer(std::size_t length) {
    const std::size_t intMax = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if(length > intMax) {
      throw std::logic_error("Requested buffer size too large");
    }

    return ::XML_GetBuffer(this->parser.get(), static_cast<int>(length));
  }

  // ------------------------------------------------------------------------------------------- //

  XML_Status ExpatParser::ParseBuffer(std::size_t length, bool isFinal) {
    const std::size_t intMax = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if(length > intMax) {
      throw std::logic_error("Amount of data to parse is too large");
    }

    XML_Status status = ::XML_ParseBuffer(
      this->parser.get(), static_cast<int>(length), isFinal ? XML_TRUE : XML_FALSE
    );
    if(status == XML_STATUS_ERROR) {
      recordError();
    }

    return status;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml
