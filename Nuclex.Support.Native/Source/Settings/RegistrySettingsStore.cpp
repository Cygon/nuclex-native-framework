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

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_SUPPORT_SOURCE 1

#include "Nuclex/Support/Settings/RegistrySettingsStore.h"

#if defined(NUCLEX_SUPPORT_WINDOWS)

#include "Nuclex/Support/ScopeGuard.h"
#include "Nuclex/Support/Text/StringMatcher.h"
#include "Nuclex/Support/Text/StringConverter.h"
#include "Nuclex/Support/Text/LexicalCast.h"

#include "../Platform/WindowsRegistryApi.h"

#include <cassert> // for assert()
#include <stdexcept> // for std::logic_error
#include <memory> // for std::unique_ptr

// TODO: This file has become too long.
//       Split the registry read/write methods into an API wrapper

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Looks for the next forward or backward slash in a string</summary>
  /// <param name="path">Path in which the next forward or backward slash is searched</param>
  /// <param nam=e"startIndex">Index at which the search will start</param>
  /// <returns>
  ///   The index of the next forward or backward slash. If no slashes were found,
  ///   std::string::npos is returned.
  /// </returns>
  std::string::size_type findNextSlash(
    const std::string &path, std::string::size_type startIndex = 0
  ) {
    std::string::size_type length = path.length();
    for(std::string::size_type index = startIndex; index < length; ++index) {
      char current = path[index];
      if((current == '\\') || (current == '/')) {
        return index;
      }
    }

    return std::string::npos;
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Changes all slashes in a UTF-8 string to backward slashes</summary>
  /// <param name="stringToChange">String in which the slashes will be changed</param>
  void makeAllSlashesBackward(std::string &stringToChange) {
    std::string::size_type length = stringToChange.length();
    for(std::string::size_type index = 0; index < length; ++index) {
      if(stringToChange[index] == '/') {
        stringToChange[index] = '\\';
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interprets a registry value as the type requested by the caller</summary>
  /// <typeparam name="TValue">Type as which the value will be interpreted</typeparam>
  /// <param name="valueBytes">Buffer that contains the value</param>
  /// <param name="valueSize">Size of the value in bytes</param>
  /// <param name="valueType">Type of the value</param>
  /// <returns>The value interpreted as the requested type</returns>
  template<typename TValue>
  std::optional<TValue> interpretValue(
    const BYTE *valueBytes, std::size_t valueSize, ::DWORD valueType
  ) {
    // Due to the specializations on the other types, we know TValue is either int32 or int64
    switch(valueType) {
      case REG_DWORD: {
        if constexpr(std::is_signed<TValue>::value) {
          return static_cast<TValue>(*reinterpret_cast<const std::int32_t *>(valueBytes));
        } else {
          return static_cast<TValue>(*reinterpret_cast<const std::uint32_t *>(valueBytes));
        }
      }
      case REG_QWORD: {
        if constexpr(std::is_signed<TValue>::value) {
          return static_cast<TValue>(*reinterpret_cast<const std::int64_t *>(valueBytes));
        } else {
          return static_cast<TValue>(*reinterpret_cast<const std::uint64_t *>(valueBytes));
        }
      }
      case REG_SZ: {
        return Nuclex::Support::Text::lexical_cast<TValue>(
          std::string(reinterpret_cast<const char *>(valueBytes), valueSize)
        );
      }
      default: {
        throw std::runtime_error(u8"Read registry value had a type we don't support");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interprets a registry value as a boolean</summary>
  /// <param name="valueBytes">Buffer that contains the value</param>
  /// <param name="valueSize">Size of the value in bytes</param>
  /// <param name="valueType">Type of the value</param>
  /// <returns>The value interpreted as a boolean</returns>
  template<>
  std::optional<bool> interpretValue(
    const BYTE *valueBytes, std::size_t valueSize, ::DWORD valueType
  ) {
    switch(valueType) {
      case REG_DWORD: {
        return (*reinterpret_cast<const std::uint32_t *>(valueBytes) != 0);
      }
      case REG_QWORD: {
        return (*reinterpret_cast<const std::uint64_t *>(valueBytes) != 0);
      }
      case REG_SZ: {
        return Nuclex::Support::Text::lexical_cast<bool>(
          std::string(reinterpret_cast<const char *>(valueBytes), valueSize)
        );
      }
      default: {
        throw std::runtime_error(u8"Read registry value had a type we don't support");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Interprets a registry value as a string</summary>
  /// <param name="valueBytes">Buffer that contains the value</param>
  /// <param name="valueSize">Size of the value in bytes</param>
  /// <param name="valueType">Type of the value</param>
  /// <returns>The value interpreted as a string</returns>
  template<>
  std::optional<std::string> interpretValue(
    const BYTE *valueBytes, std::size_t valueSize, ::DWORD valueType
  ) {
    switch(valueType) {
      case REG_DWORD: {
        return Nuclex::Support::Text::lexical_cast<std::string>(
          *reinterpret_cast<const std::uint32_t *>(valueBytes)
        );
      }
      case REG_QWORD: {
        return Nuclex::Support::Text::lexical_cast<std::string>(
          *reinterpret_cast<const std::uint64_t *>(valueBytes)
        );
      }
      case REG_SZ: {
        std::wstring valueUtf16(
          reinterpret_cast<const std::wstring::value_type *>(valueBytes),
          (valueSize - 1) / sizeof(std::wstring::value_type)
        );
        return Nuclex::Support::Text::StringConverter::Utf8FromWide(valueUtf16);
      }
      default: {
        throw std::runtime_error(u8"Read registry value had a type we don't support");
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves a value stored under a registry key</summary>
  /// <typeparam name="TValue">Type of value that will be read</typeparam>
  /// <param name="keyHandle">Handle of the registry key in which the value is stored</param>
  /// <param name="valueName">Name of the value in the registry</param>
  /// <param name="sizeHint">Expected size of the value, useful for strings</param>
  /// <returns>The registry value, if it was present</returns>
  template<typename TValue>
  std::optional<TValue> queryValue(
    ::HKEY keyHandle, const std::string &valueName, std::size_t sizeHint = 16
  ) {
    std::wstring utf16 = Nuclex::Support::Text::StringConverter::WideFromUtf8(valueName);

    ::DWORD valueType;
    ::DWORD valueSize = static_cast<::DWORD>(sizeHint);

    // First, try to use a stack-allocated memory buffer
    if(sizeHint < 17) {
      ::BYTE stackValue[16];
      ::LSTATUS result = ::RegQueryValueExW(
        keyHandle,
        utf16.c_str(), // UTF-16 value name
        nullptr,
        &valueType,
        stackValue,
        &valueSize
      );
      if(result == ERROR_FILE_NOT_FOUND) {
        return std::optional<TValue>();
      } else if(result == ERROR_SUCCESS) {
        return interpretValue<TValue>(stackValue, valueSize, valueType);
      } else if(result != ERROR_MORE_DATA) {
        Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
          u8"Could not query value stored in registry key", result
        );
      }
    }

    // Either the caller hinted at a learger size or the value turned out to be larger
    // Try a heap-allocated buffer with the hinted or known size.
    for(;;) {
      std::unique_ptr<std::uint8_t[]> heapValue(new std::uint8_t[valueSize]);
      ::LSTATUS result = ::RegQueryValueExW(
        keyHandle,
        utf16.c_str(), // UTF-16 value name
        nullptr,
        &valueType,
        heapValue.get(),
        &valueSize
      );
      if(result == ERROR_FILE_NOT_FOUND) {
        return std::optional<TValue>();
      } else if(result == ERROR_SUCCESS) {
        return interpretValue<TValue>(heapValue.get(), valueSize, valueType);
      }

      // If this was our first attempt, retry with the now known size.
      // Otherwise, we treat even ERROR_MORE_DATA as an error
      if(sizeHint >= 17) {
        if(result == ERROR_MORE_DATA) {
          sizeHint = 0;
          continue;
        }
      }

      // This point is reached if the ::RegQueryValueExW() method call called two times
      // already, the second time with a bfufer using its self-provided value size.
      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(
        u8"Could not query value stored in registry key", result
      );
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Retrieves a value stored under a registry key</summary>
  /// <typeparam name="TValue">Type of value that will be read</typeparam>
  /// <param name="settingsKeyHandle">Handle of the settings root key</param>
  /// <param name="categoryName">Name of the category in which the value is stored</param>
  /// <param name="properyName">Name of the value in the registry</param>
  /// <returns>The registry value, if it was present</returns>
  template<typename TValue>
  std::optional<TValue> retrieveValue(
    ::HKEY settingsKeyHandle, const std::string &categoryName,
    const std::string &propertyName
  ) {
    if(categoryName.empty()) {
      return queryValue<TValue>(settingsKeyHandle, propertyName);
    } else {
      ::HKEY subKeyHandle = Nuclex::Support::Platform::WindowsRegistryApi::OpenExistingSubKey(
        settingsKeyHandle, categoryName
      );
      if(subKeyHandle == ::HKEY(nullptr)) {
        return std::optional<TValue>();
      } else {
        ON_SCOPE_EXIT{
          ::LRESULT result = ::RegCloseKey(subKeyHandle);
          NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
          assert((result == ERROR_SUCCESS) && u8"Registry subkey is closed successfully");
        };
        return queryValue<TValue>(subKeyHandle, propertyName);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets a registry value under the specified parent key</summary>
  /// <typeparam name="TValue">Type of value that will be stored in the registry</typeparam>
  /// <param name="parentKeyHandle">Key under which the value will be stored</param>
  /// <param name="propertNameUtf16">UTF-16 encoded name of the value</param>
  /// <param name="propertyValue">Value that will be stored</param>
  /// <returns>The result returned from the <see cref="RegSetValueExW" /> method</returns>
  template<typename TValue>
  ::LSTATUS setValue(
    ::HKEY parentKeyHandle,
    const std::wstring &propertyNameUtf16, const TValue &propertyValue
  ) {
    ::DWORD valueSize;
    ::DWORD valueType;
    if constexpr(sizeof(TValue) == sizeof(std::uint64_t)) {
      valueSize = 8;
      valueType = REG_QWORD;
    } else {
      valueSize = 4;
      valueType = REG_DWORD;
    }

    return ::RegSetValueExW(
      parentKeyHandle,
      propertyNameUtf16.c_str(),
      0, // reserved
      valueType,
      reinterpret_cast<const BYTE *>(&propertyValue),
      valueSize
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets a registry value under the specified parent key to a boolean</summary>
  /// <typeparam name="TValue">Type of value that will be stored in the registry</typeparam>
  /// <param name="parentKeyHandle">Key under which the value will be stored</param>
  /// <param name="propertNameUtf16">UTF-16 encoded name of the value</param>
  /// <param name="propertyValue">Boolean that will be stored</param>
  /// <returns>The result returned from the <see cref="RegSetValueExW" /> method</returns>
  template<>
  ::LSTATUS setValue(
    ::HKEY parentKeyHandle,
    const std::wstring &propertyNameUtf16, const bool &propertyValue
  ) {
    ::DWORD value = (propertyValue ? 1 : 0);
    return ::RegSetValueExW(
      parentKeyHandle,
      propertyNameUtf16.c_str(),
      0, // reserved
      REG_DWORD,
      reinterpret_cast<const BYTE *>(&value),
      4
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Sets a registry value under the specified parent key to a string</summary>
  /// <typeparam name="TValue">Type of value that will be stored in the registry</typeparam>
  /// <param name="parentKeyHandle">Key under which the value will be stored</param>
  /// <param name="propertNameUtf16">UTF-16 encoded name of the value</param>
  /// <param name="propertyValue">String that will be stored</param>
  /// <returns>The result returned from the <see cref="RegSetValueExW" /> method</returns>
  template<>
  ::LSTATUS setValue(
    ::HKEY parentKeyHandle,
    const std::wstring &propertyNameUtf16, const std::string &propertyValue
  ) {
    std::wstring propertyValueUtf16 = (
      Nuclex::Support::Text::StringConverter::WideFromUtf8(propertyValue)
    );
    ::DWORD valueSize = static_cast<::DWORD>(
      (propertyValueUtf16.length() + 1) * sizeof(std::wstring::value_type)
    );
    return ::RegSetValueExW(
      parentKeyHandle,
      propertyNameUtf16.c_str(),
      0, // reserved
      REG_SZ,
      reinterpret_cast<const BYTE *>(propertyValueUtf16.c_str()),
      valueSize
    );
  }

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores a value stored in a registry key</summary>
  /// <typeparam name="TValue">Type of value that will be stores</typeparam>
  /// <param name="settingsKeyHandle">Handle of the settings root key</param>
  /// <param name="categoryName">Name of the category in which the value will be stored</param>
  /// <param name="propertyName">Name of the value in the registry</param>
  /// <param name="propertyValue">Value that will be stored in the registry</param>
  /// <returns>The registry value, if it was present</returns>
  template<typename TValue>
  void storeValue(
    ::HKEY settingsKeyHandle, const std::string &categoryName,
    const std::string &propertyName, const TValue &propertyValue
  ) {
    ::LSTATUS result;
    {
      std::wstring propertyNameUtf16 = Nuclex::Support::Text::StringConverter::WideFromUtf8(
        propertyName
      );
      if(categoryName.empty()) {
        result = setValue<TValue>(settingsKeyHandle, propertyNameUtf16, propertyValue);
      } else {
        ::HKEY subKeyHandle = Nuclex::Support::Platform::WindowsRegistryApi::OpenOrCreateSubKey(
          settingsKeyHandle, categoryName
        );
        ON_SCOPE_EXIT{
          ::LRESULT result = ::RegCloseKey(subKeyHandle);
          NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
          assert((result == ERROR_SUCCESS) && u8"Registry subkey is closed successfully");
        };
        result = setValue<TValue>(subKeyHandle, propertyNameUtf16, propertyValue);
      }
    } // propertyName scope

    if(unlikely(result != ERROR_SUCCESS)) {
      std::string message(u8"Could not store setting '", 35);
      message.append(propertyName);
      message.append(u8"' in registry", 13);
      Nuclex::Support::Platform::WindowsApi::ThrowExceptionForSystemError(message, result);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  bool RegistrySettingsStore::DeleteKey(const std::string &registryPath) {
    std::string::size_type firstSlashIndex = findNextSlash(registryPath);
    if(firstSlashIndex == std::string::npos) {
      std::string message(u8"Refusing to delete '", 20);
      message.append(registryPath);
      message.append(u8"' because it does not contain a path to a subkey", 48);
      throw std::invalid_argument(message);
    } else { // Slashes present, separate the registry hive from the rest
      ::HKEY hiveKeyHandle = Platform::WindowsRegistryApi::GetHiveFromString(
        registryPath, firstSlashIndex
      );

      // We need a string containing the name of the subkey to open, also using only
      // backward slashes (unlike Windows' file system API, the registry API isn't lax)
      std::wstring subKeyNameUtf16;
      {
        std::string subkeyName = registryPath.substr(firstSlashIndex + 1);
        makeAllSlashesBackward(subkeyName);
        subKeyNameUtf16 = Text::StringConverter::WideFromUtf8(subkeyName);
      }

      ::LSTATUS result = ::RegDeleteTreeW(hiveKeyHandle, subKeyNameUtf16.c_str());
      if(result == ERROR_FILE_NOT_FOUND) {
        return false;
      } else if(unlikely(result != ERROR_SUCCESS)) {
        std::string message(u8"Could not delete registry tree at '", 35);
        message.append(registryPath);
        message.append(u8"'", 1);
        Platform::WindowsApi::ThrowExceptionForSystemError(message, result);
      }
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  RegistrySettingsStore::RegistrySettingsStore(
    const std::string &registryPath, bool writable /* = true */
  ) : settingsKeyHandle(0) {

    // If no slashes are in the path, it may still be a valid registry hive...
    std::string::size_type firstSlashIndex = findNextSlash(registryPath);
    if(firstSlashIndex == std::string::npos) {
      ::HKEY hiveKeyHandle = Platform::WindowsRegistryApi::GetHiveFromString(
        registryPath, registryPath.length()
      );
      *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle) = (
        Platform::WindowsRegistryApi::OpenExistingSubKey(
          hiveKeyHandle, std::string(), writable
        )
      );
    } else { // Slashes present, separate the registry hive from the rest
      ::HKEY hiveKeyHandle = Platform::WindowsRegistryApi::GetHiveFromString(
        registryPath, firstSlashIndex
      );

      // We need a string containing the name of the subkey to open, also using only
      // backward slashes (unlike Windows' file system API, the registry API isn't lax)
      std::string subkeyName = registryPath.substr(firstSlashIndex + 1);
      makeAllSlashesBackward(subkeyName);

      // If the key doesn't exist, we do one of two things:
      //
      // - in read-only mode, we act as if an empty key existed. This is consistent with
      //   the behavior of the Retrieve() method and allows applications to start without
      //   their registry keys present, using default settings.
      //
      // - in writable mode, we try to create a new key. This is the correct behavior here
      //   because the caller expects us to hold onto any settings written to the registry
      //   and we can't very well do that if we don't have a registry to write to.
      //
      if(writable) {
        *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle) = (
          Platform::WindowsRegistryApi::OpenOrCreateSubKey(hiveKeyHandle, subkeyName)
        );
      } else {
        *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle) = (
          Platform::WindowsRegistryApi::OpenExistingSubKey(hiveKeyHandle, subkeyName, false)
        );
      } // if open writable
    } // if slashes present / not present
  }

  // ------------------------------------------------------------------------------------------- //

  RegistrySettingsStore::~RegistrySettingsStore() {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle != nullptr) {
      ::LSTATUS result = ::RegCloseKey(thisSettingsKeyHandle);
      NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
      assert((result == ERROR_SUCCESS) && u8"Accessed registry key was closed successfully");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RegistrySettingsStore::GetAllCategories() const {
    if(this->settingsKeyHandle == 0) {
      return std::vector<std::string>(); // Non-existent key accessed in read-only mode
    }

    return Platform::WindowsRegistryApi::GetAllSubKeyNames(
      *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle)
    );
  }

  // ------------------------------------------------------------------------------------------- //

  std::vector<std::string> RegistrySettingsStore::GetAllProperties(
    const std::string &categoryName /* = std::string() */
  ) const {
    if(this->settingsKeyHandle == 0) {
      return std::vector<std::string>(); // Non-existent key accessed in read-only mode
    }

    if(categoryName.empty()) {
      return Platform::WindowsRegistryApi::GetAllValueNames(
        *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle)
      );
    } else {
      ::HKEY subKeyHandle = Nuclex::Support::Platform::WindowsRegistryApi::OpenExistingSubKey(
        *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle), categoryName
      );
      if(subKeyHandle == ::HKEY(nullptr)) {
        return std::vector<std::string>(); // Non-existent key accessed in read-only mode
      } else {
        ON_SCOPE_EXIT{
          ::LRESULT result = ::RegCloseKey(subKeyHandle);
          NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
          assert((result == ERROR_SUCCESS) && u8"Registry subkey is closed successfully");
        };
        return Platform::WindowsRegistryApi::GetAllValueNames(subKeyHandle);
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool RegistrySettingsStore::DeleteCategory(const std::string &categoryName) {
    if(this->settingsKeyHandle == 0) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    if(categoryName.empty()) {
      std::vector<std::string> valueNames = Platform::WindowsRegistryApi::GetAllValueNames(
        *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle)
      );
      if(valueNames.empty()) {
        return false;
      }
      for(std::size_t index = 0; index < valueNames.size(); ++index) {
        std::wstring valueNameUtf16 = Text::StringConverter::WideFromUtf8(valueNames[index]);
        ::LSTATUS result = ::RegDeleteValueW(
          *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
          valueNameUtf16.c_str()
        );
        if(unlikely(result != ERROR_SUCCESS)) {
          std::string message(u8"Could not delete value '", 24);
          message.append(categoryName);
          message.append(u8"' from settings key in registry", 31);
          Platform::WindowsApi::ThrowExceptionForSystemError(message, result);
        }
      }
    } else {
      std::wstring subKeyNameUtf16 = Text::StringConverter::WideFromUtf8(categoryName);

      ::LSTATUS result = ::RegDeleteTreeW(
        *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
        subKeyNameUtf16.c_str()
      );
      if(result == ERROR_FILE_NOT_FOUND) {
        return false;
      } else if(unlikely(result != ERROR_SUCCESS)) {
        std::string message(u8"Could not delete subtree '", 26);
        message.append(categoryName);
        message.append(u8"' from settings key in registry", 31);
        Platform::WindowsApi::ThrowExceptionForSystemError(message, result);
      }
    }

    return true;
  }

  // ------------------------------------------------------------------------------------------- //

  bool RegistrySettingsStore::DeleteProperty(
    const std::string &categoryName, const std::string &propertyName
  ) {
    if(this->settingsKeyHandle == 0) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    ::LSTATUS result;
    {
      std::wstring propertyNameUtf16 = Text::StringConverter::WideFromUtf8(propertyName);
      if(categoryName.empty()) {
        result = ::RegDeleteValueW(
          *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle),
          propertyNameUtf16.c_str()
        );
      } else {
        ::HKEY subKeyHandle = Nuclex::Support::Platform::WindowsRegistryApi::OpenExistingSubKey(
          *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle), categoryName
        );
        if(subKeyHandle == ::HKEY(nullptr)) {
          return false;
        } else {
          ON_SCOPE_EXIT{
            ::LRESULT result = ::RegCloseKey(subKeyHandle);
            NUCLEX_SUPPORT_NDEBUG_UNUSED(result);
            assert((result == ERROR_SUCCESS) && u8"Registry subkey is closed successfully");
          };
          result = ::RegDeleteValueW(subKeyHandle, propertyNameUtf16.c_str());
        }
      }
    } // propertyName scope

    if(result == ERROR_FILE_NOT_FOUND) {
      return false;
    } else if(unlikely(result != ERROR_SUCCESS)) {
      std::string message(u8"Could not delete settings value '", 33);
      message.append(propertyName);
      message.append(u8"' from registry", 15);
      Platform::WindowsApi::ThrowExceptionForSystemError(message, result);
    } else {
      return true;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<bool> RegistrySettingsStore::RetrieveBooleanProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<bool>();
    }

    return retrieveValue<bool>(thisSettingsKeyHandle, categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint32_t> RegistrySettingsStore::RetrieveUInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<std::uint32_t>();
    }

    return retrieveValue<std::uint32_t>(thisSettingsKeyHandle, categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int32_t> RegistrySettingsStore::RetrieveInt32Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<std::int32_t>();
    }

    return retrieveValue<std::int32_t>(thisSettingsKeyHandle, categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::uint64_t> RegistrySettingsStore::RetrieveUInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<std::uint64_t>();
    }

    return retrieveValue<std::uint64_t>(thisSettingsKeyHandle, categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::int64_t> RegistrySettingsStore::RetrieveInt64Property(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<std::int64_t>();
    }

    return retrieveValue<std::int64_t>(thisSettingsKeyHandle, categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  std::optional<std::string> RegistrySettingsStore::RetrieveStringProperty(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      return std::optional<std::string>();
    }

    return retrieveValue<std::string>(thisSettingsKeyHandle, categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreBooleanProperty(
    const std::string &categoryName, const std::string &propertyName, bool value
  ) {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    storeValue<bool>(thisSettingsKeyHandle, categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreUInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::uint32_t value
  ) {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    storeValue<std::int32_t>(thisSettingsKeyHandle, categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreInt32Property(
    const std::string &categoryName, const std::string &propertyName, std::int32_t value
  ) {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    storeValue<std::int32_t>(thisSettingsKeyHandle, categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreUInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::uint64_t value
  ) {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    storeValue<std::uint64_t>(thisSettingsKeyHandle, categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreInt64Property(
    const std::string &categoryName, const std::string &propertyName, std::int64_t value
  ) {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    storeValue<std::int64_t>(thisSettingsKeyHandle, categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  void RegistrySettingsStore::StoreStringProperty(
    const std::string &categoryName, const std::string &propertyName, const std::string &value
  ) {
    ::HKEY thisSettingsKeyHandle = *reinterpret_cast<const ::HKEY *>(&this->settingsKeyHandle);
    if(thisSettingsKeyHandle == nullptr) {
      throw std::logic_error(u8"Registry settings store was not opened as writable");
    }

    storeValue<std::string>(thisSettingsKeyHandle, categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // defined(NUCLEX_SUPPORT_WINDOWS)
