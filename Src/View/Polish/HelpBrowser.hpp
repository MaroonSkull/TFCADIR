#pragma once

#include "../UIFSMAdapter.hpp"
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace view {
namespace Polish {

/**
 * @brief Help browser for displaying help content
 *
 * Provides context-sensitive help and searchable documentation.
 * Follows stateless coordinator pattern - all state stored in UIFSMAdapter.
 */
class HelpBrowser {
public:
  explicit HelpBrowser(view::UIFSMAdapter *fsmAdapter);
  ~HelpBrowser() = default;

  // Query-based API (stateless)
  /// Use UIFSMAdapter::HelpTopic instead of duplicate definition
  using HelpTopic = view::HelpTopic;

  struct HelpSearchResult {
    HelpTopic topic;
    float relevanceScore;
  };

  /**
   * @brief Get help topic by ID
   * @param topicId Unique topic identifier
   * @return Help topic data
   */
  HelpTopic getHelpTopic(const std::string &topicId) const;

  /**
   * @brief Get help topics for category
   * @param category Category filter
   * @return Vector of help topics in category
   */
  std::vector<HelpTopic>
  getHelpTopicsByCategory(const std::string &category) const;

  /**
   * @brief Search help topics
   * @param query Search query string
   * @return Vector of search results sorted by relevance
   */
  std::vector<HelpSearchResult>
  searchHelpTopics(const std::string &query) const;

  /**
   * @brief Get context-sensitive help for current tool
   * @param toolId Current tool identifier
   * @return Help topic for tool, or empty if not found
   */
  HelpTopic getContextSensitiveHelp(const std::string &toolId) const;

  /**
   * @brief Get all categories
   * @return Vector of category names
   */
  std::vector<std::string> getCategories() const;

  /**
   * @brief Get keyboard shortcuts reference
   * @return Formatted keyboard shortcuts documentation
   */
  std::string getKeyboardShortcutsReference() const;

  /**
   * @brief Get table of contents
   * @return Hierarchical TOC structure
   */
  std::vector<std::pair<std::string, std::string>> getTableOfContents() const;

  // Settings management
  /// Use UIFSMAdapter::HelpSettings instead of duplicate definition
  using HelpSettings = view::HelpSettings;

  /**
   * @brief Get help settings
   * @return Current help settings
   */
  HelpSettings getHelpSettings() const;

  /**
   * @brief Set help settings
   * @param settings New settings
   */
  void setHelpSettings(const HelpSettings &settings);

  /**
   * @brief Register settings changed callback
   * @param callback Callback function
   */
  using SettingsChangedCallback = std::function<void(const HelpSettings &)>;
  void setSettingsChangedCallback(SettingsChangedCallback callback);

  /**
   * @brief Mark cache as dirty
   */
  void markCacheDirty();

private:
  view::UIFSMAdapter *fsmAdapter_;
  mutable std::mutex mutex_;

  // Cached data with dirty flag for rate limiting
  mutable bool cacheDirty_;
  mutable HelpSettings cachedSettings_;
  mutable std::chrono::steady_clock::time_point lastCacheUpdate_;
  static constexpr std::chrono::milliseconds CACHE_RATE_LIMIT{100};

  SettingsChangedCallback settingsChangedCallback_;

  void updateCachedSettings() const;
};

} // namespace Polish
} // namespace view
