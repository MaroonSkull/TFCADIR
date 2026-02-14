/**
 * @file DisplaySettings.hpp
 * @brief Display settings dialog for configuring UI appearance
 *
 * This file contains the DisplaySettings class which provides a dialog
 * for configuring display options including theme, colors, fonts, and
 * UI element sizing.
 */

#pragma once

#include "SettingsConfig.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>
#include <vector>

namespace view {
namespace settings {

/**
 * @brief Display settings dialog for configuring UI appearance
 *
 * Provides a modal dialog for users to configure display settings
 * including theme selection, color customization, font selection,
 * and UI element sizing.
 */
class DisplaySettings {
public:
  /**
   * @brief Construct a new Display Settings dialog
   */
  DisplaySettings();

  /**
   * @brief Render the display settings dialog
   * @param config Reference to the display configuration to modify
   * @return true if settings were modified during this frame
   */
  bool Render(DisplayConfig &config);

  /**
   * @brief Open the display settings dialog
   */
  void Open();

  /**
   * @brief Close the display settings dialog
   */
  void Close();

  /**
   * @brief Check if the dialog is currently open
   * @return true if the dialog is open
   */
  bool IsOpen() const;

  /**
   * @brief Check if settings have changed since last apply
   * @return true if there are pending changes
   */
  bool HasChanged() const;

  /**
   * @brief Set callback for when settings are applied
   * @param callback Function to call when settings are applied
   */
  void SetOnApply(std::function<void(const DisplayConfig &)> callback);

  /**
   * @brief Apply theme to ImGui style
   * @param config The display configuration containing theme settings
   */
  static void ApplyTheme(const DisplayConfig &config);

  /**
   * @brief Reset to default values
   * @param config Reference to config to reset
   */
  static void ResetToDefaults(DisplayConfig &config);

  /**
   * @brief Get list of available fonts
   * @return Vector of available font names
   */
  static std::vector<std::string> GetAvailableFonts();

private:
  /// Whether the dialog is currently open
  bool isOpen_{false};

  /// Working copy of config (for cancel functionality)
  DisplayConfig workingConfig_;

  /// Saved copy for change detection
  DisplayConfig savedConfig_;

  /// Callback for when settings are applied
  std::function<void(const DisplayConfig &)> onApply_;

  /**
   * @brief Render the theme selection section
   * @return true if any value was modified
   */
  bool RenderThemeSection();

  /**
   * @brief Render the color customization section
   * @return true if any value was modified
   */
  bool RenderColorSection();

  /**
   * @brief Render the font selection section
   * @return true if any value was modified
   */
  bool RenderFontSection();

  /**
   * @brief Render the UI size section
   * @return true if any value was modified
   */
  bool RenderUISizeSection();

  /**
   * @brief Render the dialog buttons (Apply, Cancel, Reset)
   * @param config Reference to the original config
   * @return true if dialog should close
   */
  bool RenderButtons(DisplayConfig &config);

  /**
   * @brief Apply dark theme to ImGui style
   * @param style Reference to ImGui style to modify
   */
  static void ApplyDarkTheme(ImGuiStyle &style);

  /**
   * @brief Apply light theme to ImGui style
   * @param style Reference to ImGui style to modify
   */
  static void ApplyLightTheme(ImGuiStyle &style);

  /**
   * @brief Apply custom colors from config to ImGui style
   * @param style Reference to ImGui style to modify
   * @param config The display configuration with custom colors
   */
  static void ApplyCustomColors(ImGuiStyle &style, const DisplayConfig &config);
};

} // namespace settings
} // namespace view
