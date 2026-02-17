#pragma once

#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace view {

// Forward declarations
class UIFSMAdapter;
class SelectionManager;

/**
 * @brief Context menu type enumeration
 *
 * Defines the available context menu types in the application.
 */
enum class ContextMenuType {
  Canvas,  ///< Context menu for the canvas area
  Object,  ///< Context menu for selected objects
  Outliner ///< Context menu for the outliner panel
};

/**
 * @brief Context menu item
 *
 * Represents a single item in a context menu with its label,
 * optional keyboard shortcut, action callback, and enabled state.
 */
struct MenuItem {
  /// Unique identifier for the menu item
  std::string id;

  /// Display label for the menu item
  std::string label;

  /// Keyboard shortcut string (e.g., "Ctrl+Z")
  std::string shortcut;

  /// Action callback to execute when the menu item is selected
  std::function<void()> action;

  /// Whether the menu item is currently enabled
  bool enabled = true;

  /// Sub-menu items (for nested menus)
  std::vector<MenuItem> subItems;

  /**
   * @brief Default constructor
   */
  MenuItem() = default;

  /**
   * @brief Constructor with all fields
   */
  MenuItem(std::string id, std::string label, std::string shortcut = "",
           std::function<void()> action = nullptr, bool enabled = true)
      : id(std::move(id)), label(std::move(label)),
        shortcut(std::move(shortcut)), action(std::move(action)),
        enabled(enabled) {}
};

/**
 * @brief Context menu
 *
 * Represents a complete context menu with a title and list of menu items.
 */
struct Menu {
  /// Menu title (optional, for sub-menus)
  std::string title;

  /// List of menu items in this menu
  std::vector<MenuItem> items;

  /**
   * @brief Default constructor
   */
  Menu() = default;

  /**
   * @brief Constructor with title
   */
  explicit Menu(std::string title) : title(std::move(title)) {}
};

/**
 * @brief Manages right-click context menus throughout the application
 *
 * ContextMenuManager is a stateless coordinator that builds and displays
 * context menus for different contexts (canvas, object, outliner).
 * All UI state is queried from UIFSMAdapter, following the Phase 7
 * architecture pattern.
 *
 * This manager:
 * - Builds context menus dynamically based on current state
 * - Handles menu item actions
 * - Integrates with SelectionManager for object context
 * - Displays keyboard shortcuts in menu items
 *
 * No local domain state is stored - all state comes from UIFSMAdapter.
 */
class ContextMenuManager {
public:
  /**
   * @brief Construct a ContextMenuManager
   * @param fsmAdapter Reference to the UIFSMAdapter for state access
   * @param selectionManager Reference to the SelectionManager for selection
   * state
   */
  ContextMenuManager(UIFSMAdapter &fsmAdapter,
                     SelectionManager &selectionManager);

  /**
   * @brief Destructor
   */
  ~ContextMenuManager();

  // ==========================================================================
  // Menu Building
  // ==========================================================================

  /**
   * @brief Build the canvas context menu
   * @return Complete menu structure for the canvas context
   *
   * Builds a context menu for the canvas area with options like:
   * - Create new objects
   * - Paste from clipboard
   * - View options
   */
  Menu buildCanvasMenu();

  /**
   * @brief Build the object context menu
   * @param objectId ID of the selected object
   * @return Complete menu structure for the object context
   *
   * Builds a context menu for a specific object with options like:
   * - Delete object
   * - Duplicate object
   * - Move to layer
   * - Properties
   */
  Menu buildObjectMenu(uint32_t objectId);

  /**
   * @brief Build the outliner context menu
   * @return Complete menu structure for the outliner context
   *
   * Builds a context menu for the outliner panel with options like:
   * - Create new object
   * - Delete selected
   * - Show/hide
   * - Rename
   */
  Menu buildOutlinerMenu();

  // ==========================================================================
  // Menu Display
  // ==========================================================================

  /**
   * @brief Show a context menu at the specified position
   * @param type Type of context menu to show
   * @param position Screen position where the menu should appear
   * @param objectId Optional object ID for object context menus
   *
   * Builds and displays the appropriate context menu at the given position.
   * The menu will be rendered on the next ImGUI frame.
   */
  void showContextMenu(ContextMenuType type, const glm::vec2 &position,
                       uint32_t objectId = 0);

  /**
   * @brief Hide the currently visible context menu
   *
   * Closes any currently displayed context menu.
   */
  void hideContextMenu();

  /**
   * @brief Check if a context menu is currently visible
   * @return true if a context menu is visible
   */
  bool isMenuVisible() const;

  // ==========================================================================
  // Action Handling
  // ==========================================================================

  /**
   * @brief Handle a menu item action
   * @param menuItemId ID of the menu item that was selected
   * @return true if the action was handled, false otherwise
   *
   * Executes the callback associated with the given menu item ID.
   * Returns false if no such menu item exists.
   */
  bool handleMenuItemAction(const std::string &menuItemId);

  /**
   * @brief Get the current menu position
   * @return Current menu position in screen coordinates
   */
  glm::vec2 getMenuPosition() const;

  /**
   * @brief Get the current menu type
   * @return Current menu type
   */
  ContextMenuType getCurrentMenuType() const;

private:
  /// Reference to the UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Reference to the SelectionManager (no ownership)
  SelectionManager &selectionManager_;

  // UI-only operational state (transient)
  /// Whether a context menu is currently visible
  bool menuVisible_ = false;

  /// Type of the currently visible menu
  ContextMenuType currentMenuType_ = ContextMenuType::Canvas;

  /// Position where the menu is displayed
  glm::vec2 menuPosition_;

  /// Current menu being displayed
  Menu currentMenu_;

  /**
   * @brief Get the keyboard shortcut for a menu item
   * @param actionId Action ID to look up
   * @return Shortcut string or empty string if no shortcut
   *
   * Queries the ShortcutManager through UIFSMAdapter to get
   * the keyboard shortcut for a given action.
   */
  std::string getShortcutForAction(const std::string &actionId) const;

  /**
   * @brief Build a separator menu item
   * @return Menu item configured as a separator
   */
  static MenuItem buildSeparator();

  /**
   * @brief Build a sub-menu item
   * @param id Unique identifier for the sub-menu
   * @param label Display label for the sub-menu
   * @param subItems List of items in the sub-menu
   * @return Menu item configured as a sub-menu
   */
  static MenuItem buildSubMenu(const std::string &id, const std::string &label,
                               const std::vector<MenuItem> &subItems);

  /**
   * @brief Dispatch an action based on action string
   * @param action Action string to dispatch
   * @return true if the action was handled, false otherwise
   *
   * Parses the action string and dispatches to the appropriate component.
   * Action strings use dot notation (e.g., "object.delete", "view.toggleGrid").
   */
  bool dispatchAction(const std::string &action);
};

} // namespace view
