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

#include "Nuclex/Support/ScopeGuard.h"
#include <gtest/gtest.h>

namespace Nuclex { namespace Support {

  // ------------------------------------------------------------------------------------------- //

  TEST(ScopeGuardTest, OnScopeExitMacroWorks) {
    bool wasCleanedUp = false;
    {
      ON_SCOPE_EXIT { wasCleanedUp = true; };

      EXPECT_FALSE(wasCleanedUp);
    }

    EXPECT_TRUE(wasCleanedUp);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ScopeGuardTest, OnScopeExitTransactionMacroWorks) {
    bool wasCleanedUp = false;
    {
      auto cleanUp = ON_SCOPE_EXIT_TRANSACTION { wasCleanedUp = true; };

      EXPECT_FALSE(wasCleanedUp);
    }

    EXPECT_TRUE(wasCleanedUp);
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(ScopeGuardTest, OnScopeExitTransactionCanBeCommitted) {
    bool wasCleanedUp = false;
    {
      auto cleanUp = ON_SCOPE_EXIT_TRANSACTION { wasCleanedUp = true; };

      EXPECT_FALSE(wasCleanedUp);
      cleanUp.Commit();
    }

    EXPECT_FALSE(wasCleanedUp);
  }

  // ------------------------------------------------------------------------------------------- //

}} // namespace Nuclex::Support
