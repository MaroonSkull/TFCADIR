#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Represents a single help topic with content
 *
 * This struct is used by HelpSystem for help content management.
 * Note: This is different from the HelpTopic in UIFSMAdapter.hpp which
 * is used for FSM state storage.
 */
struct HelpContentTopic {
  /// Unique identifier for the topic
  std::string id;

  /// Display title for the topic
  std::string title;

  /// Category the topic belongs to
  std::string category;

  /// Help content text (can contain markdown-like formatting)
  std::string content;

  /// Keywords for search functionality
  std::vector<std::string> keywords;

  /// Related topic IDs
  std::vector<std::string> relatedTopics;
};

/**
 * @brief Represents a keyboard shortcut entry for help display
 */
struct HelpShortcutEntry {
  /// Key combination (e.g., "Ctrl+N", "F1")
  std::string key;

  /// Action name
  std::string action;

  /// Description of what the shortcut does
  std::string description;

  /// Category for grouping
  std::string category;
};

/**
 * @brief Represents a tutorial step
 */
struct HelpTutorialStep {
  /// Step title
  std::string title;

  /// Step instructions
  std::string instructions;

  /// Optional hint text
  std::string hint;

  /// Whether this step is completed
  bool completed{false};
};

/**
 * @brief Represents a complete tutorial
 */
struct HelpTutorial {
  /// Tutorial ID
  std::string id;

  /// Tutorial title
  std::string title;

  /// Tutorial description
  std::string description;

  /// Tutorial steps
  std::vector<HelpTutorialStep> steps;

  /// Current step index
  size_t currentStep{0};

  /// Whether the tutorial is completed
  bool completed{false};
};

/**
 * @brief Help system manager for TFCADIR CAD application
 *
 * HelpSystem provides comprehensive help functionality including:
 * - Context-sensitive help (F1 key)
 * - Keyboard shortcut reference
 * - Tooltips system
 * - Interactive tutorials
 * - Documentation browser
 *
 * The help system is a pure view-layer component and does not modify
 * the FSM state. It reads current context from the FSM to provide
 * relevant help.
 */
class HelpSystem {
public:
  /**
   * @brief Construct a HelpSystem
   */
  HelpSystem();

  /**
   * @brief Destructor
   */
  ~HelpSystem();

  /**
   * @brief Initialize the help system
   *
   * Loads help topics from configuration file and sets up internal data.
   * Must be called before using the help system.
   *
   * @param configPath Path to the help configuration file (default:
   * "config/help.yaml")
   * @return true if initialization succeeded
   */
  bool initialize(const std::string &configPath = "config/help.yaml");

  /**
   * @brief Show help for a specific context
   *
   * Opens the help dialog and displays help relevant to the current context.
   *
   * @param context The context identifier (e.g., "tool.line", "mode.sketch")
   */
  void showHelp(const std::string &context);

  /**
   * @brief Get help text for a specific context
   *
   * Returns the help content for the given context without opening the dialog.
   *
   * @param context The context identifier
   * @return Help content string, or empty string if not found
   */
  std::string getHelpText(const std::string &context) const;

  /**
   * @brief Check if help is available for a context
   *
   * @param context The context identifier
   * @return true if help exists for the context
   */
  bool hasHelp(const std::string &context) const;

  /**
   * @brief Get all help topics
   *
   * @return Vector of all available help topics
   */
  const std::vector<HelpContentTopic> &getAllTopics() const;

  /**
   * @brief Get help topics by category
   *
   * @param category The category to filter by
   * @return Vector of topics in the category
   */
  std::vector<HelpContentTopic>
  getTopicsByCategory(const std::string &category) const;

  /**
   * @brief Search help topics
   *
   * Searches topic titles, content, and keywords for matches.
   *
   * @param query The search query
   * @return Vector of matching topics
   */
  std::vector<HelpContentTopic> searchTopics(const std::string &query) const;

  /**
   * @brief Get all keyboard shortcuts
   *
   * @return Vector of all keyboard shortcuts
   */
  const std::vector<HelpShortcutEntry> &getAllShortcuts() const;

  /**
   * @brief Get keyboard shortcuts by category
   *
   * @param category The category to filter by
   * @return Vector of shortcuts in the category
   */
  std::vector<HelpShortcutEntry>
  getShortcutsByCategory(const std::string &category) const;

  /**
   * @brief Get all tutorial IDs
   *
   * @return Vector of tutorial IDs
   */
  std::vector<std::string> getTutorialIds() const;

  /**
   * @brief Get a tutorial by ID
   *
   * @param id The tutorial ID
   * @return Pointer to the tutorial, or nullptr if not found
   */
  const HelpTutorial *getTutorial(const std::string &id) const;

  /**
   * @brief Start a tutorial
   *
   * @param id The tutorial ID
   * @return true if tutorial was started successfully
   */
  bool startTutorial(const std::string &id);

  /**
   * @brief Advance the current tutorial to the next step
   *
   * @return true if advanced successfully, false if tutorial is complete
   */
  bool advanceTutorial();

  /**
   * @brief Get the current active tutorial
   *
   * @return Pointer to the current tutorial, or nullptr if none active
   */
  const HelpTutorial *getCurrentTutorial() const;

  /**
   * @brief Check if a tutorial is currently active
   *
   * @return true if a tutorial is in progress
   */
  bool isTutorialActive() const;

  /**
   * @brief Cancel the current tutorial
   */
  void cancelTutorial();

  /**
   * @brief Get tooltip text for a UI element
   *
   * @param elementId The UI element identifier
   * @return Tooltip text, or empty string if not found
   */
  std::string getTooltip(const std::string &elementId) const;

  /**
   * @brief Register a tooltip for a UI element
   *
   * @param elementId The UI element identifier
   * @param text The tooltip text
   */
  void registerTooltip(const std::string &elementId, const std::string &text);

  /**
   * @brief Get all unique categories
   *
   * @return Vector of category names
   */
  std::vector<std::string> getCategories() const;

  /**
   * @brief Set callback for help dialog open/close
   *
   * @param callback Function to call when help dialog state changes
   */
  void setHelpDialogCallback(std::function<void(bool)> callback);

  /**
   * @brief Convert string to lowercase for case-insensitive comparison
   *
   * @param str The string to convert
   * @return Lowercase version of the input string
   */
  static std::string toLower(const std::string &str);

private:
  /// Help topics storage
  std::vector<HelpContentTopic> topics_;

  /// Keyboard shortcuts storage
  std::vector<HelpShortcutEntry> shortcuts_;

  /// Tutorials storage
  std::vector<HelpTutorial> tutorials_;

  /// Tooltip storage
  std::map<std::string, std::string> tooltips_;

  /// Context to topic mapping
  std::map<std::string, std::string> contextToTopic_;

  /// Current active tutorial index (-1 if none)
  int currentTutorialIndex_{-1};

  /// Help dialog callback
  std::function<void(bool)> helpDialogCallback_;

  /// Whether the system is initialized
  bool initialized_{false};

  // ==========================================================================
  // Loading Methods
  // ==========================================================================

  /// Note: YAML loading is not implemented. Help content uses hardcoded
  /// defaults. To enable customization via config/help.yaml, integrate a
  /// YAML parser (e.g., yaml-cpp) and implement loadFromYaml().

  /**
   * @brief Load default help topics
   *
   * Called during initialization to populate help content.
   */
  void loadDefaults();

  /**
   * @brief Load default keyboard shortcuts
   */
  void loadDefaultShortcuts();

  /**
   * @brief Load default tutorials
   */
  void loadDefaultTutorials();

  /**
   * @brief Load default tooltips
   */
  void loadDefaultTooltips();

  /**
   * @brief Build context to topic mapping
   */
  void buildContextMap();
};

} // namespace view
