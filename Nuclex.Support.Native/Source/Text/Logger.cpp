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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Text/Logger.h"

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Implementation of a logger that does not do anything</summary>
  class NullLogger : public Nuclex::Support::Text::Logger {

    /// <summary>Frees all resources owned by the logger</summary>
    public: virtual ~NullLogger() = default;

    /// <summary>Whether the logger is actually doing anything with the log messages</summary>
    /// <returns>True if the log messages are processed in any way, false otherwise</returns>
    /// <remarks>
    ///   Forming the log message strings may be non-trivial and cause memory allocations, too,
    ///   so by checking this method just once, you can skip all logging if they would be
    ///   discarded anyway.
    /// </remarks>
    public: virtual bool IsLogging() const override { return false; }

  };

  // ------------------------------------------------------------------------------------------- //

  NullLogger SharedNullLogger;

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Text {

  // ------------------------------------------------------------------------------------------- //

  Logger &Logger::Null = SharedNullLogger;

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Text
