#include "ContextMenuManager.hpp"
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <spdlog/spdlog.h>

namespace view {

/**
 * @brief Construct a ContextMenuManager
 * @param fsmAdapter Reference to the UIFSMAdapter for state access
 * @param selectionManager Reference to the SelectionManager for selection state
 */
ContextMenuManager::ContextMenuManager(UIFSMAdapter &fsmAdapter,
                                       SelectionManager &selectionManager)
    : fsmAdapter_(fsmAdapter), selectionManager_(selectionManager) {}

/**
 * @brief Destructor
 */
ContextMenuManager::~ContextMenuManager() = default;

// ==========================================================================
// Menu Building
// ==========================================================================

/**
 * @brief Build the canvas context menu
 */
Menu ContextMenuManager::buildCanvasMenu() {
  Menu menu("Canvas Context");

  // Create submenu
  std::vector<MenuItem> createItems;
  createItems.push_back(MenuItem(
      "create_line", "Line", getShortcutForAction("tool.activate:Line3D"),
      [this]() { dispatchAction("tool.activate:Line3D"); }));
  createItems.push_back(MenuItem(
      "create_circle", "Circle", getShortcutForAction("tool.activate:Circle3D"),
      [this]() { dispatchAction("tool.activate:Circle3D"); }));
  createItems.push_back(
      MenuItem("create_rectangle", "Rectangle",
               getShortcutForAction("tool.activate:Rectangle3D"),
               [this]() { dispatchAction("tool.activate:Rectangle3D"); }));
  createItems.push_back(
      MenuItem("create_polygon", "Polygon",
               getShortcutForAction("tool.activate:Polygon3D"),
               [this]() { dispatchAction("tool.activate:Polygon3D"); }));

  menu.items.push_back(buildSubMenu("create", "Create", createItems));

  menu.items.push_back(buildSeparator());

  // Paste action (if clipboard has content)
  menu.items.push_back(
      MenuItem("paste", "Paste", getShortcutForAction("edit.paste"), [this]() {
        // TODO: Implement paste functionality
        spdlog::info("Paste action triggered from context menu");
      }));

  menu.items.push_back(buildSeparator());

  // View options
  std::vector<MenuItem> viewItems;
  viewItems.push_back(MenuItem(
      "view_grid", "Toggle Grid", getShortcutForAction("view.toggleGrid"),
      [this]() { dispatchAction("view.toggleGrid"); }));
  viewItems.push_back(MenuItem(
      "view_snap", "Toggle Snap", getShortcutForAction("view.toggleSnap"),
      [this]() { dispatchAction("view.toggleSnap"); }));

  menu.items.push_back(buildSubMenu("view", "View", viewItems));

  return menu;
}

/**
 * @brief Build the object context menu
 */
Menu ContextMenuManager::buildObjectMenu(uint32_t objectId) {
  Menu menu("Object Context");

  // Delete action
  menu.items.push_back(
      MenuItem("delete", "Delete", getShortcutForAction("object.delete"),
               [this, objectId]() { dispatchAction("object.delete"); }));

  // Duplicate action
  menu.items.push_back(MenuItem(
      "duplicate", "Duplicate", getShortcutForAction("object.duplicate"),
      [this, objectId]() { dispatchAction("object.duplicate"); }));

  menu.items.push_back(buildSeparator());

  // Move to layer submenu
  std::vector<MenuItem> layerItems;
  layerItems.push_back(MenuItem("layer_default", "Default Layer", "", []() {
    // TODO: Implement layer assignment
    spdlog::info("Move to default layer");
  }));
  layerItems.push_back(MenuItem("layer_1", "Layer 1", "", []() {
    // TODO: Implement layer assignment
    spdlog::info("Move to layer 1");
  }));
  layerItems.push_back(MenuItem("layer_2", "Layer 2", "", []() {
    // TODO: Implement layer assignment
    spdlog::info("Move to layer 2");
  }));

  menu.items.push_back(buildSubMenu("layers", "Move to Layer", layerItems));

  menu.items.push_back(buildSeparator());

  // Properties
  menu.items.push_back(MenuItem("properties", "Properties...", "", [this]() {
    // TODO: Open properties panel
    spdlog::info("Open properties panel");
  }));

  return menu;
}

/**
 * @brief Build the outliner context menu
 */
Menu ContextMenuManager::buildOutlinerMenu() {
  Menu menu("Outliner Context");

  // Create submenu (same as canvas)
  std::vector<MenuItem> createItems;
  createItems.push_back(MenuItem(
      "create_line", "Line", getShortcutForAction("tool.activate:Line3D"),
      [this]() { dispatchAction("tool.activate:Line3D"); }));
  createItems.push_back(MenuItem(
      "create_circle", "Circle", getShortcutForAction("tool.activate:Circle3D"),
      [this]() { dispatchAction("tool.activate:Circle3D"); }));
  createItems.push_back(
      MenuItem("create_rectangle", "Rectangle",
               getShortcutForAction("tool.activate:Rectangle3D"),
               [this]() { dispatchAction("tool.activate:Rectangle3D"); }));

  menu.items.push_back(buildSubMenu("create", "Create", createItems));

  menu.items.push_back(buildSeparator());

  // Delete selected
  menu.items.push_back(
      MenuItem("delete_selected", "Delete Selected",
               getShortcutForAction("selection.delete"),
               [this]() { dispatchAction("selection.delete"); }));

  // Show/hide submenu
  std::vector<MenuItem> visibilityItems;
  visibilityItems.push_back(MenuItem("show", "Show", "", [this]() {
    // TODO: Implement show selected
    spdlog::info("Show selected objects");
  }));
  visibilityItems.push_back(MenuItem("hide", "Hide", "", [this]() {
    // TODO: Implement hide selected
    spdlog::info("Hide selected objects");
  }));
  visibilityItems.push_back(MenuItem("show_all", "Show All", "", [this]() {
    // TODO: Implement show all
    spdlog::info("Show all objects");
  }));

  menu.items.push_back(
      buildSubMenu("visibility", "Show/Hide", visibilityItems));

  menu.items.push_back(buildSeparator());

  // Rename
  menu.items.push_back(MenuItem(
      "rename", "Rename", getShortcutForAction("object.rename"), [this]() {
        // TODO: Implement rename
        spdlog::info("Rename selected object");
      }));

  return menu;
}

// ==========================================================================
// Menu Display
// ==========================================================================

/**
 * @brief Show a context menu at the specified position
 */
void ContextMenuManager::showContextMenu(ContextMenuType type,
                                         const glm::vec2 &position,
                                         uint32_t objectId) {
  // Build the appropriate menu based on type
  switch (type) {
  case ContextMenuType::Canvas:
    currentMenu_ = buildCanvasMenu();
    break;
  case ContextMenuType::Object:
    currentMenu_ = buildObjectMenu(objectId);
    break;
  case ContextMenuType::Outliner:
    currentMenu_ = buildOutlinerMenu();
    break;
  }

  // Set menu state
  currentMenuType_ = type;
  menuPosition_ = position;
  menuVisible_ = true;

  spdlog::debug("Showing context menu type {} at position ({}, {})",
                static_cast<int>(type), position.x, position.y);
}

/**
 * @brief Hide the currently visible context menu
 */
void ContextMenuManager::hideContextMenu() {
  menuVisible_ = false;
  spdlog::debug("Hiding context menu");
}

/**
 * @brief Check if a context menu is currently visible
 */
bool ContextMenuManager::isMenuVisible() const { return menuVisible_; }

// ==========================================================================
// Action Handling
// ==========================================================================

/**
 * @brief Handle a menu item action
 */
bool ContextMenuManager::handleMenuItemAction(const std::string &menuItemId) {
  // Search for the menu item in the current menu
  for (const auto &item : currentMenu_.items) {
    if (item.id == menuItemId) {
      if (item.enabled && item.action) {
        item.action();
        return true;
      }
      return false;
    }

    // Search in sub-items
    for (const auto &subItem : item.subItems) {
      if (subItem.id == menuItemId) {
        if (subItem.enabled && subItem.action) {
          subItem.action();
          return true;
        }
        return false;
      }
    }
  }

  spdlog::warn("Menu item '{}' not found", menuItemId);
  return false;
}

/**
 * @brief Get the current menu position
 */
glm::vec2 ContextMenuManager::getMenuPosition() const { return menuPosition_; }

/**
 * @brief Get the current menu type
 */
ContextMenuType ContextMenuManager::getCurrentMenuType() const {
  return currentMenuType_;
}

// ==========================================================================
// Private Methods
// ==========================================================================

/**
 * @brief Get the keyboard shortcut for a menu item
 */
std::string
ContextMenuManager::getShortcutForAction(const std::string &actionId) const {
  // Query shortcut settings from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  // Look for a shortcut with this action
  for (const auto &[id, shortcut] : settings.shortcuts) {
    if (shortcut.action == actionId) {
      // Convert KeyCombo to string representation
      // This is a simplified version - the actual implementation
      // would use ShortcutManager's keyComboToString method
      switch (shortcut.keyCombo.modifier) {
      case KeyModifier::Ctrl:
        return "Ctrl+Key" + std::to_string(shortcut.keyCombo.keyCode);
      case KeyModifier::Shift:
        return "Shift+Key" + std::to_string(shortcut.keyCombo.keyCode);
      case KeyModifier::Alt:
        return "Alt+Key" + std::to_string(shortcut.keyCombo.keyCode);
      default:
        return "Key" + std::to_string(shortcut.keyCombo.keyCode);
      }
    }
  }

  return "";
}

/**
 * @brief Build a separator menu item
 */
MenuItem ContextMenuManager::buildSeparator() {
  MenuItem separator;
  separator.id = "__separator__";
  separator.label = "";
  separator.enabled = false;
  return separator;
}

/**
 * @brief Build a sub-menu item
 */
MenuItem
ContextMenuManager::buildSubMenu(const std::string &id,
                                 const std::string &label,
                                 const std::vector<MenuItem> &subItems) {
  MenuItem menuItem;
  menuItem.id = id;
  menuItem.label = label;
  menuItem.subItems = subItems;
  menuItem.enabled = true;
  return menuItem;
}

/**
 * @brief Dispatch an action based on action string
 */
bool ContextMenuManager::dispatchAction(const std::string &action) {
  // Parse the action string
  std::istringstream iss(action);
  std::string component;
  std::string command;

  // Split by first dot to get component and command
  if (std::getline(iss, component, '.') && std::getline(iss, command)) {
    if (component == "tool") {
      // Tool activation actions
      if (command.find("activate:") == 0) {
        std::string toolId = command.substr(9); // Skip "activate:"
        fsmAdapter_.activateTool(toolId);
        spdlog::info("Activated tool '{}' via context menu", toolId);
        return true;
      }
    } else if (component == "view") {
      // View-related actions
      if (command == "toggleGrid") {
        auto settings = fsmAdapter_.getGridSettings();
        settings.visible = !settings.visible;
        fsmAdapter_.setGridSettings(settings);
        spdlog::info("Toggled grid visibility via context menu");
        return true;
      } else if (command == "toggleSnap") {
        auto settings = fsmAdapter_.getSnapSettings();
        settings.gridEnabled = !settings.gridEnabled;
        fsmAdapter_.setSnapSettings(settings);
        spdlog::info("Toggled snap via context menu");
        return true;
      }
    } else if (component == "object") {
      // Object-related actions
      if (command == "delete") {
        // TODO: Implement object deletion
        spdlog::info("Delete object via context menu");
        return true;
      } else if (command == "duplicate") {
        // TODO: Implement object duplication
        spdlog::info("Duplicate object via context menu");
        return true;
      }
    } else if (component == "selection") {
      // Selection-related actions
      if (command == "delete") {
        // TODO: Implement delete selected
        spdlog::info("Delete selected objects via context menu");
        return true;
      }
    }
  }

  spdlog::warn("Unknown action '{}'", action);
  return false;
}

} // namespace view
