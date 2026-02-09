#pragma once

#include <View/UIFSMAdapter.hpp>
#include <imgui.h>
#include <memory>

namespace view {

/**
 * @brief ImGUI widget for coordinate input with expression support
 *
 * CoordinateInputWidget provides a user interface for precise coordinate entry
 * with expression parsing support. It follows the stateless coordinator pattern
 * - querying state from UIFSMAdapter each frame rather than maintaining local
 * visibility state.
 *
 * The widget provides:
 * - Input fields for X, Y, Z coordinates
 * - Absolute/Relative coordinate mode toggle
 * - Expression parsing support (via CoordinateInputManager)
 * - Apply button to submit coordinates
 * - Clear button to reset input
 * - Coordinate preview (parsed result display)
 */
class CoordinateInputWidget {
public:
  /**
   * @brief Construct a new Coordinate Input Widget object
   * @param uiFSMAdapter Pointer to UIFSMAdapter for state queries and updates
   *
   * The UIFSMAdapter pointer is stored for state queries. CoordinateInputWidget
   * does not manage the lifecycle of UIFSMAdapter.
   */
  explicit CoordinateInputWidget(UIFSMAdapter *uiFSMAdapter);

  /**
   * @brief Destroy the Coordinate Input Widget object
   */
  ~CoordinateInputWidget() = default;

  /**
   * @brief Render the coordinate input widget
   *
   * Displays the coordinate input window with all configuration options.
   * Queries current settings from UIFSMAdapter and updates them when
   * user modifies values through ImGUI widgets.
   */
  void render();

private:
  /// Pointer to UIFSMAdapter (non-owning)
  UIFSMAdapter *uiFSMAdapter_;

  /// Input buffers for coordinate entry (temporary state, not visibility)
  char bufferX_[64];
  char bufferY_[64];
  char bufferZ_[64];

  /**
   * @brief Render coordinate input fields
   *
   * Displays input fields for X, Y, Z coordinates with labels.
   */
  void renderCoordinateInputs();

  /**
   * @brief Render mode toggle
   *
   * Displays radio buttons for switching between Absolute and Relative modes.
   */
  void renderModeToggle();

  /**
   * @brief Render action buttons
   *
   * Displays Apply, Clear, and Cancel buttons.
   */
  void renderButtons();

  /**
   * @brief Render coordinate preview
   *
   * Displays the parsed result of coordinate input.
   */
  void renderCoordinatePreview();
};

} // namespace view
