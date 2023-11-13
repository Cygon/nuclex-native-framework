#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2023 Nuclex Development Labs

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

#ifndef NUCLEX_SUPPORT_PLATFORM_WINDOWSREGISTRYAPI_H
#define NUCLEX_SUPPORT_PLATFORM_WINDOWSREGISTRYAPI_H

#include "Nuclex/Support/Config.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "WindowsApi.h"

#include <string> // for std::string
#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Platform {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Wraps the API used to interface with the registry on Windows systems</summary>
  class WindowsRegistryApi {

    /// <summary>Returns the registry hive matching its string name</summary>
    /// <param name="hiveName">Name of the registry hive whose key will be returned</param>
    /// <returns>The registry hive with the specified hive name</returns>
    /// <remarks>
    ///   This supports both the short form (HKCU/, HKLM/) and the long form
    ///   (HKEY_CURRENT_USER/, HKEY_LOCAL_MACHINE/) for specifying the hive.
    /// </remarks>
    public: static ::HKEY GetHiveFromString(
      const std::string &hiveName, std::string::size_type hiveNameLength
    );

    /// <summary>
    ///   Builds a list of the names of all registry keys directly below the key with
    ///   the specified handle
    /// </summary>
    /// <param name="keyHandle">Handle of the key whose direct children will be queried</param>
    /// <returns>A list containing the names of all child keys</returns>
    public: static std::vector<std::string> WindowsRegistryApi::GetAllSubKeyNames(
      ::HKEY keyHandle
    );

    /// <summary>
    ///   Builds a list of the names of all value directly below the key with
    ///   the specified handle
    /// </summary>
    /// <param name="keyHandle">Handle of the key whose values will be queried</param>
    /// <returns>A list containing the names of all value below the key</returns>
    public: static std::vector<std::string> WindowsRegistryApi::GetAllValueNames(
      ::HKEY keyHandle
    );

    /// <summary>Opens a subkey below the specified parent registry key</summary>
    /// <param name="parentKeyHandle">Handle of the parent registry key</param>
    /// <param name="subKeyName">Name of the subkey that will be opened</param>
    /// <param name="writable">Whether the key will be opened with write permissions</param>
    /// <returns>
    ///   The handle of the opened registry subkey or a null pointer if the key doesn't exist
    /// </returns>
    public: static ::HKEY OpenExistingSubKey(
      ::HKEY parentKeyHandle, const std::string &subKeyName, bool writable = true
    );

    /// <summary>Opens or creates a subkey below the specified parent registry key</summary>
    /// <param name="parentKeyHandle">Handle of the parent registry key</param>
    /// <param name="subKeyName">Name of the subkey that will be opened or created</param>
    /// <returns>The handle of the opened or created registry subkey</returns>
    public: static ::HKEY OpenOrCreateSubKey(
      ::HKEY parentKeyHandle, const std::string &subKeyName
    );

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Platform

#endif // defined(NUCLEX_SUPPORT_WINDOWS)

#endif // NUCLEX_SUPPORT_PLATFORM_WINDOWSREGISTRYAPI_H
