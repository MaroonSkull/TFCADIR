#pragma once

#include <View/UIFSMAdapter.hpp>
#include <imgui.h>
#include <memory>

namespace view {

/**
 * @brief ImGUI panel for snap settings configuration
 *
 * SnapSettingsPanel provides a user interface for configuring snap
 * modes and settings for precision drawing. It follows the stateless
 * coordinator pattern - querying state from UIFSMAdapter each frame
 * rather than maintaining local state.
 *
 * The panel is divided into two sections:
 * - Snap Modes: Checkboxes for enabling/disabling snap modes
 * - Snap Settings: Tolerance, indicator visibility, indicator color
 */
class SnapSettingsPanel {
public:
  /**
   * @brief Construct a new Snap Settings Panel object
   * @param uiFSMAdapter Pointer to UIFSMAdapter for state queries and updates
   *
   * The UIFSMAdapter pointer is stored for state queries. SnapSettingsPanel
   * does not manage the lifecycle of UIFSMAdapter.
   */
  explicit SnapSettingsPanel(UIFSMAdapter *uiFSMAdapter);

  /**
   * @brief Destroy the Snap Settings Panel object
   */
  ~SnapSettingsPanel() = default;

  /**
   * @brief Render the snap settings panel
   *
   * Displays the snap settings window with all configuration options.
   * Queries current settings from UIFSMAdapter and updates them when
   * user modifies values through ImGUI widgets.
   */
  void render();

private:
  /// Pointer to UIFSMAdapter (non-owning)
  UIFSMAdapter *uiFSMAdapter_;

  /**
   * @brief Render snap modes section
   *
   * Displays checkboxes for enabling/disabling snap modes:
   * - Grid snap
   * - Endpoint snap
   * - Midpoint snap
   * - Center snap
   * - Intersection snap
   * - Nearest point snap
   * - Tangent snap
   * - Perpendicular snap
   */
  void renderSnapModes();

  /**
   * @brief Render snap settings section
   *
   * Displays widgets for:
   * - Snap tolerance slider
   * - Show indicators checkbox
   * - Indicator color picker
   */
  void renderSnapSettings();
};

} // namespace view
