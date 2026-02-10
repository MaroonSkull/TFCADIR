#include "ShortcutManager.hpp"
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/Tools/ImGUI/CommandManager.hpp>
#include <View/UIFSMAdapter.hpp>
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
  // Create or update the shortcut
  Shortcut shortcut{id, keyCombo, description, action, true};

  // Store the shortcut
  shortcuts_[id] = shortcut;

  // Rebuild the key combination to ID map
  rebuildKeyComboMap();

  spdlog::debug("Registered shortcut '{}' with key combo {} and action '{}'",
                id, keyCombo.keyCode, action);
}

/**
 * @brief Unregister a keyboard shortcut
 */
void ShortcutManager::unregisterShortcut(const std::string &id) {
  auto it = shortcuts_.find(id);
  if (it != shortcuts_.end()) {
    shortcuts_.erase(it);
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
  KeyCombo keyCombo{keyCode, modifier};

  // Find the shortcut by key combination
  auto it = keyComboToIdMap_.find(keyCombo);
  if (it == keyComboToIdMap_.end()) {
    return false; // No shortcut found for this key combination
  }

  // Get the shortcut
  const std::string &shortcutId = it->second;
  auto shortcutIt = shortcuts_.find(shortcutId);
  if (shortcutIt == shortcuts_.end()) {
    return false; // Shortcut ID not found (should not happen)
  }

  const Shortcut &shortcut = shortcutIt->second;

  // Dispatch the action
  bool handled = dispatchAction(shortcut.action);
  if (handled) {
    spdlog::debug("Shortcut '{}' triggered with action '{}'", shortcutId,
                  shortcut.action);
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
  std::vector<Conflict> conflicts;
  std::unordered_map<KeyCombo, std::vector<std::string>, KeyComboHash>
      keyComboToIds;

  // Build a map of key combinations to shortcut IDs
  for (const auto &[id, shortcut] : shortcuts_) {
    keyComboToIds[shortcut.keyCombo].push_back(id);
  }

  // Find conflicts (key combinations with more than one shortcut)
  for (const auto &[keyCombo, ids] : keyComboToIds) {
    if (ids.size() > 1) {
      // Create conflict entries for all combinations
      for (size_t i = 0; i < ids.size(); ++i) {
        for (size_t j = i + 1; j < ids.size(); ++j) {
          Conflict conflict;
          conflict.shortcut1 = shortcuts_.at(ids[i]);
          conflict.shortcut2 = shortcuts_.at(ids[j]);
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

/**
 * @brief Detect conflicts for a specific key combination
 */
std::vector<Conflict>
ShortcutManager::detectConflictsFor(const KeyCombo &keyCombo) const {
  std::vector<Conflict> conflicts;
  std::vector<std::string> conflictingIds;

  // Find all shortcuts with this key combination
  for (const auto &[id, shortcut] : shortcuts_) {
    if (shortcut.keyCombo == keyCombo) {
      conflictingIds.push_back(id);
    }
  }

  // Create conflict entries if more than one shortcut found
  if (conflictingIds.size() > 1) {
    for (size_t i = 0; i < conflictingIds.size(); ++i) {
      for (size_t j = i + 1; j < conflictingIds.size(); ++j) {
        Conflict conflict;
        conflict.shortcut1 = shortcuts_.at(conflictingIds[i]);
        conflict.shortcut2 = shortcuts_.at(conflictingIds[j]);
        conflict.description = "Shortcuts '" + conflict.shortcut1.id +
                               "' and '" + conflict.shortcut2.id +
                               "' share the same key combination";
        conflicts.push_back(conflict);
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
  std::ostringstream yaml;

  yaml << "shortcuts:\n";
  for (const auto &[id, shortcut] : shortcuts_) {
    if (shortcut.isCustomizable) {
      yaml << "  - id: " << id << "\n";
      yaml << "    keyCode: " << shortcut.keyCombo.keyCode << "\n";
      yaml << "    modifier: " << static_cast<int>(shortcut.keyCombo.modifier)
           << "\n";
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
  std::vector<Shortcut> result;
  result.reserve(shortcuts_.size());

  for (const auto &[id, shortcut] : shortcuts_) {
    result.push_back(shortcut);
  }

  return result;
}

/**
 * @brief Get a shortcut by ID
 */
std::optional<Shortcut>
ShortcutManager::getShortcut(const std::string &id) const {
  auto it = shortcuts_.find(id);
  if (it != shortcuts_.end()) {
    return it->second;
  }
  return std::nullopt;
}

/**
 * @brief Get a shortcut by key combination
 */
std::optional<Shortcut>
ShortcutManager::getShortcutByKeyCombo(KeyCombo keyCombo) const {
  auto it = keyComboToIdMap_.find(keyCombo);
  if (it != keyComboToIdMap_.end()) {
    auto shortcutIt = shortcuts_.find(it->second);
    if (shortcutIt != shortcuts_.end()) {
      return shortcutIt->second;
    }
  }
  return std::nullopt;
}

// ==========================================================================
// Private Methods
// ==========================================================================

/**
 * @brief Rebuild the key combination to ID map
 */
void ShortcutManager::rebuildKeyComboMap() {
  keyComboToIdMap_.clear();

  for (const auto &[id, shortcut] : shortcuts_) {
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

} // namespace view
