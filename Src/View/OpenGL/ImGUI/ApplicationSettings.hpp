/**
 * @file ApplicationSettings.hpp
 * @brief Application settings persistence for TFCADIR
 *
 * Phase 9.5-r: Properties Panel Rework
 *
 * Provides persistent storage for application-wide settings including
 * grid and snap configuration. Settings are saved to YAML file.
 *
 * Uses existing GridSettings and SnapSettings from UIFSMAdapter.hpp.
 */

#ifndef TFCADIR_APPLICATION_SETTINGS_HPP
#define TFCADIR_APPLICATION_SETTINGS_HPP

#include <View/UIFSMAdapter.hpp>

#include <functional>
#include <string>

namespace view {

/**
 * @brief Application-wide settings with persistence
 *
 * Manages grid, snap, and other application settings with
 * automatic save/load from YAML configuration file.
 *
 * This class provides persistence for the GridSettings and SnapSettings
 * already defined in UIFSMAdapter.hpp.
 */
class ApplicationSettings {
public:
  /// Callback type for settings change notifications
  using SettingsChangedCallback = std::function<void()>;

  /**
   * @brief Construct settings with optional config file path
   * @param configPath Path to YAML configuration file
   */
  explicit ApplicationSettings(
      const std::string &configPath = "config/application_settings.yaml");

  /**
   * @brief Destructor - saves settings on destruction
   */
  ~ApplicationSettings();

  // === Grid Settings ===

  /**
   * @brief Get current grid settings
   * @return Const reference to grid settings
   */
  const GridSettings &getGridSettings() const;

  /**
   * @brief Set grid settings
   * @param settings New grid settings
   */
  void setGridSettings(const GridSettings &settings);

  /**
   * @brief Check if grid is visible
   * @return true if grid should be displayed
   */
  bool isGridVisible() const;

  /**
   * @brief Set grid visibility
   * @param visible Whether grid should be displayed
   */
  void setGridVisible(bool visible);

  /**
   * @brief Get grid major spacing
   * @return Grid major spacing in world units
   */
  float getGridMajorSpacing() const;

  /**
   * @brief Set grid major spacing
   * @param spacing Grid major spacing in world units
   */
  void setGridMajorSpacing(float spacing);

  // === Snap Settings ===

  /**
   * @brief Get current snap settings
   * @return Const reference to snap settings
   */
  const SnapSettings &getSnapSettings() const;

  /**
   * @brief Set snap settings
   * @param settings New snap settings
   */
  void setSnapSettings(const SnapSettings &settings);

  /**
   * @brief Check if grid snap is enabled
   * @return true if grid snap is active
   */
  bool isSnapEnabled() const;

  /**
   * @brief Set grid snap enabled state
   * @param enabled Whether grid snap should be active
   */
  void setSnapEnabled(bool enabled);

  // === Persistence ===

  /**
   * @brief Load settings from YAML file
   * @return true if loaded successfully, false otherwise
   */
  bool load();

  /**
   * @brief Save settings to YAML file
   * @return true if saved successfully, false otherwise
   */
  bool save();

  // === Callbacks ===

  /**
   * @brief Set callback for settings changes
   * @param callback Function to call when settings change
   */
  void setSettingsChangedCallback(SettingsChangedCallback callback);

private:
  std::string configPath_;
  GridSettings gridSettings_;
  SnapSettings snapSettings_;
  SettingsChangedCallback settingsChangedCallback_;

  /**
   * @brief Notify callback of settings change
   */
  void notifySettingsChanged();

  /**
   * @brief Ensure config directory exists
   * @return true if directory exists or was created
   */
  bool ensureConfigDirectory();
};

} // namespace view

#endif // TFCADIR_APPLICATION_SETTINGS_HPP
