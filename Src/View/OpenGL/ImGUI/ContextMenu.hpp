#pragma once

#include <functional>
#include <imgui.h>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Represents a single item in a context menu
 *
 * Contains the display label, optional keyboard shortcut, action callback,
 * and enabled state for a menu item.
 */
struct ContextMenuItem {
  /// Display label for the menu item
  std::string label;

  /// Keyboard shortcut string (e.g., "Ctrl+Z")
  std::string shortcut;

  /// Action callback to execute when the menu item is selected
  std::function<void()> action;

  /// Whether the menu item is currently enabled
  bool enabled = true;

  /**
   * @brief Default constructor
   */
  ContextMenuItem() = default;

  /**
   * @brief Constructor with all fields
   * @param label Display label for the menu item
   * @param shortcut Keyboard shortcut string (optional)
   * @param action Action callback (optional)
   * @param enabled Whether the item is enabled
   */
  ContextMenuItem(std::string label, std::string shortcut = "",
                  std::function<void()> action = nullptr, bool enabled = true)
      : label(std::move(label)), shortcut(std::move(shortcut)),
        action(std::move(action)), enabled(enabled) {}
};

/**
 * @brief Base class for context menus
 *
 * Provides common functionality for context menus including visibility state,
 * position management, and rendering. Derived classes implement specific menu
 * structures and actions.
 */
class ContextMenu {
protected:
  /// Whether the menu is currently visible
  bool isVisible_ = false;

  /// Position where the menu was opened
  ImVec2 position_;

  /// Unique identifier for the popup
  std::string popupId_;

  /// List of menu items
  std::vector<ContextMenuItem> items_;

  /// Name of the last executed command (for "Repeat Last Command" feature)
  static inline std::string lastCommandName_;

  /**
   * @brief Render a separator in the menu
   */
  void renderSeparator();

  /**
   * @brief Render a single menu item
   * @param item The menu item to render
   * @return true if the item was clicked
   */
  bool renderMenuItem(const ContextMenuItem &item);

public:
  /**
   * @brief Construct a ContextMenu with a unique popup ID
   * @param popupId Unique identifier for the ImGUI popup
   */
  explicit ContextMenu(const std::string &popupId);

  /**
   * @brief Virtual destructor
   */
  virtual ~ContextMenu() = default;

  /**
   * @brief Show the context menu at the specified position
   * @param pos Screen position where the menu should appear
   */
  virtual void show(const ImVec2 &pos);

  /**
   * @brief Hide the context menu
   */
  virtual void hide();

  /**
   * @brief Check if the menu is currently visible
   * @return true if the menu is visible
   */
  bool isVisible() const;

  /**
   * @brief Render the context menu
   *
   * Must be called every frame. Handles the ImGUI popup rendering.
   */
  virtual void render() = 0;

  /**
   * @brief Get the last command name for "Repeat Last Command" feature
   * @return Name of the last executed command
   */
  static std::string getLastCommandName() { return lastCommandName_; }

  /**
   * @brief Set the last command name
   * @param name Name of the last executed command
   */
  static void setLastCommandName(const std::string &name) {
    lastCommandName_ = name;
  }
};

} // namespace view
