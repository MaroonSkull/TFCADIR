#pragma once

#include <View/UIFSMAdapter.hpp>
#include <filesystem>
#include <map>
#include <set>
#include <string>

namespace view {

/**
 * @brief Key combination for keyboard shortcuts
 *
 * Represents a keyboard shortcut as a combination of a key code
 * and optional modifier flags.
 */
struct ShortcutKey {
  /// ImGui key code (e.g., ImGuiKey_L, ImGuiKey_C)
  int key;

  /// Modifier flags: Ctrl=1, Shift=2, Alt=4
  int modifiers;

  /**
   * @brief Default constructor
   */
  ShortcutKey() : key(0), modifiers(0) {}

  /**
   * @brief Constructor with key and modifiers
   */
  ShortcutKey(int k, int mods) : key(k), modifiers(mods) {}

  /**
   * @brief Equality operator
   */
  bool operator==(const ShortcutKey &other) const {
    return key == other.key && modifiers == other.modifiers;
  }

  /**
   * @brief Less than operator for std::set
   */
  bool operator<(const ShortcutKey &other) const {
    if (key != other.key)
      return key < other.key;
    return modifiers < other.modifiers;
  }

  /**
   * @brief Convert to string representation
   * @return String representation (e.g., "Ctrl+L", "Shift+R")
   */
  std::string toString() const;
};

/**
 * @brief Shortcut entry for dialog display
 *
 * Contains information about a single shortcut for display
 * in the shortcut customization dialog.
 */
struct ShortcutEntry {
  /// Command identifier (e.g., "line", "circle", "undo")
  std::string command;

  /// Display name for the command
  std::string displayName;

  /// Current shortcut key assignment
  ShortcutKey shortcut;

  /// Whether this shortcut can be customized
  bool isCustomizable;

  /**
   * @brief Default constructor
   */
  ShortcutEntry() : isCustomizable(true) {}

  /**
   * @brief Constructor with all fields
   */
  ShortcutEntry(const std::string &cmd, const std::string &name,
                const ShortcutKey &key, bool customizable = true)
      : command(cmd), displayName(name), shortcut(key),
        isCustomizable(customizable) {}
};

/**
 * @brief Manages keyboard shortcuts for the customization dialog
 *
 * ShortcutConfigManager handles loading, saving, and managing keyboard
 * shortcuts for the shortcut customization dialog. It provides:
 * - YAML configuration file loading/saving
 * - Shortcut conflict detection
 * - Default shortcuts management
 *
 * This manager is separate from the runtime ShortcutManager in Polish/
 * which handles actual keyboard event dispatching.
 */
class ShortcutConfigManager {
public:
  /**
   * @brief Construct a ShortcutConfigManager
   * @param configPath Path to the YAML configuration file
   */
  explicit ShortcutConfigManager(
      const std::string &configPath = "config/shortcuts.yaml");

  /**
   * @brief Destructor
   */
  ~ShortcutConfigManager();

  // ==========================================================================
  // Shortcut Loading/Saving
  // ==========================================================================

  /**
   * @brief Load shortcuts from the YAML configuration file
   * @return true if loading was successful, false otherwise
   *
   * Loads shortcut definitions from the YAML config file.
   * If the file doesn't exist, default shortcuts are used.
   */
  bool loadShortcuts();

  /**
   * @brief Save shortcuts to the YAML configuration file
   * @return true if saving was successful, false otherwise
   *
   * Saves all current shortcuts to the YAML config file.
   */
  bool saveShortcuts();

  // ==========================================================================
  // Shortcut Access
  // ==========================================================================

  /**
   * @brief Get the shortcut for a command
   * @param command The command identifier
   * @return The shortcut key, or default ShortcutKey if not found
   */
  ShortcutKey getShortcut(const std::string &command) const;

  /**
   * @brief Set the shortcut for a command
   * @param command The command identifier
   * @param shortcut The new shortcut key
   * @return true if the shortcut was set successfully
   *
   * Sets the shortcut for the given command. If the shortcut
   * conflicts with an existing shortcut, the conflict is detected
   * but the shortcut is still set.
   */
  bool setShortcut(const std::string &command, const ShortcutKey &shortcut);

  /**
   * @brief Clear the shortcut for a command
   * @param command The command identifier
   */
  void clearShortcut(const std::string &command);

  /**
   * @brief Get all shortcut entries
   * @return Vector of all shortcut entries
   */
  std::vector<ShortcutEntry> getAllShortcuts() const;

  /**
   * @brief Get all default shortcut entries
   * @return Vector of default shortcut entries
   */
  std::vector<ShortcutEntry> getDefaultShortcuts() const;

  // ==========================================================================
  // Conflict Detection
  // ==========================================================================

  /**
   * @brief Check if a shortcut conflicts with existing shortcuts
   * @param shortcut The shortcut to check
   * @param excludeCommand Command to exclude from conflict check
   * @return true if there is a conflict
   */
  bool hasConflict(const ShortcutKey &shortcut,
                   const std::string &excludeCommand = "") const;

  /**
   * @brief Get the command that conflicts with a shortcut
   * @param shortcut The shortcut to check
   * @param excludeCommand Command to exclude from conflict check
   * @return The conflicting command name, or empty string if no conflict
   */
  std::string getConflictCommand(const ShortcutKey &shortcut,
                                 const std::string &excludeCommand = "") const;

  // ==========================================================================
  // Reset
  // ==========================================================================

  /**
   * @brief Reset all shortcuts to their default values
   */
  void resetToDefaults();

  /**
   * @brief Reset a specific shortcut to its default value
   * @param command The command identifier
   */
  void resetToDefault(const std::string &command);

  // ==========================================================================
  // Import/Export
  // ==========================================================================

  /**
   * @brief Import shortcuts from a file
   * @param filePath Path to the file to import
   * @return true if import was successful
   */
  bool importFromFile(const std::string &filePath);

  /**
   * @brief Export shortcuts to a file
   * @param filePath Path to export to
   * @return true if export was successful
   */
  bool exportToFile(const std::string &filePath);

private:
  /// Path to the YAML configuration file
  std::filesystem::path configPath_;

  /// Map of command ID to shortcut entry
  std::map<std::string, ShortcutEntry> shortcuts_;

  /// Map of command ID to default shortcut entry
  std::map<std::string, ShortcutEntry> defaultShortcuts_;

  /// Set of assigned shortcuts for conflict detection
  std::set<ShortcutKey> assignedShortcuts_;

  /**
   * @brief Initialize default shortcuts
   *
   * Populates the defaultShortcuts_ map with default shortcut definitions.
   */
  void initializeDefaults();

  /**
   * @brief Rebuild the assigned shortcuts set
   *
   * Rebuilds the assignedShortcuts_ set from the current shortcuts.
   */
  void rebuildAssignedSet();

  /**
   * @brief Parse a key name string to ImGui key code
   * @param keyName The key name (e.g., "L", "C", "Z")
   * @return The ImGui key code
   */
  int parseKeyName(const std::string &keyName) const;
};

} // namespace view
