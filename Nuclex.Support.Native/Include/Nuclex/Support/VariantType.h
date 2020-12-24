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

#ifndef NUCLEX_SUPPORT_VARIANTTYPE_H
#define NUCLEX_SUPPORT_VARIANTTYPE_H

#include "Nuclex/Support/Config.h"

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Different types that can be stored in a variant</summary>
  enum class VariantType {

    /// <summary>Nothing is being stored</summary>
    Empty = 0,
    /// <summary>A C++ boolean is being stored</summary>
    Boolean = 1,
    /// <summary>An unsigned 8 bit integer is being stored</summary>
    Uint8 = 2,
    /// <summary>A signed 8 bit integer is being stored</summary>
    Int8 = 3,
    /// <summary>An unsigned 16 bit integer is being stored</summary>
    Uint16 = 4,
    /// <summary>A signed 16 bit integer is being stored</summary>
    Int16 = 5,
    /// <summary>An unsigned 32 bit integer is being stored</summary>
    Uint32 = 6,
    /// <summary>A signed 32 bit integer is being stored</summary>
    Int32 = 7,
    /// <summary>An unsigned 64 bit integer is being stored</summary>
    Uint64 = 8,
    /// <summary>A signed 64 bit integer is being stored</summary>
    Int64 = 9,
    /// <summary>A floating point value is being stored</summary>
    Float = 10,
    /// <summary>A double precision floating point value is being stored</summary>
    Double = 11,
    /// <summary>A string is being stored</summary>
    String = 12,
    /// <summary>A wide character string is being stored</summary>
    WString = 13,
    /// <summary>An arbitrary type is being stored</summary>
    Any = 14,
    /// <summary>A pointer to void is being stored</summary>
    VoidPointer = 15

  };

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support

#endif // NUCLEX_SUPPORT_VARIANTTYPE_H
