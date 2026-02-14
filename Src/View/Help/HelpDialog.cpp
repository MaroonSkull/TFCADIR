#include "HelpDialog.hpp"
#include <algorithm>
#include <cctype>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

HelpDialog::HelpDialog(std::shared_ptr<HelpSystem> helpSystem)
    : helpSystem_(std::move(helpSystem)) {
  // Initialize cached data
  if (helpSystem_) {
    cachedTopics_ = helpSystem_->getAllTopics();
    cachedShortcuts_ = helpSystem_->getAllShortcuts();
    updateShortcutCategories();
    updateTutorialIds();
  }
  updateFilteredTopics();
}

HelpDialog::~HelpDialog() = default;

void HelpDialog::render() {
  if (!isOpen_) {
    return;
  }

  renderDialog();
}

void HelpDialog::open() {
  isOpen_ = true;
  spdlog::info("Help dialog opened");

  // Refresh cached data
  if (helpSystem_) {
    cachedTopics_ = helpSystem_->getAllTopics();
    cachedShortcuts_ = helpSystem_->getAllShortcuts();
    updateShortcutCategories();
    updateTutorialIds();
  }
  updateFilteredTopics();
}

void HelpDialog::close() {
  isOpen_ = false;
  spdlog::info("Help dialog closed");
}

bool HelpDialog::isOpen() const { return isOpen_; }

void HelpDialog::setContext(const std::string &context) {
  currentContext_ = context;
}

void HelpDialog::openWithContext(const std::string &context) {
  setContext(context);
  open();

  // Find and select the relevant topic
  if (helpSystem_ && helpSystem_->hasHelp(context)) {
    std::string helpText = helpSystem_->getHelpText(context);
    // Find topic index by context
    for (size_t i = 0; i < cachedTopics_.size(); ++i) {
      if (cachedTopics_[i].id == context ||
          cachedTopics_[i].content == helpText) {
        selectTopic(static_cast<int>(i));
        break;
      }
    }
  }
}

void HelpDialog::switchToTab(int tab) { currentTab_ = std::clamp(tab, 0, 2); }

void HelpDialog::renderDialog() {
  // Set dialog size and position
  ImGui::SetNextWindowSize(dialogSize_, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(
      ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - dialogSize_.x * 0.5f,
             ImGui::GetIO().DisplaySize.y * 0.5f - dialogSize_.y * 0.5f),
      ImGuiCond_FirstUseEver);

  // Dialog flags
  ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

  if (!ImGui::Begin("TFCADIR Help", &isOpen_, flags)) {
    ImGui::End();
    return;
  }

  // Render tab bar
  renderTabBar();

  // Render content based on current tab
  switch (currentTab_) {
  case 0:
    renderTopicsTab();
    break;
  case 1:
    renderShortcutsTab();
    break;
  case 2:
    renderTutorialsTab();
    break;
  }

  // Render close button
  renderCloseButton();

  ImGui::End();
}

void HelpDialog::renderTabBar() {
  if (ImGui::BeginTabBar("HelpTabs")) {
    if (ImGui::BeginTabItem("Help Topics")) {
      if (currentTab_ != 0) {
        currentTab_ = 0;
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Keyboard Shortcuts")) {
      if (currentTab_ != 1) {
        currentTab_ = 1;
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Tutorials")) {
      if (currentTab_ != 2) {
        currentTab_ = 2;
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}

void HelpDialog::renderTopicsTab() {
  // Render search bar
  renderSearchBar();

  ImGui::Separator();

  // Two-column layout: topics list (left) and content (right)
  float leftWidth = dialogSize_.x * leftPanelRatio_;
  float rightWidth = dialogSize_.x - leftWidth - 20.0f;

  ImGui::Columns(2, "HelpColumns", true);
  ImGui::SetColumnWidth(0, leftWidth);

  // Left panel: Topics list
  renderTopicsList();

  ImGui::NextColumn();

  // Right panel: Topic content
  renderTopicContent();

  ImGui::Columns(1);
}

void HelpDialog::renderShortcutsTab() {
  // Category filter
  ImGui::Text("Category:");
  ImGui::SameLine();
  ImGui::PushItemWidth(200);
  if (ImGui::BeginCombo(
          "##Category",
          shortcutCategories_[selectedShortcutCategory_].c_str())) {
    for (int i = 0; i < static_cast<int>(shortcutCategories_.size()); ++i) {
      bool isSelected = (i == selectedShortcutCategory_);
      if (ImGui::Selectable(shortcutCategories_[i].c_str(), isSelected)) {
        selectedShortcutCategory_ = i;
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  ImGui::PopItemWidth();

  ImGui::Separator();

  // Shortcuts table
  renderShortcutsTable();
}

void HelpDialog::renderTutorialsTab() {
  // Two-column layout: tutorials list (left) and viewer (right)
  float leftWidth = dialogSize_.x * leftPanelRatio_;
  // float rightWidth = dialogSize_.x - leftWidth - 20.0f;

  ImGui::Columns(2, "TutorialColumns", true);
  ImGui::SetColumnWidth(0, leftWidth);

  // Left panel: Tutorials list
  renderTutorialsList();

  ImGui::NextColumn();

  // Right panel: Tutorial viewer
  renderTutorialViewer();

  ImGui::Columns(1);
}

void HelpDialog::renderSearchBar() {
  ImGui::Text("Search:");
  ImGui::SameLine();
  ImGui::PushItemWidth(-FLT_MIN);

  bool searchChanged =
      ImGui::InputText("##Search", searchText_, IM_ARRAYSIZE(searchText_));

  if (searchChanged) {
    updateFilteredTopics();
  }

  ImGui::PopItemWidth();

  // Clear button
  if (searchText_[0] != '\0') {
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) {
      searchText_[0] = '\0';
      updateFilteredTopics();
    }
  }
}

void HelpDialog::renderTopicsList() {
  ImGui::BeginChild("TopicsList", ImVec2(0, -30), true);

  for (size_t i : filteredTopicIndices_) {
    if (i >= cachedTopics_.size()) {
      continue;
    }

    const auto &topic = cachedTopics_[i];
    bool isSelected = (static_cast<int>(i) == selectedTopicIndex_);

    if (ImGui::Selectable(topic.title.c_str(), isSelected)) {
      selectTopic(static_cast<int>(i));
    }

    // Show category as subtle text
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("%s", topic.category.c_str());
    }
  }

  ImGui::EndChild();

  // Show count
  ImGui::Text("%zu topics", filteredTopicIndices_.size());
}

void HelpDialog::renderTopicContent() {
  ImGui::BeginChild("TopicContent", ImVec2(0, -30), true);

  if (selectedTopicIndex_ >= 0 &&
      selectedTopicIndex_ < static_cast<int>(cachedTopics_.size())) {
    const auto &topic = cachedTopics_[selectedTopicIndex_];

    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.8f, 1.0f));
    ImGui::Text("%s", topic.title.c_str());
    ImGui::PopStyleColor();

    ImGui::Separator();

    // Content with basic formatting
    renderHelpText(topic.content);

    // Related topics
    if (!topic.relatedTopics.empty()) {
      ImGui::Separator();
      ImGui::Text("Related Topics:");
      for (const auto &relatedId : topic.relatedTopics) {
        for (size_t i = 0; i < cachedTopics_.size(); ++i) {
          if (cachedTopics_[i].id == relatedId) {
            if (ImGui::SmallButton(cachedTopics_[i].title.c_str())) {
              selectTopic(static_cast<int>(i));
            }
            break;
          }
        }
      }
    }
  } else {
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "Select a topic from the list");
  }

  ImGui::EndChild();
}

void HelpDialog::renderShortcutsTable() {
  ImGui::BeginChild("ShortcutsTable", ImVec2(0, -30), true);

  // Table header
  ImGui::Columns(3, "ShortcutsColumns");
  ImGui::Separator();
  ImGui::Text("Key");
  ImGui::NextColumn();
  ImGui::Text("Action");
  ImGui::NextColumn();
  ImGui::Text("Description");
  ImGui::NextColumn();
  ImGui::Separator();

  // Get shortcuts for selected category
  std::string category = shortcutCategories_[selectedShortcutCategory_];
  std::vector<HelpShortcutEntry> shortcuts;
  if (category == "All") {
    shortcuts = cachedShortcuts_;
  } else {
    shortcuts = helpSystem_->getShortcutsByCategory(category);
  }

  for (const auto &shortcut : shortcuts) {
    // Key column with colored background
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
    ImGui::Text("%s", formatShortcutKey(shortcut.key).c_str());
    ImGui::PopStyleColor();
    ImGui::NextColumn();

    // Action column
    ImGui::Text("%s", shortcut.action.c_str());
    ImGui::NextColumn();

    // Description column
    ImGui::TextWrapped("%s", shortcut.description.c_str());
    ImGui::NextColumn();

    ImGui::Separator();
  }

  ImGui::Columns(1);
  ImGui::EndChild();

  // Show count
  ImGui::Text("%zu shortcuts", shortcuts.size());
}

void HelpDialog::renderTutorialsList() {
  ImGui::BeginChild("TutorialsList", ImVec2(0, -30), true);

  for (size_t i = 0; i < tutorialIds_.size(); ++i) {
    const HelpTutorial *tutorial = helpSystem_->getTutorial(tutorialIds_[i]);
    if (!tutorial) {
      continue;
    }

    bool isSelected = (static_cast<int>(i) == selectedTutorialIndex_);

    // Show completion status
    std::string label = tutorial->title;
    if (tutorial->completed) {
      label = "[Done] " + label;
    }

    if (ImGui::Selectable(label.c_str(), isSelected)) {
      selectedTutorialIndex_ = static_cast<int>(i);
    }

    // Show description on hover
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("%s", tutorial->description.c_str());
    }
  }

  ImGui::EndChild();

  // Show count
  ImGui::Text("%zu tutorials", tutorialIds_.size());
}

void HelpDialog::renderTutorialViewer() {
  ImGui::BeginChild("TutorialViewer", ImVec2(0, -30), true);

  if (selectedTutorialIndex_ >= 0 &&
      selectedTutorialIndex_ < static_cast<int>(tutorialIds_.size())) {
    const HelpTutorial *tutorial =
        helpSystem_->getTutorial(tutorialIds_[selectedTutorialIndex_]);

    if (tutorial) {
      // Check if this is the active tutorial
      const HelpTutorial *activeTutorial = helpSystem_->getCurrentTutorial();
      bool isActive = (activeTutorial && activeTutorial->id == tutorial->id);

      // Title
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.8f, 1.0f));
      ImGui::Text("%s", tutorial->title.c_str());
      ImGui::PopStyleColor();

      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s",
                         tutorial->description.c_str());

      ImGui::Separator();

      if (isActive) {
        // Show current step
        if (tutorial->currentStep < tutorial->steps.size()) {
          renderTutorialStep(*tutorial, tutorial->steps[tutorial->currentStep]);
        }

        // Progress
        ImGui::Separator();
        ImGui::Text("Step %zu of %zu", tutorial->currentStep + 1,
                    tutorial->steps.size());

        // Navigation buttons
        if (ImGui::Button("Next Step")) {
          helpSystem_->advanceTutorial();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel Tutorial")) {
          helpSystem_->cancelTutorial();
        }
      } else {
        // Show tutorial overview
        ImGui::Text("Steps:");
        for (size_t i = 0; i < tutorial->steps.size(); ++i) {
          ImGui::BulletText("%s", tutorial->steps[i].title.c_str());
        }

        ImGui::Separator();

        // Start button
        if (ImGui::Button("Start Tutorial")) {
          helpSystem_->startTutorial(tutorial->id);
          selectedTutorialIndex_ = -1; // Refresh selection
        }
      }
    }
  } else {
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "Select a tutorial from the list");
    ImGui::TextWrapped(
        "Tutorials guide you through common tasks step by step.");
  }

  ImGui::EndChild();
}

void HelpDialog::renderTutorialStep(const HelpTutorial &tutorial,
                                    const HelpTutorialStep &step) {
  // Step title
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 1.0f, 0.8f, 1.0f));
  ImGui::Text("Step %zu: %s", tutorial.currentStep + 1, step.title.c_str());
  ImGui::PopStyleColor();

  ImGui::Separator();

  // Instructions
  ImGui::TextWrapped("%s", step.instructions.c_str());

  // Hint
  if (!step.hint.empty()) {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Hint: %s",
                       step.hint.c_str());
  }
}

void HelpDialog::renderCloseButton() {
  ImGui::Separator();

  // Right-align the close button
  float buttonWidth = 100.0f;
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth -
                       ImGui::GetStyle().WindowPadding.x);

  if (ImGui::Button("Close", ImVec2(buttonWidth, 0))) {
    close();
  }

  // Keyboard shortcut hint
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Press F1 for help");
}

void HelpDialog::updateFilteredTopics() {
  filteredTopicIndices_.clear();

  std::string searchLower = helpSystem_->toLower(searchText_);

  for (size_t i = 0; i < cachedTopics_.size(); ++i) {
    if (searchLower.empty()) {
      filteredTopicIndices_.push_back(i);
      continue;
    }

    const auto &topic = cachedTopics_[i];

    // Search in title
    std::string titleLower = helpSystem_->toLower(topic.title);
    if (titleLower.find(searchLower) != std::string::npos) {
      filteredTopicIndices_.push_back(i);
      continue;
    }

    // Search in content
    std::string contentLower = helpSystem_->toLower(topic.content);
    if (contentLower.find(searchLower) != std::string::npos) {
      filteredTopicIndices_.push_back(i);
      continue;
    }

    // Search in keywords
    for (const auto &keyword : topic.keywords) {
      std::string keywordLower = helpSystem_->toLower(keyword);
      if (keywordLower.find(searchLower) != std::string::npos) {
        filteredTopicIndices_.push_back(i);
        break;
      }
    }
  }
}

void HelpDialog::selectTopic(int index) {
  selectedTopicIndex_ = index;
  contentScrollY_ = 0.0f;
}

std::string HelpDialog::formatShortcutKey(const std::string &key) const {
  // Add visual separators for display
  std::string result = key;

  // Replace "+" with " + " for better readability
  size_t pos = 0;
  while ((pos = result.find("+", pos)) != std::string::npos) {
    result.replace(pos, 1, " + ");
    pos += 3;
  }

  return result;
}

void HelpDialog::updateShortcutCategories() {
  shortcutCategories_.clear();
  shortcutCategories_.push_back("All");

  for (const auto &shortcut : cachedShortcuts_) {
    if (std::find(shortcutCategories_.begin(), shortcutCategories_.end(),
                  shortcut.category) == shortcutCategories_.end()) {
      shortcutCategories_.push_back(shortcut.category);
    }
  }
}

void HelpDialog::updateTutorialIds() {
  tutorialIds_ = helpSystem_->getTutorialIds();
}

void HelpDialog::renderHelpText(const std::string &text) {
  // Split text into lines and render with basic formatting
  auto lines = splitLines(text);

  for (const auto &line : lines) {
    // Check for headers (lines ending with :)
    if (!line.empty() && line.back() == ':') {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
      ImGui::Text("%s", line.c_str());
      ImGui::PopStyleColor();
    }
    // Check for bullet points
    else if (line.size() >= 2 && line[0] == '-' && line[1] == ' ') {
      ImGui::BulletText("%s", line.substr(2).c_str());
    }
    // Check for numbered items
    else if (line.size() >= 3 && std::isdigit(line[0]) && line[1] == '.' &&
             line[2] == ' ') {
      ImGui::Text("  %s", line.c_str());
    }
    // Regular text
    else if (!line.empty()) {
      ImGui::TextWrapped("%s", line.c_str());
    } else {
      // Empty line - add some spacing
      ImGui::Spacing();
    }
  }
}

std::vector<std::string> HelpDialog::splitLines(const std::string &text) const {
  std::vector<std::string> lines;
  std::istringstream stream(text);
  std::string line;

  while (std::getline(stream, line)) {
    lines.push_back(line);
  }

  return lines;
}

} // namespace view
