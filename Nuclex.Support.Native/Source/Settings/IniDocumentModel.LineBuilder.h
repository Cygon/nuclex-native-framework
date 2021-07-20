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

#ifndef NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_LINEBUILDER_H
#define NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_LINEBUILDER_H

#include "Nuclex/Support/Config.h"

#include "IniDocumentModel.h"

// #include <utility> // for std::pair (already present via std::unordered_map)

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  class IniDocumentModel::LineBuilder {

    /// <summary>Calculates the number of bytes required to store the specified string</summary>
    /// <param name="text">String that will be checked</param>
    /// <returns>
    ///   The number of bytes needed to store the string with quotes (if needed) and
    ///   escaped characters. The second member of the pair states whether quotes must
    ///   be placed around the string in storage.
    /// </returns>
    public: static std::pair<std::string::size_type, bool> GetRequiredByteCount(
      const std::string &text
    );



  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_INIDOCUMENTMODEL_FILEPARSER_H
