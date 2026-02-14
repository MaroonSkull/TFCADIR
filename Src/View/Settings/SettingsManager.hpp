/**
 * @file SettingsManager.hpp
 * @brief Settings persistence manager for loading and saving application
 * settings
 *
 * This file contains the SettingsManager class which provides singleton
 * access to application settings and handles persistence to YAML files.
 */

#pragma once

#include "SettingsConfig.hpp"

#include <memory>
#include <string>

namespace view {
namespace settings {

/**
 * @brief Settings persistence manager for application settings
 *
 * Provides singleton access to application settings and handles
 * loading from and saving to YAML configuration files.
 * Manages both grid and display settings.
 */
class SettingsManager {
public:
  /**
   * @brief Get the singleton instance of SettingsManager
   * @return Reference to the singleton instance
   */
  static SettingsManager &Instance();

  /**
   * @brief Load settings from a YAML file
   * @param filepath Path to the YAML settings file
   * @throws std::runtime_error if the file cannot be read or parsed
   */
  void LoadFromFile(const std::string &filepath);

  /**
   * @brief Save settings to a YAML file
   * @param filepath Path to the YAML settings file
   * @throws std::runtime_error if the file cannot be written
   */
  void SaveToFile(const std::string &filepath);

  /**
   * @brief Get the current grid configuration
   * @return Copy of the current grid configuration
   */
  GridConfig GetGridConfig() const;

  /**
   * @brief Set the grid configuration
   * @param config The new grid configuration
   */
  void SetGridConfig(const GridConfig &config);

  /**
   * @brief Get the current display configuration
   * @return Copy of the current display configuration
   */
  DisplayConfig GetDisplayConfig() const;

  /**
   * @brief Set the display configuration
   * @param config The new display configuration
   */
  void SetDisplayConfig(const DisplayConfig &config);

  /**
   * @brief Get the complete application settings
   * @return Copy of the current application settings
   */
  AppSettings GetAppSettings() const;

  /**
   * @brief Set the complete application settings
   * @param settings The new application settings
   */
  void SetAppSettings(const AppSettings &settings);

  /**
   * @brief Apply current settings to the application
   *
   * Applies the current display theme and grid settings to the UI.
   */
  void ApplySettings();

  /**
   * @brief Reset all settings to default values
   */
  void ResetToDefaults();

  /**
   * @brief Check if settings have been modified since last save
   * @return true if there are unsaved changes
   */
  bool HasUnsavedChanges() const;

  /**
   * @brief Mark settings as saved (clear unsaved changes flag)
   */
  void MarkAsSaved();

private:
  /// Private constructor for singleton pattern
  SettingsManager();

  /// Private destructor for singleton pattern
  ~SettingsManager() = default;

  /// Deleted copy constructor
  SettingsManager(const SettingsManager &) = delete;

  /// Deleted assignment operator
  SettingsManager &operator=(const SettingsManager &) = delete;

  /// Current grid configuration
  GridConfig gridConfig_;

  /// Current display configuration
  DisplayConfig displayConfig_;

  /// Flag indicating unsaved changes
  bool hasUnsavedChanges_{false};

  /// Path to the last loaded/saved settings file
  std::string lastFilePath_;

  /**
   * @brief Load default settings values
   */
  void LoadDefaults();

  /**
   * @brief Parse grid settings from a YAML node
   * @param node The YAML node containing grid settings
   */
  void ParseGridConfig(const std::string &content);

  /**
   * @brief Parse display settings from a YAML node
   * @param node The YAML node containing display settings
   */
  void ParseDisplayConfig(const std::string &content);

  /**
   * @brief Serialize grid settings to a YAML string
   * @return YAML string representation of grid settings
   */
  std::string SerializeGridConfig() const;

  /**
   * @brief Serialize display settings to a YAML string
   * @return YAML string representation of display settings
   */
  std::string SerializeDisplayConfig() const;
};

} // namespace settings
} // namespace view
