#pragma once

#include "HelpSystem.hpp"
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Dialog for displaying help content and documentation
 *
 * HelpDialog provides a comprehensive help interface including:
 * - Searchable help topics
 * - Keyboard shortcuts reference
 * - Interactive tutorials
 * - Context-sensitive help
 *
 * The dialog follows the same pattern as ShortcutDialog for consistency.
 */
class HelpDialog {
public:
  /**
   * @brief Construct a HelpDialog
   * @param helpSystem Shared pointer to the help system
   */
  explicit HelpDialog(std::shared_ptr<HelpSystem> helpSystem);

  /**
   * @brief Destructor
   */
  ~HelpDialog();

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

  /**
   * @brief Set the current help context
   *
   * When a context is set, the dialog will show relevant help
   * for that context when opened.
   *
   * @param context The context identifier
   */
  void setContext(const std::string &context);

  /**
   * @brief Open the dialog with a specific context
   *
   * @param context The context identifier
   */
  void openWithContext(const std::string &context);

  /**
   * @brief Switch to a specific tab
   *
   * @param tab Tab index (0=Topics, 1=Shortcuts, 2=Tutorials)
   */
  void switchToTab(int tab);

private:
  /// Shared pointer to the help system
  std::shared_ptr<HelpSystem> helpSystem_;

  /// Whether the dialog is open
  bool isOpen_{false};

  /// Current help context
  std::string currentContext_;

  /// Search filter text
  char searchText_[256]{};

  /// Currently selected topic index
  int selectedTopicIndex_{-1};

  /// Current tab (0=Topics, 1=Shortcuts, 2=Tutorials)
  int currentTab_{0};

  /// Selected shortcut category
  int selectedShortcutCategory_{0};

  /// Selected tutorial index
  int selectedTutorialIndex_{-1};

  /// Cached list of topics for display
  std::vector<HelpContentTopic> cachedTopics_;

  /// Cached list of filtered topics
  std::vector<size_t> filteredTopicIndices_;

  /// Cached list of shortcuts for display
  std::vector<HelpShortcutEntry> cachedShortcuts_;

  /// Cached list of shortcut categories
  std::vector<std::string> shortcutCategories_;

  /// Cached list of tutorials
  std::vector<std::string> tutorialIds_;

  /// Scroll position for content area
  float contentScrollY_{0.0f};

  /// Dialog size
  ImVec2 dialogSize_{700.0f, 500.0f};

  /// Left panel width ratio (0.0-1.0)
  float leftPanelRatio_{0.3f};

  // ==========================================================================
  // Rendering Methods
  // ==========================================================================

  /**
   * @brief Render the main dialog window
   */
  void renderDialog();

  /**
   * @brief Render the tab bar
   */
  void renderTabBar();

  /**
   * @brief Render the help topics tab
   */
  void renderTopicsTab();

  /**
   * @brief Render the keyboard shortcuts tab
   */
  void renderShortcutsTab();

  /**
   * @brief Render the tutorials tab
   */
  void renderTutorialsTab();

  /**
   * @brief Render the search bar
   */
  void renderSearchBar();

  /**
   * @brief Render the topics list (left panel)
   */
  void renderTopicsList();

  /**
   * @brief Render the topic content (right panel)
   */
  void renderTopicContent();

  /**
   * @brief Render the shortcuts table
   */
  void renderShortcutsTable();

  /**
   * @brief Render the tutorials list
   */
  void renderTutorialsList();

  /**
   * @brief Render the tutorial viewer
   */
  void renderTutorialViewer();

  /**
   * @brief Render the tutorial step
   * @param tutorial The tutorial
   * @param step The current step
   */
  void renderTutorialStep(const HelpTutorial &tutorial,
                          const HelpTutorialStep &step);

  /**
   * @brief Render the close button
   */
  void renderCloseButton();

  // ==========================================================================
  // Helper Methods
  // ==========================================================================

  /**
   * @brief Update the filtered topics list based on search text
   */
  void updateFilteredTopics();

  /**
   * @brief Select a topic by index
   * @param index The topic index in cachedTopics_
   */
  void selectTopic(int index);

  /**
   * @brief Format shortcut key for display
   * @param key The shortcut key string
   * @return Formatted string for display
   */
  std::string formatShortcutKey(const std::string &key) const;

  /**
   * @brief Get shortcut categories from the help system
   */
  void updateShortcutCategories();

  /**
   * @brief Update tutorial IDs from the help system
   */
  void updateTutorialIds();

  /**
   * @brief Render help text with basic formatting
   * @param text The help text to render
   */
  void renderHelpText(const std::string &text);

  /**
   * @brief Split text into lines
   * @param text The text to split
   * @return Vector of lines
   */
  std::vector<std::string> splitLines(const std::string &text) const;
};

} // namespace view
