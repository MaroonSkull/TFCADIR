#pragma once

#include <functional>
#include <imgui.h>
#include <string>

namespace view::ImGUI {

/**
 * @brief ImGUI button widget for tool activation in Command Manager toolbar
 * @details Provides a specialized button widget for CAD tool activation with
 * SolidWorks-inspired design including icons, labels, keyboard shortcuts, and
 * tooltips.
 *
 * Features:
 * - Icon and text label display
 * - Active state highlighting
 * - Keyboard shortcut display
 * - Tooltip support
 * - Click callback for tool activation
 *
 * Thread Safety: This class is NOT thread-safe. External synchronization
 * is required if used from multiple threads.
 */
class ToolButton {
public:
  /// Callback function type for button click events
  using ClickCallback = std::function<void()>;

  /**
   * @brief Construct a ToolButton
   * @param id Unique identifier for the tool
   * @param label Display label for the button
   * @param icon Icon character (UTF-8) or empty string for no icon
   * @param shortcut Keyboard shortcut (e.g., "L", "Ctrl+Z") or empty string
   * @param tooltip Tooltip text for hover
   * @param callback Function to call when button is clicked
   */
  ToolButton(const std::string &id, const std::string &label,
             const std::string &icon, const std::string &shortcut,
             const std::string &tooltip, ClickCallback callback);

  // Copy prohibition
  ToolButton(const ToolButton &) = delete;
  ToolButton &operator=(const ToolButton &) = delete;

  // Move permission
  ToolButton(ToolButton &&) noexcept = default;
  ToolButton &operator=(ToolButton &&) noexcept = default;

  /**
   * @brief Destructor
   */
  ~ToolButton() = default;

  /**
   * @brief Render the button and handle interaction
   * @return true if the button was clicked, false otherwise
   *
   * Renders the button using ImGUI. Handles hover, active states,
   * and click detection. Returns true if the button was clicked this frame.
   */
  bool render();

  /**
   * @brief Set the active state of the button
   * @param active true if this tool is currently active
   *
   * Active buttons are highlighted to show which tool is currently selected.
   */
  void setActive(bool active);

  /**
   * @brief Check if the button is currently active
   * @return true if this tool is active
   */
  bool isActive() const;

  /**
   * @brief Get the tool ID
   * @return Unique identifier for this tool
   */
  const std::string &getId() const;

  /**
   * @brief Get the button label
   * @return Display label text
   */
  const std::string &getLabel() const;

  /**
   * @brief Set the click callback
   * @param callback Function to call when button is clicked
   */
  void setCallback(ClickCallback callback);

private:
  /// Unique identifier for the tool
  std::string id_;

  /// Display label for the button
  std::string label_;

  /// Icon character (UTF-8) or empty string
  std::string icon_;

  /// Keyboard shortcut (e.g., "L", "Ctrl+Z")
  std::string shortcut_;

  /// Tooltip text for hover
  std::string tooltip_;

  /// Function to call when button is clicked
  ClickCallback callback_;

  /// Active state flag
  bool active_;

  /**
   * @brief Render the button content (icon, label, shortcut)
   *
   * Internal helper to render the button's visual content.
   */
  void renderContent();
};

} // namespace view::ImGUI
