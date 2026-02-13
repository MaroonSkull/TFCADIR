#include "CommandHistoryPanel.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <format>
#include <imgui.h>
#include <sstream>

namespace view {

CommandHistoryPanel::CommandHistoryPanel(ExtendedCommandManager &commandManager)
    : commandManager_(commandManager), historyCache_{} {
  // Initialize cache with current time
  historyCache_.lastUpdate = std::chrono::steady_clock::now();
}

void CommandHistoryPanel::render() const {
  ImGui::Begin("Command History");

  // Render toolbar
  renderToolbar();
  ImGui::Separator();

  // Search/Filter input
  char filterText[256];
  ::strncpy(filterText, filterText_.c_str(), sizeof(filterText) - 1);
  filterText[sizeof(filterText) - 1] = '\0';
  if (ImGui::InputText("##Filter", filterText, sizeof(filterText))) {
    filterText_ = filterText;
    historyCache_.dirty = true;
  }
  ImGui::SameLine();
  ImGui::TextDisabled("Filter commands");
  ImGui::Separator();

  // Performance: Only update if dirty or > 100ms elapsed
  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - historyCache_.lastUpdate)
                     .count();

  if (!historyCache_.dirty && elapsed < 100) {
    renderCachedHistory();
  } else {
    updateHistoryCache();
    historyCache_.lastUpdate = now;
    historyCache_.dirty = false;
    renderCachedHistory();
  }

  ImGui::End();
}

void CommandHistoryPanel::renderToolbar() const {
  bool canUndo = commandManager_.canUndo();
  bool canRedo = commandManager_.canRedo();

  // Undo button
  ImGui::BeginDisabled(!canUndo);
  if (ImGui::Button("Undo (Ctrl+Z)")) {
    commandManager_.undo();
  }
  ImGui::EndDisabled();

  ImGui::SameLine();

  // Redo button
  ImGui::BeginDisabled(!canRedo);
  if (ImGui::Button("Redo (Ctrl+Y)")) {
    commandManager_.redo();
  }
  ImGui::EndDisabled();

  // Show command count
  size_t historySize = commandManager_.getHistorySize();
  size_t currentIndex = commandManager_.getCurrentIndex();
  ImGui::SameLine();
  ImGui::TextDisabled("(%zu commands, index %zu)", historySize, currentIndex);
}

void CommandHistoryPanel::renderRedoStack() const {
  size_t redoCount = historyCache_.cachedRedoStack.size();

  // Format header string
  std::string headerText;
  if (redoCount > 0) {
    headerText = std::format("Redo Stack ({})", redoCount);
  } else {
    headerText = "Redo Stack (Empty)";
  }

  // Fix: Check IsItemClicked OUTSIDE the CollapsingHeader block
  // to properly detect both expand and collapse actions
  ImGuiTreeNodeFlags headerFlags =
      showRedoStackExpanded_ ? ImGuiTreeNodeFlags_DefaultOpen : 0;
  bool headerVisible = ImGui::CollapsingHeader(headerText.c_str(), headerFlags);

  // Update expanded state when header is clicked (both expand and collapse)
  if (ImGui::IsItemClicked()) {
    showRedoStackExpanded_ = !showRedoStackExpanded_;
  }

  if (headerVisible) {
    if (redoCount == 0) {
      ImGui::TextDisabled("No commands to redo");
    } else {
      // Render redo stack in reverse order (next to redo is at top)
      for (int i = static_cast<int>(redoCount) - 1; i >= 0; --i) {
        const auto &info = historyCache_.cachedRedoStack[i];
        renderCommandItem(info, false, info.index);
      }
    }
  }
}

void CommandHistoryPanel::renderCurrentPosition() const {
  size_t currentIndex = historyCache_.cachedCurrentIndex;
  size_t historySize = historyCache_.cachedCommands.size();

  ImGui::Separator();
  if (currentIndex < historySize) {
    const auto &currentInfo = historyCache_.cachedCommands[currentIndex];
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[●] Current: %s",
                       currentInfo.description.c_str());
  } else if (historySize == 0) {
    ImGui::TextDisabled("No commands in history");
  } else {
    ImGui::TextDisabled("Current position: %zu / %zu", currentIndex,
                        historySize);
  }
  ImGui::Separator();
}

void CommandHistoryPanel::renderUndoStack() const {
  size_t undoCount = historyCache_.cachedUndoStack.size();

  // Format header string
  std::string headerText;
  if (undoCount > 0) {
    headerText = std::format("Undo Stack ({})", undoCount);
  } else {
    headerText = "Undo Stack (Empty)";
  }

  // Fix: Check IsItemClicked OUTSIDE of CollapsingHeader block
  // to properly detect both expand and collapse actions
  ImGuiTreeNodeFlags headerFlags =
      showUndoStackExpanded_ ? ImGuiTreeNodeFlags_DefaultOpen : 0;
  bool headerVisible = ImGui::CollapsingHeader(headerText.c_str(), headerFlags);
  // Update expanded state when header is clicked (both expand and collapse)
  if (ImGui::IsItemClicked()) {
    showUndoStackExpanded_ = !showUndoStackExpanded_;
  }

  if (headerVisible) {
    if (undoCount == 0) {
      ImGui::TextDisabled("No commands to undo");
    } else {
      // Render undo stack (most recent at top)
      for (int i = static_cast<int>(undoCount) - 1; i >= 0; --i) {
        const auto &info = historyCache_.cachedUndoStack[i];
        bool isCurrent = (info.index == historyCache_.cachedCurrentIndex);
        renderCommandItem(info, isCurrent, info.index);
      }
    }
  }
}

void CommandHistoryPanel::renderCommandItem(const CommandInfo &info,
                                            bool isCurrent,
                                            size_t index) const {
  // Selectable row for the command
  ImGui::PushID(static_cast<int>(index));

  // Icon based on command state
  const char *icon = isCurrent ? "[●]" : "[x]";

  // Command type color
  ImVec4 typeColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
  if (info.isMacro) {
    typeColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Orange for macros
  } else if (info.type == "CreateFigure") {
    typeColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for creation
  } else if (info.type == "DeleteFigure") {
    typeColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for deletion
  } else if (info.type == "MoveFigure") {
    typeColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f); // Blue for move
  } else if (info.type == "PropertyChange") {
    typeColor = ImVec4(0.8f, 0.0f, 1.0f, 1.0f); // Purple for property change
  }

  // Render the command item
  ImGui::TextColored(typeColor, "%s %zu: %s", icon, info.index,
                     info.description.c_str());

  // Show tooltip with additional info on hover
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::Text("Type: %s", info.type.c_str());
    ImGui::Text("Description: %s", info.description.c_str());
    if (!info.timestamp.empty()) {
      ImGui::Text("Timestamp: %s", info.timestamp.c_str());
    }
    if (!info.affectedFigures.empty()) {
      ImGui::Text("Affected Figures: %zu", info.affectedFigures.size());
      ImGui::Indent();
      for (uint32_t figId : info.affectedFigures) {
        ImGui::Text("Figure #%u", figId);
      }
      ImGui::Unindent();
    }
    if (info.isMacro && info.subCommandCount > 0) {
      ImGui::Text("Macro Command: %zu sub-commands", info.subCommandCount);
    }
    ImGui::EndTooltip();
  }

  // Allow clicking to jump to this state
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    /// Jump to state: undo/redo to reach target index
    size_t currentIndex = commandManager_.getCurrentIndex();

    if (index < currentIndex) {
      // Need to undo to reach this state
      size_t undoCount = currentIndex - index;
      for (size_t i = 0; i < undoCount; ++i) {
        commandManager_.undo();
      }
    } else if (index > currentIndex) {
      // Need to redo to reach this state
      size_t redoCount = index - currentIndex;
      for (size_t i = 0; i < redoCount; ++i) {
        commandManager_.redo();
      }
    }
    // If index == currentIndex, we're already there, do nothing
  }

  ImGui::PopID();
}

void CommandHistoryPanel::updateHistoryCache() const {
  // Query command manager for current state
  historyCache_.cachedCommands = commandManager_.getCommandHistory();
  historyCache_.cachedCurrentIndex = commandManager_.getCurrentIndex();

  // Split into undo and redo stacks
  historyCache_.cachedUndoStack.clear();
  historyCache_.cachedRedoStack.clear();

  size_t currentIndex = historyCache_.cachedCurrentIndex;
  const auto &commands = historyCache_.cachedCommands;

  // Filter commands based on filterText
  std::string filterTextLower = filterText_;
  std::transform(filterTextLower.begin(), filterTextLower.end(),
                 filterTextLower.begin(), ::tolower);

  // Commands from 0 to currentIndex are in undo stack (can be undone)
  for (size_t i = 0; i <= currentIndex && i < commands.size(); ++i) {
    const auto &cmd = commands[i];
    // Apply filter if specified
    if (filterTextLower.empty()) {
      historyCache_.cachedUndoStack.push_back(cmd);
    } else {
      std::string descLower = cmd.description;
      std::transform(descLower.begin(), descLower.end(), descLower.begin(),
                     ::tolower);
      if (descLower.find(filterTextLower) != std::string::npos) {
        historyCache_.cachedUndoStack.push_back(cmd);
      }
    }
  }

  // Commands after currentIndex are in redo stack (can be redone)
  for (size_t i = currentIndex + 1; i < commands.size(); ++i) {
    const auto &cmd = commands[i];
    // Apply filter if specified
    if (filterTextLower.empty()) {
      historyCache_.cachedRedoStack.push_back(cmd);
    } else {
      std::string descLower = cmd.description;
      std::transform(descLower.begin(), descLower.end(), descLower.begin(),
                     ::tolower);
      if (descLower.find(filterTextLower) != std::string::npos) {
        historyCache_.cachedRedoStack.push_back(cmd);
      }
    }
  }
}

void CommandHistoryPanel::renderCachedHistory() const {
  // Render redo stack (commands that can be redone)
  renderRedoStack();

  // Render current position indicator
  renderCurrentPosition();

  // Render undo stack (commands that can be undone)
  renderUndoStack();
}

} // namespace view
