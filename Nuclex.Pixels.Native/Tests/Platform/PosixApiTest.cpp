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
#define NUCLEX_PIXELS_SOURCE 1

#include "../../Source/Platform/PosixApi.h"
#if !defined(NUCLEX_PIXELS_PLATFORM_POSIXAPI_H)
#error Your compiler went bonkers and violated the standard search rules for headers.
#endif

#include <gtest/gtest.h>

#if !defined(NUCLEX_PIXELS_WINDOWS)

namespace Nuclex { namespace Pixels { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(PosixApiTest, CanGetErrorMessage) {
    int errorNumber = EACCES;
    std::string errorMessage = PosixApi::GetErrorMessage(errorNumber);
    EXPECT_GT(errorMessage.length(), 10U); // We can expect 10 letters at least, eh?
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Platform

#endif // !defined(NUCLEX_PIXELS_WINDOWS)
