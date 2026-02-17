#include "ShortcutManager.hpp"
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/Tools/ImGUI/CommandManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <mutex>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

/**
 * @brief Construct a ShortcutManager
 * @param fsmAdapter Reference to the UIFSMAdapter for state access
 */
ShortcutManager::ShortcutManager(UIFSMAdapter &fsmAdapter)
    : fsmAdapter_(fsmAdapter) {
  // Initialize with default shortcuts
  // Default undo/redo shortcuts will be registered by GUI during initialization
}

/**
 * @brief Destructor
 */
ShortcutManager::~ShortcutManager() = default;

// ==========================================================================
// Shortcut Registration
// ==========================================================================

/**
 * @brief Register a new keyboard shortcut
 */
void ShortcutManager::registerShortcut(const std::string &id, KeyCombo keyCombo,
                                       const std::string &action,
                                       const std::string &description) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Get current shortcut settings from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  // Create shortcut
  Shortcut shortcut;
  shortcut.id = id;
  shortcut.keyCombo = keyCombo;
  shortcut.description = description;
  shortcut.action = action;
  shortcut.isCustomizable = true;

  // Add or update shortcut in map
  settings.shortcuts[id] = shortcut;

  // Store updated settings back to UIFSMAdapter
  fsmAdapter_.setShortcutSettings(settings);

  // Rebuild the key combination to ID map
  rebuildKeyComboMap();

  spdlog::debug("Registered shortcut '{}' with key combo {} and action '{}'",
                id, keyCombo.keyCode, action);
}

/**
 * @brief Unregister a keyboard shortcut
 */
void ShortcutManager::unregisterShortcut(const std::string &id) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Get current shortcut settings from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  // Remove the shortcut from map
  if (settings.shortcuts.erase(id) > 0) {
    // Store updated settings back to UIFSMAdapter
    fsmAdapter_.setShortcutSettings(settings);

    // Rebuild the key combination to ID map
    rebuildKeyComboMap();

    spdlog::debug("Unregistered shortcut '{}'", id);
  }
}

// ==========================================================================
// Key Handling
// ==========================================================================

/**
 * @brief Handle a key press event
 */
bool ShortcutManager::handleKeyPress(int keyCode, KeyModifier modifier) {
  std::lock_guard<std::mutex> lock(mutex_);

  KeyCombo keyCombo{keyCode, modifier};

  // Find the shortcut by key combination
  auto it = keyComboToIdMap_.find(keyCombo);
  if (it == keyComboToIdMap_.end()) {
    return false; // No shortcut found for this key combination
  }

  // Get the shortcut from UIFSMAdapter
  const std::string &shortcutId = it->second;
  auto settings = fsmAdapter_.getShortcutSettings();
  auto shortcutIt = settings.shortcuts.find(shortcutId);

  if (shortcutIt == settings.shortcuts.end()) {
    return false; // Shortcut ID not found (should not happen)
  }

  // Dispatch the action
  bool handled = dispatchAction(shortcutIt->second.action);
  if (handled) {
    spdlog::debug("Shortcut '{}' triggered with action '{}'", shortcutId,
                  shortcutIt->second.action);
  }

  return handled;
}

// ==========================================================================
// Conflict Detection
// ==========================================================================

/**
 * @brief Detect all shortcut conflicts
 */
std::vector<Conflict> ShortcutManager::detectConflicts() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Conflict> conflicts;
  std::unordered_map<KeyCombo, std::vector<std::string>, KeyComboHash>
      keyComboToIds;

  // Get shortcuts from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  // Build a map of key combinations to shortcut IDs
  for (const auto &[id, shortcut] : settings.shortcuts) {
    keyComboToIds[shortcut.keyCombo].push_back(id);
  }

  // Find conflicts (key combinations with more than one shortcut)
  for (const auto &[keyCombo, ids] : keyComboToIds) {
    if (ids.size() > 1) {
      // Create conflict entries for all combinations
      for (size_t i = 0; i < ids.size(); ++i) {
        for (size_t j = i + 1; j < ids.size(); ++j) {
          Conflict conflict;

          auto s1 = settings.shortcuts.find(ids[i]);
          auto s2 = settings.shortcuts.find(ids[j]);

          if (s1 != settings.shortcuts.end() &&
              s2 != settings.shortcuts.end()) {
            conflict.shortcut1 = s1->second;
            conflict.shortcut2 = s2->second;
            conflict.description = "Shortcuts '" + conflict.shortcut1.id +
                                   "' and '" + conflict.shortcut2.id +
                                   "' share the same key combination";
            conflicts.push_back(conflict);
          }
        }
      }
    }
  }

  return conflicts;
}

/**
 * @brief Detect conflicts for a specific key combination
 */
std::vector<Conflict>
ShortcutManager::detectConflictsFor(const KeyCombo &keyCombo) const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Conflict> conflicts;
  std::vector<std::string> conflictingIds;

  // Get shortcuts from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  // Find all shortcuts with this key combination
  for (const auto &[id, shortcut] : settings.shortcuts) {
    if (shortcut.keyCombo == keyCombo) {
      conflictingIds.push_back(id);
    }
  }

  // Create conflict entries if more than one shortcut found
  if (conflictingIds.size() > 1) {
    for (size_t i = 0; i < conflictingIds.size(); ++i) {
      for (size_t j = i + 1; j < conflictingIds.size(); ++j) {
        Conflict conflict;

        auto s1 = settings.shortcuts.find(conflictingIds[i]);
        auto s2 = settings.shortcuts.find(conflictingIds[j]);

        if (s1 != settings.shortcuts.end() && s2 != settings.shortcuts.end()) {
          conflict.shortcut1 = s1->second;
          conflict.shortcut2 = s2->second;
          conflict.description = "Shortcuts '" + conflict.shortcut1.id +
                                 "' and '" + conflict.shortcut2.id +
                                 "' share the same key combination";
          conflicts.push_back(conflict);
        }
      }
    }
  }

  return conflicts;
}

// ==========================================================================
// Import/Export
// ==========================================================================

/**
 * @brief Export all shortcuts to YAML format
 */
std::string ShortcutManager::exportShortcuts() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::ostringstream yaml;

  // Get shortcuts from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  yaml << "shortcuts:\n";
  for (const auto &[id, shortcut] : settings.shortcuts) {
    if (shortcut.isCustomizable) {
      yaml << "  - id: " << id << "\n";
      yaml << "    keyCombo: " << keyComboToString(shortcut.keyCombo) << "\n";
      yaml << "    action: " << shortcut.action << "\n";
      yaml << "    description: " << shortcut.description << "\n";
    }
  }

  return yaml.str();
}

/**
 * @brief Import shortcuts from YAML configuration
 */
bool ShortcutManager::importShortcuts(const std::string &yamlConfig) {
  // TODO: Implement YAML parsing
  // For now, this is a stub that returns false
  // Full implementation will require YAML library integration
  spdlog::warn("Shortcut import not yet implemented - YAML parsing required");
  return false;
}

// ==========================================================================
// Query Methods (Stateless)
// ==========================================================================

/**
 * @brief Get all registered shortcuts
 */
std::vector<Shortcut> ShortcutManager::getAllShortcuts() const {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<Shortcut> result;

  // Get shortcuts from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  result.reserve(settings.shortcuts.size());

  for (const auto &[id, shortcut] : settings.shortcuts) {
    result.push_back(shortcut);
  }

  return result;
}

/**
 * @brief Get a shortcut by ID
 */
std::optional<Shortcut>
ShortcutManager::getShortcut(const std::string &id) const {
  std::lock_guard<std::mutex> lock(mutex_);

  // Get shortcuts from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  auto it = settings.shortcuts.find(id);

  if (it != settings.shortcuts.end()) {
    return it->second;
  }

  return std::nullopt;
}

/**
 * @brief Get a shortcut by key combination
 */
std::optional<Shortcut>
ShortcutManager::getShortcutByKeyCombo(KeyCombo keyCombo) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = keyComboToIdMap_.find(keyCombo);
  if (it != keyComboToIdMap_.end()) {
    return getShortcut(it->second);
  }

  return std::nullopt;
}

// ==========================================================================
// Private Methods
// ==========================================================================

/**
 * @brief Rebuild the key combination to ID map from UIFSMAdapter settings
 */
void ShortcutManager::rebuildKeyComboMap() {
  keyComboToIdMap_.clear();

  // Get shortcuts from UIFSMAdapter
  auto settings = fsmAdapter_.getShortcutSettings();

  for (const auto &[id, shortcut] : settings.shortcuts) {
    keyComboToIdMap_[shortcut.keyCombo] = id;
  }
}

/**
 * @brief Dispatch an action based on action string
 *
 * Parses the action string and dispatches to the appropriate component.
 * Action strings use dot notation (e.g., "commandManager.undo",
 * "tool.activate", "view.toggleGrid").
 *
 * Supported action formats:
 * - "commandManager.undo" - Undo last command
 * - "commandManager.redo" - Redo next command
 * - "tool.activate:<toolId>" - Activate a tool
 * - "view.toggleGrid" - Toggle grid visibility
 * - "view.toggleSnap" - Toggle snap enabled state
 */
bool ShortcutManager::dispatchAction(const std::string &action) {
  // Parse the action string
  std::istringstream iss(action);
  std::string component;
  std::string command;

  // Split by first dot to get component and command
  if (std::getline(iss, component, '.') && std::getline(iss, command)) {
    if (component == "commandManager") {
      // Command manager actions
      if (command == "undo") {
        // Undo through UIFSMAdapter
        if (fsmAdapter_.canUndo()) {
          fsmAdapter_.undoCommand();
          spdlog::info("Undo performed via shortcut");
          return true;
        }
      } else if (command == "redo") {
        // Redo through UIFSMAdapter
        if (fsmAdapter_.canRedo()) {
          fsmAdapter_.redoCommand();
          spdlog::info("Redo performed via shortcut");
          return true;
        }
      }
    } else if (component == "tool") {
      // Tool activation actions
      if (command.find("activate:") == 0) {
        std::string toolId = command.substr(9); // Skip "activate:"
        fsmAdapter_.activateTool(toolId);
        spdlog::info("Activated tool '{}' via shortcut", toolId);
        return true;
      }
    } else if (component == "view") {
      // View-related actions
      if (command == "toggleGrid") {
        // Toggle grid visibility
        auto settings = fsmAdapter_.getGridSettings();
        settings.visible = !settings.visible;
        fsmAdapter_.setGridSettings(settings);
        spdlog::info("Toggled grid visibility via shortcut");
        return true;
      } else if (command == "toggleSnap") {
        // Toggle snap enabled state
        auto settings = fsmAdapter_.getSnapSettings();
        settings.gridEnabled = !settings.gridEnabled;
        fsmAdapter_.setSnapSettings(settings);
        spdlog::info("Toggled snap via shortcut");
        return true;
      }
    }
  }

  spdlog::warn("Unknown action '{}'", action);
  return false;
}

/**
 * @brief Convert KeyCombo to string representation
 */
std::string ShortcutManager::keyComboToString(const KeyCombo &keyCombo) const {
  std::string result;

  switch (keyCombo.modifier) {
  case KeyModifier::Ctrl:
    result = "Ctrl+";
    break;
  case KeyModifier::Shift:
    result = "Shift+";
    break;
  case KeyModifier::Alt:
    result = "Alt+";
    break;
  case KeyModifier::CtrlShift:
    result = "Ctrl+Shift+";
    break;
  case KeyModifier::CtrlAlt:
    result = "Ctrl+Alt+";
    break;
  case KeyModifier::ShiftAlt:
    result = "Shift+Alt+";
    break;
  case KeyModifier::CtrlShiftAlt:
    result = "Ctrl+Shift+Alt+";
    break;
  default:
    break;
  }

  // Add key code (convert ImGuiKey to string)
  result += "Key" + std::to_string(keyCombo.keyCode);

  return result;
}

/**
 * @brief Convert string representation to KeyCombo
 */
KeyCombo ShortcutManager::stringToKeyCombo(const std::string &str) const {
  KeyCombo result{0, KeyModifier::None};

  std::string s = str;

  // Parse modifiers
  if (s.find("Ctrl+Shift+Alt+") == 0) {
    result.modifier = KeyModifier::CtrlShiftAlt;
    s = s.substr(14);
  } else if (s.find("Ctrl+Shift+") == 0) {
    result.modifier = KeyModifier::CtrlShift;
    s = s.substr(10);
  } else if (s.find("Ctrl+Alt+") == 0) {
    result.modifier = KeyModifier::CtrlAlt;
    s = s.substr(8);
  } else if (s.find("Shift+Alt+") == 0) {
    result.modifier = KeyModifier::ShiftAlt;
    s = s.substr(9);
  } else if (s.find("Ctrl+") == 0) {
    result.modifier = KeyModifier::Ctrl;
    s = s.substr(5);
  } else if (s.find("Shift+") == 0) {
    result.modifier = KeyModifier::Shift;
    s = s.substr(6);
  } else if (s.find("Alt+") == 0) {
    result.modifier = KeyModifier::Alt;
    s = s.substr(4);
  }

  // Parse key code (expecting "KeyN" format)
  if (s.find("Key") == 0) {
    result.keyCode = std::stoi(s.substr(3));
  }

  return result;
}

} // namespace view
