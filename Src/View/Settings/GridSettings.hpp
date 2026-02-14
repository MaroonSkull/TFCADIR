/**
 * @file GridSettings.hpp
 * @brief Grid settings dialog for configuring grid visualization
 *
 * This file contains the GridSettings class which provides a dialog
 * for configuring grid display options including visibility, spacing,
 * colors, opacity, and grid type.
 */

#pragma once

#include "SettingsConfig.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>

namespace view {
namespace settings {

/**
 * @brief Grid settings dialog for configuring grid visualization
 *
 * Provides a modal dialog for users to configure grid display settings
 * including visibility toggle, major/minor grid spacing, colors,
 * opacity, and grid type selection.
 */
class GridSettings {
public:
  /**
   * @brief Construct a new Grid Settings dialog
   */
  GridSettings();

  /**
   * @brief Render the grid settings dialog
   * @param config Reference to the grid configuration to modify
   * @return true if settings were modified during this frame
   */
  bool Render(GridConfig &config);

  /**
   * @brief Open the grid settings dialog
   */
  void Open();

  /**
   * @brief Close the grid settings dialog
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
  void SetOnApply(std::function<void(const GridConfig &)> callback);

  /**
   * @brief Reset to default values
   * @param config Reference to config to reset
   */
  static void ResetToDefaults(GridConfig &config);

private:
  /// Whether the dialog is currently open
  bool isOpen_{false};

  /// Working copy of config (for cancel functionality)
  GridConfig workingConfig_;

  /// Saved copy for change detection
  GridConfig savedConfig_;

  /// Callback for when settings are applied
  std::function<void(const GridConfig &)> onApply_;

  /**
   * @brief Render the grid visibility section
   * @return true if any value was modified
   */
  bool RenderVisibilitySection();

  /**
   * @brief Render the grid spacing section
   * @return true if any value was modified
   */
  bool RenderSpacingSection();

  /**
   * @brief Render the grid color section
   * @return true if any value was modified
   */
  bool RenderColorSection();

  /**
   * @brief Render the grid type section
   * @return true if any value was modified
   */
  bool RenderTypeSection();

  /**
   * @brief Render the dialog buttons (Apply, Cancel, Reset)
   * @param config Reference to the original config
   * @return true if dialog should close
   */
  bool RenderButtons(GridConfig &config);
};

} // namespace settings
} // namespace view
