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

#ifndef NUCLEX_SUPPORT_SETTINGS_INISETTINGSSTORE_H
#define NUCLEX_SUPPORT_SETTINGS_INISETTINGSSTORE_H

#include "Nuclex/Support/Config.h"
#include "Nuclex/Support/Settings/SettingsStore.h"

#include <vector> // for std::vector

namespace Nuclex { namespace Support { namespace Settings {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Stores application settings in an .ini / .cfg file</summary>
  /// <remarks>
  ///   <para>
  ///     This implementation of the settings store reads and writes traditional .ini files.
  ///     It does its very best to be non-destructive, meaning that ordering, formatting
  ///     and comments in .ini files are preserved even when they are updated.
  ///   </para>
  ///   <para>
  ///     Using .ini files is the recommended way to store local application configuration
  ///     because it's easy to understand, portable and makes it simple to copy configurations
  ///     around and fully delete an application without potentially leaving unwanted stuff
  ///     behind as would be the case with some alternatives such as the Windows registry.
  ///   </para>
  ///   <para>
  ///     Do notice that this implementation does not automatically update the file on disk
  ///     when values change. You will have to call <see cref="Save" /> upon completing your
  ///     changes or before exiting the application. To aid you in deciding whether this is
  ///     necessary, the <see cref="HasChangedSinceLoading" /> method is provided.
  ///   </para>
  ///   <example>
  ///     <code>
  ///       void test() {
  ///         IniSettingsStore settings(u8"GameSettings.ini");
  ///
  ///         // Retrieve() returns std::optional&lt;T&gt;, so you can either
  ///         // check if the value was present with .has_value() and .value() or
  ///         // directly provide a default via .value_or() as shown below
  ///
  ///         std::uint32_t resolutionX = settings.Retrieve&lt;std::uint32_t&gt;(
  ///           u8"Video", u8"ResolutionX").value_or(1920)
  ///         );
  ///         std::uint32_t resolutionY = settings.Retrieve&lt;std::uint32_t&gt;(
  ///           u8"Video", u8"ResolutionY").value_or(1080)
  ///         );
  ///
  ///         settings.Store&lt;bool&gt;(std::string(), u8"FirstLaunch", false);
  ///
  ///         settings.Save(u8"GameSettings.ini");
  ///       }
  ///     </code>
  ///   </example>
  ///   <para>
  ///     Figuring out the path in which the look for/store an .ini file is not
  ///     covered by this class. One options is to use the Nuclex.Support.Storage
  ///     library to determine the paths to your application's executable directory,
  ///     data directory or user settings directory. Another option would be to use
  ///     the <see cref="Nuclex.Support.Threading.Process.GetExecutableDirectory" />
  ///     method, but not that storing .ini files in your application directory is
  ///     not a good idea for cross-platform development.
  ///   </para>
  /// </remarks>
  class NUCLEX_SUPPORT_TYPE IniSettingsStore : public SettingsStore {

    /// <summary>Initializes a new .ini settings store with no backing file</summary>
    /// <remarks>
    ///   The configuration may be saved as an .ini file at a later point in time by
    ///   using the <see cref="Save" /> method. If you only need a transient settings store,
    ///   you should prefer the <see cref="MemorySettingsStore" /> as it's much faster
    ///   while also reducing processing overhead.
    /// </remarks>
    public: NUCLEX_SUPPORT_API IniSettingsStore();

    /// <summary>
    ///   Initializes a new .ini settings store with settings from the specified file
    /// </summary>
    /// <param name="iniFilePath">Absolute path to the .ini file that will be loaded</param>
    public: NUCLEX_SUPPORT_API IniSettingsStore(const std::string &iniFilePath);

    /// <summary>
    ///   Initializes a new .ini settings store with settings loaded from an .ini file
    ///   that has already been copied to memory
    /// </summary>
    /// <param name="iniFileContents">Contents of the .ini file in memory</param>
    /// <param name="iniFileByteCount">Total length of the .ini file in bytes</param>
    public: NUCLEX_SUPPORT_API IniSettingsStore(
      const std::uint8_t *iniFileContents, std::size_t iniFileByteCount
    );

    /// <summary>Frees all resources owned by the .ini settings store</summary>
    public: NUCLEX_SUPPORT_API ~IniSettingsStore() override;

    /// <summary>Loads the settings from an .ini file</summary>
    /// <param name="iniFilePath">Absolute path to the .ini file that will be loaded</param>
    public: NUCLEX_SUPPORT_API void Load(const std::string &iniFilePath);

    /// <summary>
    ///   Loads the settings from an .ini file that has already been copied into memory
    /// </summary>
    /// <param name="iniFileContents">Contents of the .ini file in memory</param>
    /// <param name="iniFileByteCount">Total length of the .ini file in bytes</param>
    public: NUCLEX_SUPPORT_API void Load(
      const std::uint8_t *iniFileContents, std::size_t iniFileByteCount
    );

    /// <summary>Saves the settings into an .ini file with the specified name</summary>
    /// <param name="iniFilePath">Absolute path where the .ini file will be saved</param>
    public: NUCLEX_SUPPORT_API void Save(const std::string &iniFilePath) const;

    /// <summary>Saves the settings into an .ini file that is created in memory</summary>
    /// <returns>A memory block holding the file contents of the .ini file</returns>
    public: NUCLEX_SUPPORT_API std::vector<std::uint8_t> Save() const;

    /// <summary>Checks if any settings have changed since the .ini file was loaded</summary>
    /// <returns>True if the settings were modified, false if no changes were made</returns>
    public: NUCLEX_SUPPORT_API bool HasChangedSinceLoad() const;

    /// <summary>Returns a list of all categories contained in the store</summary>
    /// <returns>A list of all categories present in the store currently</returns>
    public: NUCLEX_SUPPORT_API std::vector<std::string> GetAllCategories() const override;

    /// <summary>Returns a list of all properties found within a category</summary>
    /// <param name="categoryName">Name of the category whose properties will be returned</param>
    /// <returns>A list of all properties present in the specified category</returns>
    /// <remarks>
    ///   If the root level of properties should be listed, pass an empty string as
    ///   the category name. Specifying the name of a category that doesn't exist will
    ///   simply return an empty list (because )
    /// </remarks>
    public: NUCLEX_SUPPORT_API std::vector<std::string> GetAllProperties(
      const std::string &categoryName = std::string()
    ) const override;

    /// <summary>Deletes an entire category with all its properties from the store</summary>
    /// <param name="categoryName">Name of the category that will be deleted</param>
    /// <returns>True if the category existed and was deleted, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool DeleteCategory(
      const std::string &categoryName
    ) override;

    /// <summary>Deletes the specified property from the store</summary>
    /// <param name="categoryName">
    ///   Name of the category from which the property will be deleted
    /// </param>
    /// <param name="propertyName">Name of the property that will be deleted</param>
    /// <returns>True if the property existed and was deleted, false otherwise</returns>
    public: NUCLEX_SUPPORT_API bool DeleteProperty(
      const std::string &categoryName, const std::string &propertyName
    ) override;

    /// <summary>Retrieves the value of a boolean property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<bool> RetrieveBooleanProperty(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::uint32_t> RetrieveUInt32Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 32 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::int32_t> RetrieveInt32Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::uint64_t> RetrieveUInt64Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a 64 bit integer property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::int64_t> RetrieveInt64Property(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Retrieves the value of a string property from the store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <returns>The value of the requested property or nothing if it didn't exist</returns>
    protected: NUCLEX_SUPPORT_API std::optional<std::string> RetrieveStringProperty(
      const std::string &categoryName, const std::string &propertyName
    ) const override;

    /// <summary>Stores or updates a boolean property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreBooleanProperty(
      const std::string &categoryName, const std::string &propertyName, bool value
    ) override;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreUInt32Property(
      const std::string &categoryName, const std::string &propertyName, std::uint32_t value
    ) override;

    /// <summary>Stores or updates a 32 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreInt32Property(
      const std::string &categoryName, const std::string &propertyName, std::int32_t value
    ) override;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreUInt64Property(
      const std::string &categoryName, const std::string &propertyName, std::uint64_t value
    ) override;

    /// <summary>Stores or updates a 64 bit integer property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreInt64Property(
      const std::string &categoryName, const std::string &propertyName, std::int64_t value
    ) override;

    /// <summary>Stores or updates a string property in the settings store</summary>
    /// <param name="categoryName">Category from which the property will be read</param>
    /// <param name="propertyName">Name of the property whose value will be read</param>
    /// <param name="value">Value that will be stored</param>
    protected: NUCLEX_SUPPORT_API void StoreStringProperty(
      const std::string &categoryName, const std::string &propertyName, const std::string &value
    ) override;

    /// <summary>Hidden document model and formatting informations</summary>
    private: struct PrivateImplementationData;
    /// <summary>Hidden implementation details only required internally</summary>
    private: PrivateImplementationData *privateImplementationData;
    /// <summary>Whether the settings have been modified since they were loaded</summary>
    private: bool modified;

  };

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Support::Settings

#endif // NUCLEX_SUPPORT_SETTINGS_MEMORYSETTINGSSTORE_H
