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

#ifndef NUCLEX_SUPPORT_SETTINGS_SETTINGSSTORE_H
#define NUCLEX_SUPPORT_SETTINGS_SETTINGSSTORE_H

#include "Nuclex/Support/Config.h"

#include <vector> // for std::vector
#include <string> // for std::string
#include <cstdint> // for std::uint32_t, std::int32_t, std::uint64_t, std::int64_t
#include <optional> // for std::optional
#include <type_traits> // for std::is_same

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores application settings as named properties</summary>
  /// <remarks>
  ///   <para>
  ///     Through this interface, settings can be accessed and modified, regardless of
  ///     whether they're stored temporarily in memory, in an .ini / .cfg file or
  ///     buried somewhere in the registry of a Windows system.
  ///   </para>
  ///   <para>
  ///     For simplicity, only booleans, 32 bit integers, 64 bit integers and strings
  ///     are supported. These are the types that can be natively represented by common
  ///     .ini files and the Windows registry. It also avoids burdening implementations
  ///     with complex serialization.
  ///   </para>
  ///   <para>
  ///     Any implementation has to also support categories. These are a single directory
  ///     level of folders each holding their own properties, allowing a basic level of
  ///     separation. In .ini files these are mapped to sections and in the registry
  ///     they're mapped to a single sublevel of registry keys.
  ///   </para>
  ///   <para>
  ///     Any properties in an .ini file before the first section marker, or the root level
  ///     of the mapped Windows registry path, are represented as a nameless category.
  ///     So if you need to access properties stored at this level, simply pass an empty
  ///     string as the category name.
  ///   </para>
  ///   <example>
  ///     <code>
  ///       void applySettings(const SettingsStore &settings);
  ///
  ///       int main() {
  ///         {
  ///           const IniSettingsStore settings(u8"appsettings.ini");
  ///           applySettings(settings);
  ///         }
  ///
  ///         doAwesomeThing();
  ///       }
  ///
  ///       // By using the base class, this method doesn't care whether it's
  ///       // reading properties from the registry, an .ini file or being handed
  ///       // a MemorySettingStore as a mock from some unit test.
  ///       void applySettings(const SettingsStore &settings) {
  ///         int theMagicNumber = (
  ///           settings.Retrieve&lt;int&gt;(std::string(), u8"MagicNumber").value_or(0)
  ///         );
  ///         setMagicNumber(theMagicNumber);
  ///       }
  ///     </code>
  ///   </example>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE SettingsStore {

    /// <summary>Frees all resources owned by the settings store</summary>
    public: NUCLEX_SUPPORT_API virtual ~SettingsStore() = default;

    /// <summary>Returns a list of all categories contained in the store</summary>
    /// <returns>A list of all categories present in the store currently</returns>
    /// <remarks>
    ///   Categories are like folders containing independent sets of properties.
    ///   See the remarks section of the <see cref="SettingsStore" /> class for
    ///   a detailed explanation of categories.
    /// </remarks>
    public: virtual std::vector<std::string> GetAllCategories() const = 0;

    /// <summary>Returns a list of all properties found within a category</summary>
    /// <param name="categoryName">Name of the category whose properties will be returned</param>
    /// <returns>A list of all properties present in the specified category</returns>
    /// <remarks>
    ///   If the root level of properties should be listed, pass an empty string as
    ///   the category name. Specifying the name of a category that doesn't exist will
    ///   simply return an empty list (because )
    /// </remarks>
    public: virtual std::vector<std::string> GetAllProperties(
      const std::string &categoryName = std::string()
    ) const = 0;

    /// <summary>Deletes an entire category with all its properties from the store</summary>
    /// <param name="categoryName">Name of the category that will be deleted</param>
    /// <returns>True if the category existed and was deleted, false otherwise</returns>
    public: virtual bool DeleteCategory(
      const std::string &categoryName
    ) = 0;

    /// <summary>Deletes the specified property from the store</summary>
    /// <param name="categoryName">
    ///   Name of the category from which the property will be deleted
    /// </param>
    /// <param name="propertyName">Name of the property that will be deleted</param>
    /// <returns>True if the property existed and was deleted, false otherwise</returns>
    public: virtual bool DeleteProperty(
      const std::string &categoryName, const std::string &propertyName
    ) = 0;

    /// <summary>Retrieves the value of a property from the store</summary>
    /// <typeparam name="TValue">
    ///   Type of the property value that will be retrieved, must be either a boolean,
    ///   32/64 bit integer or a string. No other types are supported.
    /// </typeparam>
    /// <param name="categoryName">
    ///   Name of the category from which the property will be retrieved. Pass an empty
    ///   string to access the root category.
    /// </param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    public: template<typename TValue>
    NUCLEX_SUPPORT_API inline std::optional<TValue> Retrieve(
      const std::string &categoryName, const std::string &propertyName
    ) const;

    /// <summary>Stores a property's value in the store or updates it</summary>
    /// <typeparam name="TValue">
    ///   Type of the property value that will be stored, must be either a boolean,
    ///   32/64 bit integer or a string. No other types are supported.
    /// </typeparam>
    /// <param name="categoryName">
    ///   Name of the category in which the property will be stored. Pass an empty
    ///   string to store it in the root category.
    /// </param>
    /// <param name="propertyName">Name of the property whose value will be stored</param>
    /// <param name="value">Value that will be stored in the settings store</param>
    public: template<typename TValue>
    NUCLEX_SUPPORT_API inline void Store(
      const std::string &categoryName, const std::string &propertyName, const TValue &value
    );

    //
    // *** public interface ends here, all methods below are protected or private ***
    //

    /// <summary>Retrieves the value of a boolean property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: virtual std::optional<bool> RetrieveBooleanProperty(
      const std::string &categoryName, const std::string &propertyName
    ) const = 0;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: virtual std::optional<std::uint32_t> RetrieveUInt32Property(
      const std::string &categoryName, const std::string &propertyName
    ) const = 0;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: virtual std::optional<std::int32_t> RetrieveInt32Property(
      const std::string &categoryName, const std::string &propertyName
    ) const = 0;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: virtual std::optional<std::uint64_t> RetrieveUInt64Property(
      const std::string &categoryName, const std::string &propertyName
    ) const = 0;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: virtual std::optional<std::int64_t> RetrieveInt64Property(
      const std::string &categoryName, const std::string &propertyName
    ) const = 0;

    /// <summary>Retrieves the value of a string property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: virtual std::optional<std::string> RetrieveStringProperty(
      const std::string &categoryName, const std::string &propertyName
    ) const = 0;

    /// <summary>Stores or updates a boolean property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: virtual void StoreBooleanProperty(
      const std::string &categoryName, const std::string &propertyName, bool value
    ) = 0;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: virtual void StoreUInt32Property(
      const std::string &categoryName, const std::string &propertyName, std::uint32_t value
    ) = 0;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: virtual void StoreInt32Property(
      const std::string &categoryName, const std::string &propertyName, std::int32_t value
    ) = 0;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: virtual void StoreUInt64Property(
      const std::string &categoryName, const std::string &propertyName, std::uint64_t value
    ) = 0;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: virtual void StoreInt64Property(
      const std::string &categoryName, const std::string &propertyName, std::int64_t value
    ) = 0;

    /// <summary>Stores or updates a string property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: virtual void StoreStringProperty(
      const std::string &categoryName, const std::string &propertyName, const std::string &value
    ) = 0;

  };

  // ------------------------------------------------------------------------------------------- //

  template<typename TValue>
  NUCLEX_SUPPORT_API inline std::optional<TValue> SettingsStore::Retrieve(
    const std::string &, const std::string &
  ) const {
    static_assert(
      (
        std::is_same<TValue, bool>::value ||
        std::is_same<TValue, std::uint32_t>::value ||
        std::is_same<TValue, std::int32_t>::value ||
        std::is_same<TValue, std::uint64_t>::value ||
        std::is_same<TValue, std::int64_t>::value ||
        std::is_same<TValue, std::string>::value
      ) &&
      u8"Only boolean, 32/64 bit integer and std::string properties are supported"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline std::optional<bool> SettingsStore::Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return RetrieveBooleanProperty(categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline std::optional<std::uint32_t> SettingsStore::Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return RetrieveUInt32Property(categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline std::optional<std::int32_t> SettingsStore::Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return RetrieveInt32Property(categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline std::optional<std::uint64_t> SettingsStore::Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return RetrieveUInt64Property(categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline std::optional<std::int64_t> SettingsStore::Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return RetrieveInt64Property(categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline std::optional<std::string> SettingsStore::Retrieve(
    const std::string &categoryName, const std::string &propertyName
  ) const {
    return RetrieveStringProperty(categoryName, propertyName);
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TValue>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &, const std::string &, const TValue &
  ) {
    static_assert(
      (
        std::is_same<TValue, bool>::value ||
        std::is_same<TValue, std::uint32_t>::value ||
        std::is_same<TValue, std::int32_t>::value ||
        std::is_same<TValue, std::uint64_t>::value ||
        std::is_same<TValue, std::int64_t>::value ||
        std::is_same<TValue, std::string>::value
      ) &&
      u8"Only boolean, 32/64 bit integer and std::string properties are supported"
    );
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &categoryName, const std::string &propertyName, const bool &value
  ) {
    StoreBooleanProperty(categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &categoryName, const std::string &propertyName, const std::uint32_t &value
  ) {
    StoreUInt32Property(categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &categoryName, const std::string &propertyName, const std::int32_t &value
  ) {
    StoreInt32Property(categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &categoryName, const std::string &propertyName, const std::uint64_t &value
  ) {
    StoreUInt64Property(categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &categoryName, const std::string &propertyName, const std::int64_t &value
  ) {
    StoreInt64Property(categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

  template<>
  NUCLEX_SUPPORT_API inline void SettingsStore::Store(
    const std::string &categoryName, const std::string &propertyName, const std::string &value
  ) {
    StoreStringProperty(categoryName, propertyName, value);
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_SETTINGSSTORE_H
