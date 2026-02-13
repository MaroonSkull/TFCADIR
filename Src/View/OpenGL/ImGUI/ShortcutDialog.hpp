#pragma once

#include <View/Shortcuts/ShortcutManager.hpp>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Dialog for customizing keyboard shortcuts
 *
 * ShortcutDialog provides a modal dialog for viewing and editing
 * keyboard shortcuts. Features include:
 * - Search/filter shortcuts
 * - Edit shortcuts with key capture
 * - Conflict detection and warning
 * - Import/Export functionality
 * - Reset to defaults
 */
class ShortcutDialog {
public:
  /**
   * @brief Construct a ShortcutDialog
   * @param shortcutManager Shared pointer to the shortcut manager
   */
  explicit ShortcutDialog(
      std::shared_ptr<ShortcutConfigManager> shortcutManager);

  /**
   * @brief Destructor
   */
  ~ShortcutDialog();

  /**
   * @brief Render the dialog
   *
   * Call this each frame to render the dialog if it's open.
   */
  void render();

  /**
   * @brief Open the dialog
   */
  void open();

  /**
   * @brief Close the dialog
   */
  void close();

  /**
   * @brief Check if the dialog is open
   * @return true if the dialog is open
   */
  bool isOpen() const;

private:
  /// Shared pointer to the shortcut manager
  std::shared_ptr<ShortcutConfigManager> shortcutManager_;

  /// Whether the dialog is open
  bool isOpen_{false};

  /// Search filter text
  char searchText_[256]{};

  /// Currently selected command index (-1 = none)
  int selectedIndex_{-1};

  /// Whether we're capturing a key for shortcut assignment
  bool isCapturingKey_{false};

  /// The command we're capturing a key for
  std::string capturingCommand_;

  /// The captured key while capturing
  int capturedKey_{0};

  /// The captured modifiers while capturing
  int capturedModifiers_{0};

  /// Conflict warning message (empty if no conflict)
  std::string conflictWarning_;

  /// Whether changes have been made
  bool hasChanges_{false};

  /// Cached list of shortcuts for display
  std::vector<ShortcutEntry> cachedShortcuts_;

  /// Filtered list indices
  std::vector<size_t> filteredIndices_;

  /// File path buffer for import/export
  char filePathBuffer_[512]{};

  /// Whether to show import file dialog
  bool showImportDialog_{false};

  /// Whether to show export file dialog
  bool showExportDialog_{false};

  // ==========================================================================
  // Rendering Methods
  // ==========================================================================

  /**
   * @brief Render the search bar
   */
  void renderSearchBar();

  /**
   * @brief Render the shortcuts table
   */
  void renderShortcutsTable();

  /**
   * @brief Render the action buttons
   */
  void renderActionButtons();

  /**
   * @brief Render the key capture popup
   */
  void renderKeyCapturePopup();

  /**
   * @brief Render the conflict warning
   */
  void renderConflictWarning();

  /**
   * @brief Render import/export dialogs
   */
  void renderFileDialogs();

  // ==========================================================================
  // Helper Methods
  // ==========================================================================

  /**
   * @brief Update the filtered list based on search text
   */
  void updateFilteredList();

  /**
   * @brief Start capturing a key for a command
   * @param command The command to capture a key for
   */
  void startKeyCapture(const std::string &command);

  /**
   * @brief Stop key capture and apply the shortcut
   */
  void stopKeyCapture();

  /**
   * @brief Cancel key capture
   */
  void cancelKeyCapture();

  /**
   * @brief Check for conflicts with the current shortcut
   * @param shortcut The shortcut to check
   * @param excludeCommand Command to exclude from check
   * @return true if there's a conflict
   */
  bool checkConflict(const ShortcutKey &shortcut,
                     const std::string &excludeCommand);

  /**
   * @brief Apply a shortcut to a command
   * @param command The command
   * @param shortcut The shortcut to apply
   */
  void applyShortcut(const std::string &command, const ShortcutKey &shortcut);

  /**
   * @brief Clear the shortcut for a command
   * @param command The command
   */
  void clearShortcut(const std::string &command);

  /**
   * @brief Reset a command to its default shortcut
   * @param command The command
   */
  void resetToDefault(const std::string &command);

  /**
   * @brief Save shortcuts to file
   */
  void saveShortcuts();

  /**
   * @brief Import shortcuts from a file
   * @param filePath Path to import from
   */
  void importShortcuts(const std::string &filePath);

  /**
   * @brief Export shortcuts to a file
   * @param filePath Path to export to
   */
  void exportShortcuts(const std::string &filePath);
};

} // namespace view
