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
#define NUCLEX_SUPPORT_SOURCE 1

#include "../../Source/Platform/WindowsRegistryApi.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include <gtest/gtest.h>

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsRegistryApiTest, CanGetHiveFromStringInShortForm) {
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"hkcr", 4));
    EXPECT_EQ(HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"HKCR", 4));

    EXPECT_EQ(HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"hkcc", 4));
    EXPECT_EQ(HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"HKCC", 4));

    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"hkcu", 4));
    EXPECT_EQ(HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"HKCU", 4));

    EXPECT_EQ(HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"hklm", 4));
    EXPECT_EQ(HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"HKLM", 4));

    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"hku", 3));
    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"HKU", 3));
  }

  // ------------------------------------------------------------------------------------------- //

  TEST(WindowsRegistryApiTest, CanGetHiveFromStringInLongForm) {
    EXPECT_EQ(
      HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"hkey_classes_root", 17)
    );
    EXPECT_EQ(
      HKEY_CLASSES_ROOT, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CLASSES_ROOT", 17)
    );

    EXPECT_EQ(
      HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"hkey_current_config", 19)
    );
    EXPECT_EQ(
      HKEY_CURRENT_CONFIG, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CURRENT_CONFIG", 19)
    );

    EXPECT_EQ(
      HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"hkey_current_user", 17)
    );
    EXPECT_EQ(
      HKEY_CURRENT_USER, WindowsRegistryApi::GetHiveFromString(u8"HKEY_CURRENT_USER", 17)
    );

    EXPECT_EQ(
      HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"hkey_local_machine", 18)
    );
    EXPECT_EQ(
      HKEY_LOCAL_MACHINE, WindowsRegistryApi::GetHiveFromString(u8"HKEY_LOCAL_MACHINE", 18)
    );

    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"hkey_users", 10));
    EXPECT_EQ(HKEY_USERS, WindowsRegistryApi::GetHiveFromString(u8"HKEY_USERS", 10));
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)