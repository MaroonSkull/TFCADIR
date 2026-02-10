#pragma once

#include <View/UIFSMAdapter.hpp>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace view {

// Forward declaration
class UIFSMAdapter;

/**
 * @brief Hash function for KeyCombo to use in unordered_map
 *
 * Uses boost::hash_combine pattern for robust hash mixing to avoid collisions.
 */
struct KeyComboHash {
  std::size_t operator()(const KeyCombo &keyCombo) const noexcept {
    std::size_t seed = 0;
    std::hash<int> hasher;

    // Use boost::hash_combine pattern for better mixing
    seed ^= hasher(keyCombo.keyCode) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hasher(static_cast<int>(keyCombo.modifier)) + 0x9e3779b9 +
            (seed << 6) + (seed >> 2);

    return seed;
  }
};

/**
 * @brief Shortcut conflict information
 *
 * Represents a conflict between two shortcuts that share
 * the same key combination.
 */
struct Conflict {
  /// First shortcut in the conflict
  Shortcut shortcut1;

  /// Second shortcut in the conflict
  Shortcut shortcut2;

  /// Description of the conflict
  std::string description;
};

/**
 * @brief Manages keyboard shortcuts and action dispatching
 *
 * ShortcutManager is a stateless coordinator that registers and handles
 * keyboard shortcuts, detects conflicts, and dispatches actions through
 * the UIFSMAdapter. All shortcut settings are stored in UIFSMAdapter.
 *
 * This manager follows the Phase 7 architecture pattern:
 * - No local domain state (settings queried from UIFSMAdapter)
 * - Stateless coordinator design
 * - Action string dispatch system
 * - Thread-safe operations with mutex protection
 */
class ShortcutManager {
public:
  /**
   * @brief Construct a ShortcutManager
   * @param fsmAdapter Reference to the UIFSMAdapter for state access
   */
  explicit ShortcutManager(UIFSMAdapter &fsmAdapter);

  /**
   * @brief Destructor
   */
  ~ShortcutManager();

  // ==========================================================================
  // Shortcut Registration
  // ==========================================================================

  /**
   * @brief Register a new keyboard shortcut
   * @param id Unique identifier for the shortcut
   * @param keyCombo Key combination that triggers this shortcut
   * @param action Action string for dispatching
   * @param description Human-readable description
   *
   * Registers a new shortcut with the manager. If a shortcut with the
   * same ID already exists, it will be updated with the new key combination.
   */
  void registerShortcut(const std::string &id, KeyCombo keyCombo,
                        const std::string &action,
                        const std::string &description);

  /**
   * @brief Unregister a keyboard shortcut
   * @param id ID of the shortcut to remove
   *
   * Removes the shortcut with the given ID from the registry.
   * Does nothing if no such shortcut exists.
   */
  void unregisterShortcut(const std::string &id);

  // ==========================================================================
  // Key Handling
  // ==========================================================================

  /**
   * @brief Handle a key press event
   * @param keyCode ImGui key code that was pressed
   * @param modifier Modifier keys that were held down
   * @return true if the key press was handled by a shortcut, false otherwise
   *
   * Checks if the given key combination matches any registered shortcut
   * and dispatches the associated action if found.
   */
  bool handleKeyPress(int keyCode, KeyModifier modifier);

  // ==========================================================================
  // Conflict Detection
  // ==========================================================================

  /**
   * @brief Detect all shortcut conflicts
   * @return Vector of conflicts found
   *
   * Scans all registered shortcuts and returns a list of conflicts
   * where multiple shortcuts share the same key combination.
   */
  std::vector<Conflict> detectConflicts() const;

  /**
   * @brief Detect conflicts for a specific key combination
   * @param keyCombo Key combination to check for conflicts
   * @return Vector of conflicts found for this key combination
   *
   * Returns all shortcuts that share the given key combination.
   */
  std::vector<Conflict> detectConflictsFor(const KeyCombo &keyCombo) const;

  // ==========================================================================
  // Import/Export
  // ==========================================================================

  /**
   * @brief Export all shortcuts to YAML format
   * @return YAML string containing all shortcut configurations
   *
   * Exports all registered shortcuts in YAML format for backup
   * or sharing purposes.
   */
  std::string exportShortcuts() const;

  /**
   * @brief Import shortcuts from YAML configuration
   * @param yamlConfig YAML string containing shortcut configurations
   * @return true if import was successful, false otherwise
   *
   * Parses the YAML configuration and updates shortcuts with the
   * imported key combinations. Existing shortcuts are updated,
   * new shortcuts are registered, and unknown shortcuts are ignored.
   */
  bool importShortcuts(const std::string &yamlConfig);

  // ==========================================================================
  // Query Methods (Stateless)
  // ==========================================================================

  /**
   * @brief Get all registered shortcuts
   * @return Vector of all shortcuts
   *
   * Returns a copy of all currently registered shortcuts.
   */
  std::vector<Shortcut> getAllShortcuts() const;

  /**
   * @brief Get a shortcut by ID
   * @param id ID of the shortcut to retrieve
   * @return Optional shortcut if found, nullopt otherwise
   */
  std::optional<Shortcut> getShortcut(const std::string &id) const;

  /**
   * @brief Get a shortcut by key combination
   * @param keyCombo Key combination to search for
   * @return Optional shortcut if found, nullopt otherwise
   */
  std::optional<Shortcut> getShortcutByKeyCombo(KeyCombo keyCombo) const;

private:
  /// Reference to the UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Mutex for thread-safe access to internal state
  mutable std::mutex mutex_;

  /// Map from key combination to shortcut ID for O(1) lookup
  /// UI-only operational state (transient, cached from UIFSMAdapter)
  std::unordered_map<KeyCombo, std::string, KeyComboHash> keyComboToIdMap_;

  /**
   * @brief Rebuild the key combination to ID map from UIFSMAdapter settings
   *
   * Rebuilds the keyComboToIdMap_ by querying the UIFSMAdapter for
   * current shortcut settings. Called after shortcuts are added or removed.
   */
  void rebuildKeyComboMap();

  /**
   * @brief Dispatch an action based on action string
   * @param action Action string to dispatch
   * @return true if the action was handled, false otherwise
   *
   * Parses the action string and dispatches to the appropriate component.
   * Action strings use dot notation (e.g., "commandManager.undo",
   * "tool.activate", "view.toggleGrid").
   */
  bool dispatchAction(const std::string &action);

  /**
   * @brief Convert KeyCombo to string representation
   * @param keyCombo Key combination to convert
   * @return String representation (e.g., "Ctrl+Z", "Shift+Key5")
   */
  std::string keyComboToString(const KeyCombo &keyCombo) const;

  /**
   * @brief Convert string representation to KeyCombo
   * @param str String representation (e.g., "Ctrl+Z", "Shift+Key5")
   * @return KeyCombo object
   */
  KeyCombo stringToKeyCombo(const std::string &str) const;
};

} // namespace view
