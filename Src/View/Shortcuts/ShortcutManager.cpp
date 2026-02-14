#include "ShortcutManager.hpp"
#include <fstream>
#include <imgui.h>
#include <spdlog/spdlog.h>

namespace view {

// ==========================================================================
// Helper Functions
// ==========================================================================

/**
 * @brief Convert ImGui key code to key name string
 * @param key The ImGui key code
 * @return The key name string
 */
static std::string keyToString(int key) {
  // Map ImGui key codes to key names
  static const std::map<int, std::string> keyNameMap = {
      {ImGuiKey_A, "A"},
      {ImGuiKey_B, "B"},
      {ImGuiKey_C, "C"},
      {ImGuiKey_D, "D"},
      {ImGuiKey_E, "E"},
      {ImGuiKey_F, "F"},
      {ImGuiKey_G, "G"},
      {ImGuiKey_H, "H"},
      {ImGuiKey_I, "I"},
      {ImGuiKey_J, "J"},
      {ImGuiKey_K, "K"},
      {ImGuiKey_L, "L"},
      {ImGuiKey_M, "M"},
      {ImGuiKey_N, "N"},
      {ImGuiKey_O, "O"},
      {ImGuiKey_P, "P"},
      {ImGuiKey_Q, "Q"},
      {ImGuiKey_R, "R"},
      {ImGuiKey_S, "S"},
      {ImGuiKey_T, "T"},
      {ImGuiKey_U, "U"},
      {ImGuiKey_V, "V"},
      {ImGuiKey_W, "W"},
      {ImGuiKey_X, "X"},
      {ImGuiKey_Y, "Y"},
      {ImGuiKey_Z, "Z"},
      {ImGuiKey_0, "0"},
      {ImGuiKey_1, "1"},
      {ImGuiKey_2, "2"},
      {ImGuiKey_3, "3"},
      {ImGuiKey_4, "4"},
      {ImGuiKey_5, "5"},
      {ImGuiKey_6, "6"},
      {ImGuiKey_7, "7"},
      {ImGuiKey_8, "8"},
      {ImGuiKey_9, "9"},
      {ImGuiKey_F1, "F1"},
      {ImGuiKey_F2, "F2"},
      {ImGuiKey_F3, "F3"},
      {ImGuiKey_F4, "F4"},
      {ImGuiKey_F5, "F5"},
      {ImGuiKey_F6, "F6"},
      {ImGuiKey_F7, "F7"},
      {ImGuiKey_F8, "F8"},
      {ImGuiKey_F9, "F9"},
      {ImGuiKey_F10, "F10"},
      {ImGuiKey_F11, "F11"},
      {ImGuiKey_F12, "F12"},
      {ImGuiKey_Delete, "Delete"},
      {ImGuiKey_Escape, "Escape"},
      {ImGuiKey_Enter, "Enter"},
      {ImGuiKey_Space, "Space"},
      {ImGuiKey_Tab, "Tab"},
      {ImGuiKey_Backspace, "Backspace"},
      {ImGuiKey_Insert, "Insert"},
      {ImGuiKey_Home, "Home"},
      {ImGuiKey_End, "End"},
      {ImGuiKey_PageUp, "PageUp"},
      {ImGuiKey_PageDown, "PageDown"},
      {ImGuiKey_Equal, "Equal"},
      {ImGuiKey_Minus, "Minus"},
      {ImGuiKey_LeftArrow, "LeftArrow"},
      {ImGuiKey_RightArrow, "RightArrow"},
      {ImGuiKey_UpArrow, "UpArrow"},
      {ImGuiKey_DownArrow, "DownArrow"},
  };

  auto it = keyNameMap.find(key);
  if (it != keyNameMap.end()) {
    return it->second;
  }

  // Return as number if not found
  return std::to_string(key);
}

// ==========================================================================
// ShortcutKey Implementation
// ==========================================================================

std::string ShortcutKey::toString() const {
  std::string result;

  // Add modifiers in standard order
  if (modifiers & 1) { // Ctrl
    result += "Ctrl+";
  }
  if (modifiers & 2) { // Shift
    result += "Shift+";
  }
  if (modifiers & 4) { // Alt
    result += "Alt+";
  }

  // Add key name
  result += keyToString(key);

  return result;
}

// ==========================================================================
// ShortcutConfigManager Implementation
// ==========================================================================

ShortcutConfigManager::ShortcutConfigManager(const std::string &configPath)
    : configPath_(configPath) {
  // Initialize default shortcuts first
  initializeDefaults();

  // Copy defaults to current shortcuts
  shortcuts_ = defaultShortcuts_;

  // Rebuild the assigned shortcuts set
  rebuildAssignedSet();

  // Try to load from config file (will use defaults if file doesn't exist)
  loadShortcuts();
}

ShortcutConfigManager::~ShortcutConfigManager() = default;

// ==========================================================================
// Shortcut Loading/Saving
// ==========================================================================

bool ShortcutConfigManager::loadShortcuts() {
  // Check if config file exists
  if (!std::filesystem::exists(configPath_)) {
    spdlog::info("Shortcut config file not found: {}, using defaults",
                 configPath_.string());
    return true; // Use defaults
  }

  try {
    std::ifstream file(configPath_);
    if (!file.is_open()) {
      spdlog::error("Failed to open shortcut config file: {}",
                    configPath_.string());
      return false;
    }

    // Simple YAML parsing for shortcuts
    std::string line;
    std::string currentCommand;
    int currentKey = 0;
    int currentModifiers = 0;

    while (std::getline(file, line)) {
      // Skip empty lines and comments
      if (line.empty() || line[0] == '#') {
        continue;
      }

      // Check for command start (e.g., "  line:")
      if (line.find("    key:") == 0) {
        // Parse key value
        std::string keyStr = line.substr(9);
        // Remove quotes if present
        if (!keyStr.empty() && keyStr.front() == '"') {
          keyStr = keyStr.substr(1);
        }
        if (!keyStr.empty() && keyStr.back() == '"') {
          keyStr = keyStr.substr(0, keyStr.length() - 1);
        }
        currentKey = parseKeyName(keyStr);
      } else if (line.find("    modifiers:") == 0) {
        // Parse modifiers value
        std::string modStr = line.substr(15);
        currentModifiers = std::stoi(modStr);
      } else if (line.find("shortcuts:") == 0) {
        // Start of shortcuts section
        continue;
      } else if (line.find("  ") == 0 && line.find(':') != std::string::npos) {
        // This is a command line (e.g., "  line:")
        // If we have a previous command with key/modifiers, save it
        if (!currentCommand.empty() && currentKey != 0) {
          auto it = shortcuts_.find(currentCommand);
          if (it != shortcuts_.end()) {
            it->second.shortcut = ShortcutKey(currentKey, currentModifiers);
          }
        }

        // Parse new command
        currentCommand = line.substr(2);
        size_t colonPos = currentCommand.find(':');
        if (colonPos != std::string::npos) {
          currentCommand = currentCommand.substr(0, colonPos);
        }
        currentKey = 0;
        currentModifiers = 0;
      }
    }

    // Save last command if any
    if (!currentCommand.empty() && currentKey != 0) {
      auto it = shortcuts_.find(currentCommand);
      if (it != shortcuts_.end()) {
        it->second.shortcut = ShortcutKey(currentKey, currentModifiers);
      }
    }

    // Rebuild assigned shortcuts set
    rebuildAssignedSet();

    spdlog::info("Loaded shortcuts from: {}", configPath_.string());
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Error loading shortcuts: {}", e.what());
    return false;
  }
}

bool ShortcutConfigManager::saveShortcuts() {
  try {
    // Create parent directories if they don't exist
    if (configPath_.has_parent_path()) {
      std::filesystem::create_directories(configPath_.parent_path());
    }

    std::ofstream file(configPath_);
    if (!file.is_open()) {
      spdlog::error("Failed to open shortcut config file for writing: {}",
                    configPath_.string());
      return false;
    }

    // Write YAML header
    file << "# TFCADIR Keyboard Shortcuts Configuration\n";
    file << "# This file can be edited manually or through the Shortcuts "
            "Dialog\n\n";
    file << "shortcuts:\n";

    // Write each shortcut
    for (const auto &[id, entry] : shortcuts_) {
      file << "  " << id << ":\n";
      file << "    key: \"" << keyToString(entry.shortcut.key) << "\"\n";
      file << "    modifiers: " << entry.shortcut.modifiers << "\n";
    }

    spdlog::info("Saved shortcuts to: {}", configPath_.string());
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Error saving shortcuts: {}", e.what());
    return false;
  }
}

// ==========================================================================
// Shortcut Access
// ==========================================================================

ShortcutKey
ShortcutConfigManager::getShortcut(const std::string &command) const {
  auto it = shortcuts_.find(command);
  if (it != shortcuts_.end()) {
    return it->second.shortcut;
  }
  return ShortcutKey();
}

bool ShortcutConfigManager::setShortcut(const std::string &command,
                                        const ShortcutKey &shortcut) {
  auto it = shortcuts_.find(command);
  if (it == shortcuts_.end()) {
    spdlog::warn("Attempted to set shortcut for unknown command: {}", command);
    return false;
  }

  // Remove old shortcut from assigned set
  if (assignedShortcuts_.count(it->second.shortcut) > 0) {
    assignedShortcuts_.erase(it->second.shortcut);
  }

  // Set new shortcut
  it->second.shortcut = shortcut;

  // Add new shortcut to assigned set
  if (shortcut.key != 0) {
    assignedShortcuts_.insert(shortcut);
  }

  spdlog::debug("Set shortcut for '{}': {}", command, shortcut.toString());
  return true;
}

void ShortcutConfigManager::clearShortcut(const std::string &command) {
  auto it = shortcuts_.find(command);
  if (it != shortcuts_.end()) {
    // Remove from assigned set
    if (assignedShortcuts_.count(it->second.shortcut) > 0) {
      assignedShortcuts_.erase(it->second.shortcut);
    }
    // Clear the shortcut
    it->second.shortcut = ShortcutKey();
  }
}

std::vector<ShortcutEntry> ShortcutConfigManager::getAllShortcuts() const {
  std::vector<ShortcutEntry> result;
  result.reserve(shortcuts_.size());
  for (const auto &[id, entry] : shortcuts_) {
    result.push_back(entry);
  }
  return result;
}

std::vector<ShortcutEntry> ShortcutConfigManager::getDefaultShortcuts() const {
  std::vector<ShortcutEntry> result;
  result.reserve(defaultShortcuts_.size());
  for (const auto &[id, entry] : defaultShortcuts_) {
    result.push_back(entry);
  }
  return result;
}

// ==========================================================================
// Conflict Detection
// ==========================================================================

bool ShortcutConfigManager::hasConflict(
    const ShortcutKey &shortcut, const std::string &excludeCommand) const {
  if (shortcut.key == 0) {
    return false; // No shortcut = no conflict
  }

  for (const auto &[id, entry] : shortcuts_) {
    if (id == excludeCommand) {
      continue;
    }
    if (entry.shortcut == shortcut) {
      return true;
    }
  }
  return false;
}

std::string ShortcutConfigManager::getConflictCommand(
    const ShortcutKey &shortcut, const std::string &excludeCommand) const {
  if (shortcut.key == 0) {
    return "";
  }

  for (const auto &[id, entry] : shortcuts_) {
    if (id == excludeCommand) {
      continue;
    }
    if (entry.shortcut == shortcut) {
      return entry.displayName;
    }
  }
  return "";
}

// ==========================================================================
// Reset
// ==========================================================================

void ShortcutConfigManager::resetToDefaults() {
  shortcuts_ = defaultShortcuts_;
  rebuildAssignedSet();
  spdlog::info("Reset all shortcuts to defaults");
}

void ShortcutConfigManager::resetToDefault(const std::string &command) {
  auto it = shortcuts_.find(command);
  auto defaultIt = defaultShortcuts_.find(command);

  if (it != shortcuts_.end() && defaultIt != defaultShortcuts_.end()) {
    // Remove old shortcut from assigned set
    if (assignedShortcuts_.count(it->second.shortcut) > 0) {
      assignedShortcuts_.erase(it->second.shortcut);
    }

    // Reset to default
    it->second.shortcut = defaultIt->second.shortcut;

    // Add default shortcut to assigned set
    if (it->second.shortcut.key != 0) {
      assignedShortcuts_.insert(it->second.shortcut);
    }

    spdlog::debug("Reset shortcut '{}' to default: {}", command,
                  it->second.shortcut.toString());
  }
}

// ==========================================================================
// Import/Export
// ==========================================================================

bool ShortcutConfigManager::importFromFile(const std::string &filePath) {
  try {
    std::filesystem::path importPath(filePath);

    // Check if file exists
    if (!std::filesystem::exists(importPath)) {
      spdlog::error("Import file not found: {}", filePath);
      return false;
    }

    // Copy file to config path
    std::filesystem::copy_file(
        importPath, configPath_,
        std::filesystem::copy_options::overwrite_existing);

    // Reload shortcuts
    return loadShortcuts();
  } catch (const std::exception &e) {
    spdlog::error("Error importing shortcuts: {}", e.what());
    return false;
  }
}

bool ShortcutConfigManager::exportToFile(const std::string &filePath) {
  try {
    std::filesystem::path exportPath(filePath);

    // Create parent directories if they don't exist
    if (exportPath.has_parent_path()) {
      std::filesystem::create_directories(exportPath.parent_path());
    }

    std::ofstream file(exportPath);
    if (!file.is_open()) {
      spdlog::error("Failed to open export file: {}", filePath);
      return false;
    }

    // Write YAML header
    file << "# TFCADIR Keyboard Shortcuts Export\n";
    file << "# Generated by TFCADIR Shortcuts Dialog\n\n";
    file << "shortcuts:\n";

    // Write each shortcut
    for (const auto &[id, entry] : shortcuts_) {
      file << "  " << id << ":\n";
      file << "    key: \"" << keyToString(entry.shortcut.key) << "\"\n";
      file << "    modifiers: " << entry.shortcut.modifiers << "\n";
    }

    spdlog::info("Exported shortcuts to: {}", filePath);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Error exporting shortcuts: {}", e.what());
    return false;
  }
}

// ==========================================================================
// Private Methods
// ==========================================================================

void ShortcutConfigManager::initializeDefaults() {
  // ==========================================================================
  // Drawing Tools (SolidWorks-inspired)
  // ==========================================================================
  defaultShortcuts_["select"] =
      ShortcutEntry("select", "Select Tool", ShortcutKey(ImGuiKey_Q, 0));
  defaultShortcuts_["line"] =
      ShortcutEntry("line", "Line", ShortcutKey(ImGuiKey_L, 0));
  defaultShortcuts_["rectangle"] =
      ShortcutEntry("rectangle", "Rectangle", ShortcutKey(ImGuiKey_R, 0));
  defaultShortcuts_["circle"] =
      ShortcutEntry("circle", "Circle", ShortcutKey(ImGuiKey_C, 0));
  defaultShortcuts_["polygon"] =
      ShortcutEntry("polygon", "Polygon", ShortcutKey(ImGuiKey_P, 0));
  defaultShortcuts_["triangle"] =
      ShortcutEntry("triangle", "Triangle", ShortcutKey(ImGuiKey_T, 0));
  defaultShortcuts_["arc"] =
      ShortcutEntry("arc", "Arc", ShortcutKey(ImGuiKey_A, 0));
  defaultShortcuts_["ellipse"] =
      ShortcutEntry("ellipse", "Ellipse", ShortcutKey(ImGuiKey_E, 0));
  defaultShortcuts_["spline"] =
      ShortcutEntry("spline", "Spline", ShortcutKey(ImGuiKey_S, 0));

  // ==========================================================================
  // Modify Tools
  // ==========================================================================
  defaultShortcuts_["move"] =
      ShortcutEntry("move", "Move", ShortcutKey(ImGuiKey_M, 0));
  defaultShortcuts_["rotate"] =
      ShortcutEntry("rotate", "Rotate", ShortcutKey(ImGuiKey_R, 2)); // Shift+R
  defaultShortcuts_["scale"] =
      ShortcutEntry("scale", "Scale", ShortcutKey(ImGuiKey_X, 0));
  defaultShortcuts_["mirror"] =
      ShortcutEntry("mirror", "Mirror", ShortcutKey(ImGuiKey_M, 2)); // Shift+M
  defaultShortcuts_["fillet"] =
      ShortcutEntry("fillet", "Fillet", ShortcutKey(ImGuiKey_F, 0));
  defaultShortcuts_["delete"] =
      ShortcutEntry("delete", "Delete", ShortcutKey(ImGuiKey_Delete, 0));

  // ==========================================================================
  // Annotation Tools
  // ==========================================================================
  defaultShortcuts_["dimension"] =
      ShortcutEntry("dimension", "Dimension", ShortcutKey(ImGuiKey_D, 0));
  defaultShortcuts_["measure"] =
      ShortcutEntry("measure", "Measure", ShortcutKey(ImGuiKey_M, 6)); // Alt+M

  // ==========================================================================
  // Edit Operations
  // ==========================================================================
  defaultShortcuts_["undo"] =
      ShortcutEntry("undo", "Undo", ShortcutKey(ImGuiKey_Z, 1)); // Ctrl+Z
  defaultShortcuts_["redo"] =
      ShortcutEntry("redo", "Redo", ShortcutKey(ImGuiKey_Y, 1)); // Ctrl+Y
  defaultShortcuts_["copy"] =
      ShortcutEntry("copy", "Copy", ShortcutKey(ImGuiKey_C, 1)); // Ctrl+C
  defaultShortcuts_["paste"] =
      ShortcutEntry("paste", "Paste", ShortcutKey(ImGuiKey_V, 1)); // Ctrl+V
  defaultShortcuts_["cut"] =
      ShortcutEntry("cut", "Cut", ShortcutKey(ImGuiKey_X, 1)); // Ctrl+X
  defaultShortcuts_["duplicate"] = ShortcutEntry(
      "duplicate", "Duplicate", ShortcutKey(ImGuiKey_D, 1)); // Ctrl+D

  // ==========================================================================
  // View Operations
  // ==========================================================================
  defaultShortcuts_["mode_2d"] =
      ShortcutEntry("mode_2d", "2D Mode", ShortcutKey(ImGuiKey_F2, 0));
  defaultShortcuts_["mode_3d"] =
      ShortcutEntry("mode_3d", "3D Mode", ShortcutKey(ImGuiKey_F3, 0));
  defaultShortcuts_["zoom_in"] =
      ShortcutEntry("zoom_in", "Zoom In", ShortcutKey(ImGuiKey_Equal, 1));
  defaultShortcuts_["zoom_out"] =
      ShortcutEntry("zoom_out", "Zoom Out", ShortcutKey(ImGuiKey_Minus, 1));
  defaultShortcuts_["zoom_fit"] = ShortcutEntry(
      "zoom_fit", "Zoom to Fit", ShortcutKey(ImGuiKey_F, 1)); // Ctrl+F
  defaultShortcuts_["toggle_grid"] =
      ShortcutEntry("toggle_grid", "Toggle Grid", ShortcutKey(ImGuiKey_G, 0));
  defaultShortcuts_["toggle_snap"] = ShortcutEntry(
      "toggle_snap", "Toggle Snap", ShortcutKey(ImGuiKey_G, 2)); // Shift+G

  // ==========================================================================
  // File Operations
  // ==========================================================================
  defaultShortcuts_["new"] =
      ShortcutEntry("new", "New Project", ShortcutKey(ImGuiKey_N, 1)); // Ctrl+N
  defaultShortcuts_["open"] = ShortcutEntry(
      "open", "Open Project", ShortcutKey(ImGuiKey_O, 1)); // Ctrl+O
  defaultShortcuts_["save"] = ShortcutEntry(
      "save", "Save Project", ShortcutKey(ImGuiKey_S, 1)); // Ctrl+S
  defaultShortcuts_["save_as"] = ShortcutEntry(
      "save_as", "Save As", ShortcutKey(ImGuiKey_S, 3)); // Ctrl+Shift+S

  // ==========================================================================
  // Help and Tools
  // ==========================================================================
  defaultShortcuts_["help"] =
      ShortcutEntry("help", "Documentation", ShortcutKey(ImGuiKey_F1, 0));
  defaultShortcuts_["shortcuts_dialog"] = ShortcutEntry(
      "shortcuts_dialog", "Keyboard Shortcuts", ShortcutKey(ImGuiKey_F9, 0));

  // ==========================================================================
  // Misc
  // ==========================================================================
  defaultShortcuts_["escape"] = ShortcutEntry("escape", "Cancel/Deselect",
                                              ShortcutKey(ImGuiKey_Escape, 0));
}

void ShortcutConfigManager::rebuildAssignedSet() {
  assignedShortcuts_.clear();
  for (const auto &[id, entry] : shortcuts_) {
    if (entry.shortcut.key != 0) {
      assignedShortcuts_.insert(entry.shortcut);
    }
  }
}

int ShortcutConfigManager::parseKeyName(const std::string &keyName) const {
  // Map common key names to ImGui key codes
  static const std::map<std::string, int> keyMap = {
      {"A", ImGuiKey_A},
      {"B", ImGuiKey_B},
      {"C", ImGuiKey_C},
      {"D", ImGuiKey_D},
      {"E", ImGuiKey_E},
      {"F", ImGuiKey_F},
      {"G", ImGuiKey_G},
      {"H", ImGuiKey_H},
      {"I", ImGuiKey_I},
      {"J", ImGuiKey_J},
      {"K", ImGuiKey_K},
      {"L", ImGuiKey_L},
      {"M", ImGuiKey_M},
      {"N", ImGuiKey_N},
      {"O", ImGuiKey_O},
      {"P", ImGuiKey_P},
      {"Q", ImGuiKey_Q},
      {"R", ImGuiKey_R},
      {"S", ImGuiKey_S},
      {"T", ImGuiKey_T},
      {"U", ImGuiKey_U},
      {"V", ImGuiKey_V},
      {"W", ImGuiKey_W},
      {"X", ImGuiKey_X},
      {"Y", ImGuiKey_Y},
      {"Z", ImGuiKey_Z},
      {"0", ImGuiKey_0},
      {"1", ImGuiKey_1},
      {"2", ImGuiKey_2},
      {"3", ImGuiKey_3},
      {"4", ImGuiKey_4},
      {"5", ImGuiKey_5},
      {"6", ImGuiKey_6},
      {"7", ImGuiKey_7},
      {"8", ImGuiKey_8},
      {"9", ImGuiKey_9},
      {"F1", ImGuiKey_F1},
      {"F2", ImGuiKey_F2},
      {"F3", ImGuiKey_F3},
      {"F4", ImGuiKey_F4},
      {"F5", ImGuiKey_F5},
      {"F6", ImGuiKey_F6},
      {"F7", ImGuiKey_F7},
      {"F8", ImGuiKey_F8},
      {"F9", ImGuiKey_F9},
      {"F10", ImGuiKey_F10},
      {"F11", ImGuiKey_F11},
      {"F12", ImGuiKey_F12},
      {"Delete", ImGuiKey_Delete},
      {"Escape", ImGuiKey_Escape},
      {"Enter", ImGuiKey_Enter},
      {"Space", ImGuiKey_Space},
      {"Tab", ImGuiKey_Tab},
      {"Backspace", ImGuiKey_Backspace},
      {"Insert", ImGuiKey_Insert},
      {"Home", ImGuiKey_Home},
      {"End", ImGuiKey_End},
      {"PageUp", ImGuiKey_PageUp},
      {"PageDown", ImGuiKey_PageDown},
      {"Equal", ImGuiKey_Equal},
      {"Minus", ImGuiKey_Minus},
      {"LeftArrow", ImGuiKey_LeftArrow},
      {"RightArrow", ImGuiKey_RightArrow},
      {"UpArrow", ImGuiKey_UpArrow},
      {"DownArrow", ImGuiKey_DownArrow},
  };

  auto it = keyMap.find(keyName);
  if (it != keyMap.end()) {
    return it->second;
  }

  // Try to parse as number (direct ImGui key code)
  try {
    return std::stoi(keyName);
  } catch (...) {
    spdlog::warn("Unknown key name: {}", keyName);
    return 0;
  }
}

} // namespace view
