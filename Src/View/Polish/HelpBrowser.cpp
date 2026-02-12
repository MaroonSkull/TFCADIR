#include "HelpBrowser.hpp"
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

namespace view {
namespace Polish {

HelpBrowser::HelpBrowser(view::UIFSMAdapter *fsmAdapter)
    : fsmAdapter_(fsmAdapter), cacheDirty_(true), cachedSettings_{},
      lastCacheUpdate_{} {}

HelpBrowser::HelpTopic
HelpBrowser::getHelpTopic(const std::string &topicId) const {
  std::lock_guard<std::mutex> lock(mutex_);

  // Query UIFSMAdapter for help topic data
  auto topics = fsmAdapter_->getHelpTopics();

  auto it =
      std::find_if(topics.begin(), topics.end(), [&topicId](const auto &topic) {
        return topic.id == topicId;
      });

  if (it != topics.end()) {
    return *it;
  }

  // Return empty topic if not found
  return HelpTopic{};
}

std::vector<HelpBrowser::HelpTopic>
HelpBrowser::getHelpTopicsByCategory(const std::string &category) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto topics = fsmAdapter_->getHelpTopics();

  std::vector<HelpTopic> result;
  std::copy_if(
      topics.begin(), topics.end(), std::back_inserter(result),
      [&category](const auto &topic) { return topic.category == category; });

  return result;
}

std::vector<HelpBrowser::HelpSearchResult>
HelpBrowser::searchHelpTopics(const std::string &query) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto topics = fsmAdapter_->getHelpTopics();
  std::vector<HelpSearchResult> results;

  // Simple relevance scoring based on keyword matching
  std::string lowerQuery = query;
  std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(),
                 ::tolower);

  for (const auto &topic : topics) {
    float score = 0.0f;

    // Check title match
    std::string lowerTitle = topic.title;
    std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(),
                   ::tolower);

    if (lowerTitle.find(lowerQuery) != std::string::npos) {
      score += 1.0f;
    }

    // Check keywords match
    for (const auto &keyword : topic.keywords) {
      std::string lowerKeyword = keyword;
      std::transform(lowerKeyword.begin(), lowerKeyword.end(),
                     lowerKeyword.begin(), ::tolower);

      if (lowerKeyword.find(lowerQuery) != std::string::npos) {
        score += 0.5f;
      }
    }

    // Check content match
    std::string lowerContent = topic.content;
    std::transform(lowerContent.begin(), lowerContent.end(),
                   lowerContent.begin(), ::tolower);

    size_t pos = 0;
    int matchCount = 0;
    while ((pos = lowerContent.find(lowerQuery, pos)) != std::string::npos) {
      matchCount++;
      pos += lowerQuery.length();
    }
    score += matchCount * 0.1f;

    if (score > 0.0f) {
      results.push_back({topic, score});
    }
  }

  // Sort by relevance score (descending)
  std::sort(results.begin(), results.end(), [](const auto &a, const auto &b) {
    return a.relevanceScore > b.relevanceScore;
  });

  return results;
}

HelpBrowser::HelpTopic
HelpBrowser::getContextSensitiveHelp(const std::string &toolId) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto topics = fsmAdapter_->getHelpTopics();

  // Look for topic with matching tool ID in keywords
  auto it =
      std::find_if(topics.begin(), topics.end(), [&toolId](const auto &topic) {
        return std::find(topic.keywords.begin(), topic.keywords.end(),
                         toolId) != topic.keywords.end();
      });

  if (it != topics.end()) {
    return *it;
  }

  return HelpTopic{};
}

std::vector<std::string> HelpBrowser::getCategories() const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto topics = fsmAdapter_->getHelpTopics();

  std::vector<std::string> categories;
  for (const auto &topic : topics) {
    if (std::find(categories.begin(), categories.end(), topic.category) ==
        categories.end()) {
      categories.push_back(topic.category);
    }
  }

  return categories;
}

std::string HelpBrowser::getKeyboardShortcutsReference() const {
  std::lock_guard<std::mutex> lock(mutex_);

  // Query ShortcutManager for shortcuts
  auto shortcuts = fsmAdapter_->getShortcuts();

  std::ostringstream oss;
  oss << "Keyboard Shortcuts Reference\n";
  oss << "========================\n\n";

  // List all shortcuts
  for (const auto &pair : shortcuts.shortcuts) {
    oss << pair.second.description << ": " << pair.second.id << "\n";
  }

  oss << "\n";
  return oss.str();
}

std::vector<std::pair<std::string, std::string>>
HelpBrowser::getTableOfContents() const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto topics = fsmAdapter_->getHelpTopics();

  std::vector<std::pair<std::string, std::string>> toc;

  for (const auto &topic : topics) {
    toc.emplace_back(topic.id, topic.title);
  }

  return toc;
}

HelpBrowser::HelpSettings HelpBrowser::getHelpSettings() const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto now = std::chrono::steady_clock::now();
  if (cacheDirty_ || (now - lastCacheUpdate_) >= CACHE_RATE_LIMIT) {
    updateCachedSettings();
  }

  return cachedSettings_;
}

void HelpBrowser::setHelpSettings(const HelpSettings &settings) {
  fsmAdapter_->setHelpSettings(settings);
  markCacheDirty();
}

void HelpBrowser::setSettingsChangedCallback(SettingsChangedCallback callback) {
  settingsChangedCallback_ = std::move(callback);
}

void HelpBrowser::markCacheDirty() { cacheDirty_ = true; }

void HelpBrowser::updateCachedSettings() const {
  cachedSettings_ = fsmAdapter_->getHelpSettings();
  cacheDirty_ = false;
  lastCacheUpdate_ = std::chrono::steady_clock::now();
}

} // namespace Polish
} // namespace view
