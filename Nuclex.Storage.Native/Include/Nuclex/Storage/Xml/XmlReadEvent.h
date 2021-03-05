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

#ifndef NUCLEX_STORAGE_XML_XMLNODETYPE_H
#define NUCLEX_STORAGE_XML_XMLNODETYPE_H

#include "Nuclex/Storage/Config.h"

#include <string>
#include <cstdint>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Reads data using the XML format</summary>
  enum class XmlReadEvent {

    /// <summary>The XML reader has encountered the start of an element</summary>
    ElementStart,

    /// <summary>The XML reader has encountered the end of an element</summary>
    ElementEnd,

    /// <summary>The XML reader has encountered contents within an element</summary>
    Content,

    /// <summary>The XML reader has reached the end of the document</summary>
    End

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLNODETYPE_H
