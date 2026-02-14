#pragma once

#include <View/UIFSMAdapter.hpp>
#include <imgui.h>
#include <memory>

namespace view {

/**
 * @brief ImGUI panel for grid settings configuration
 *
 * GridSettingsPanel provides a user interface for configuring grid
 * visualization settings. It follows the stateless coordinator pattern -
 * querying state from UIFSMAdapter each frame rather than maintaining local
 * state.
 *
 * The panel is divided into three sections:
 * - Display Settings: Grid visibility, colors, opacity
 * - Grid Settings: Major/minor spacing, divisions
 * - Axis Settings: Show axes, show origin
 */
class GridSettingsPanel {
public:
  /**
   * @brief Construct a new Grid Settings Panel object
   * @param uiFSMAdapter Pointer to UIFSMAdapter for state queries and updates
   *
   * The UIFSMAdapter pointer is stored for state queries. GridSettingsPanel
   * does not manage the lifecycle of UIFSMAdapter.
   */
  explicit GridSettingsPanel(UIFSMAdapter *uiFSMAdapter);

  /**
   * @brief Destroy the Grid Settings Panel object
   */
  ~GridSettingsPanel() = default;

  /**
   * @brief Render the grid settings panel
   *
   * Displays the grid settings window with all configuration options.
   * Queries current settings from UIFSMAdapter and updates them when
   * user modifies values through ImGUI widgets.
   */
  void render();

private:
  /// Pointer to UIFSMAdapter (non-owning)
  UIFSMAdapter *uiFSMAdapter_;

  /**
   * @brief Render display settings section
   *
   * Displays widgets for:
   * - Grid visibility checkbox
   * - Show minor lines checkbox
   * - Major grid color picker
   * - Minor grid color picker
   * - Grid opacity slider
   */
  void renderDisplaySettings();

  /**
   * @brief Render grid settings section
   *
   * Displays widgets for:
   * - Major spacing drag float
   * - Minor divisions slider
   */
  void renderGridSettings();

  /**
   * @brief Render axis settings section
   *
   * Displays widgets for:
   * - Show axes checkbox
   * - Show origin checkbox
   */
  void renderAxisSettings();
};

} // namespace view
