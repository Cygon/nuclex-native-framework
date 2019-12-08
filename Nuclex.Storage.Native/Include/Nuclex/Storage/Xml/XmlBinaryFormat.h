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

#ifndef NUCLEX_STORAGE_XML_XMLBINARYFORMAT_H
#define NUCLEX_STORAGE_XML_XMLBINARYFORMAT_H

#include "Nuclex/Storage/Config.h"

#include <string>
#include <cstdint>

namespace Nuclex { namespace Storage { namespace Xml {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Format in which binary data is stored in an XML document</summary>
  enum class XmlBinaryFormat {

    /// <summary>Binary data is stored in hexadecimal notation</summary>
    BinHex,

    /// <summary>Binary data is encoded to base-64</summary>
    Base64

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Storage::Xml

#endif // NUCLEX_STORAGE_XML_XMLBINARYFORMAT_H
