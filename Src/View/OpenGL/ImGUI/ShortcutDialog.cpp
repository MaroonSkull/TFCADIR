#include "ShortcutDialog.hpp"
#include <algorithm>
#include <cstring>
#include <spdlog/spdlog.h>

namespace view {

// ==========================================================================
// Constructor / Destructor
// ==========================================================================

ShortcutDialog::ShortcutDialog(
    std::shared_ptr<ShortcutConfigManager> shortcutManager)
    : shortcutManager_(shortcutManager) {
  // Initialize search text buffer
  std::memset(searchText_, 0, sizeof(searchText_));
  std::memset(filePathBuffer_, 0, sizeof(filePathBuffer_));

  // Cache shortcuts
  if (shortcutManager_) {
    cachedShortcuts_ = shortcutManager_->getAllShortcuts();
    updateFilteredList();
  }
}

ShortcutDialog::~ShortcutDialog() = default;

// ==========================================================================
// Public Methods
// ==========================================================================

void ShortcutDialog::render() {
  if (!isOpen_) {
    return;
  }

  // Set dialog size
  ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);

  // Begin dialog
  if (ImGui::Begin("Keyboard Shortcuts", &isOpen_,
                   ImGuiWindowFlags_NoCollapse)) {
    // Render search bar
    renderSearchBar();

    ImGui::Separator();

    // Render shortcuts table
    renderShortcutsTable();

    ImGui::Separator();

    // Render action buttons
    renderActionButtons();

    // Render conflict warning if any
    renderConflictWarning();
  }
  ImGui::End();

  // Render key capture popup if active
  if (isCapturingKey_) {
    renderKeyCapturePopup();
  }

  // Render file dialogs
  renderFileDialogs();
}

void ShortcutDialog::open() {
  isOpen_ = true;
  // Refresh cached shortcuts
  if (shortcutManager_) {
    cachedShortcuts_ = shortcutManager_->getAllShortcuts();
    updateFilteredList();
  }
  // Reset state
  selectedIndex_ = -1;
  conflictWarning_.clear();
  hasChanges_ = false;
}

void ShortcutDialog::close() {
  isOpen_ = false;
  isCapturingKey_ = false;
  conflictWarning_.clear();
}

bool ShortcutDialog::isOpen() const { return isOpen_; }

// ==========================================================================
// Rendering Methods
// ==========================================================================

void ShortcutDialog::renderSearchBar() {
  ImGui::Text("Search:");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(-1);
  bool changed = ImGui::InputText("##search", searchText_, sizeof(searchText_));
  if (changed) {
    updateFilteredList();
  }
}

void ShortcutDialog::renderShortcutsTable() {
  // Table with 3 columns: Command, Shortcut, Actions
  if (ImGui::BeginTable("shortcuts_table", 3,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY,
                        ImVec2(0, 300))) {
    // Set up columns
    ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_WidthStretch,
                            0.5f);
    ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed,
                            120.0f);
    ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed,
                            120.0f);
    ImGui::TableHeadersRow();

    // List filtered shortcuts
    for (size_t i = 0; i < filteredIndices_.size(); ++i) {
      const auto &entry = cachedShortcuts_[filteredIndices_[i]];

      ImGui::TableNextRow();

      // Column 1: Command name
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%s", entry.displayName.c_str());

      // Column 2: Shortcut
      ImGui::TableSetColumnIndex(1);
      std::string shortcutText = entry.shortcut.toString();
      if (shortcutText.empty() || entry.shortcut.key == 0) {
        shortcutText = "-";
      }
      ImGui::Text("%s", shortcutText.c_str());

      // Column 3: Actions
      ImGui::TableSetColumnIndex(2);

      // Set button
      std::string setBtnId = "Set##" + entry.command;
      if (ImGui::SmallButton(setBtnId.c_str())) {
        startKeyCapture(entry.command);
      }

      ImGui::SameLine();

      // Clear button
      std::string clearBtnId = "Clear##" + entry.command;
      if (ImGui::SmallButton(clearBtnId.c_str())) {
        clearShortcut(entry.command);
        hasChanges_ = true;
      }
    }

    ImGui::EndTable();
  }
}

void ShortcutDialog::renderActionButtons() {
  // Row 1: Import/Export
  if (ImGui::Button("Import...")) {
    showImportDialog_ = true;
    std::strncpy(filePathBuffer_, "shortcuts.yaml",
                 sizeof(filePathBuffer_) - 1);
  }
  ImGui::SameLine();
  if (ImGui::Button("Export...")) {
    showExportDialog_ = true;
    std::strncpy(filePathBuffer_, "shortcuts_export.yaml",
                 sizeof(filePathBuffer_) - 1);
  }

  // Row 2: Reset
  ImGui::SameLine(ImGui::GetWindowWidth() - 150);
  if (ImGui::Button("Reset to Defaults")) {
    if (shortcutManager_) {
      shortcutManager_->resetToDefaults();
      cachedShortcuts_ = shortcutManager_->getAllShortcuts();
      updateFilteredList();
      hasChanges_ = true;
      spdlog::info("Reset all shortcuts to defaults");
    }
  }

  // Row 3: Save (if changes)
  if (hasChanges_) {
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      saveShortcuts();
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "*");
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Unsaved changes");
    }
  }
}

void ShortcutDialog::renderKeyCapturePopup() {
  // Open popup on first frame
  if (!ImGui::IsPopupOpen("Capture Key")) {
    ImGui::OpenPopup("Capture Key");
  }

  // Center popup
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("Capture Key", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Press a key combination for: %s", capturingCommand_.c_str());
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "(Ctrl, Shift, Alt + Key)");
    ImGui::Spacing();

    // Display current captured combination
    if (capturedKey_ != 0) {
      ShortcutKey tempKey(capturedKey_, capturedModifiers_);
      ImGui::Text("Captured: %s", tempKey.toString().c_str());
    } else {
      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Waiting...");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Capture keyboard input
    ImGuiIO &io = ImGui::GetIO();

    // Check for modifier keys
    int mods = 0;
    if (io.KeyCtrl) {
      mods |= 1;
    }
    if (io.KeyShift) {
      mods |= 2;
    }
    if (io.KeyAlt) {
      mods |= 4;
    }

    // Check for key press (non-modifier keys)
    for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
      if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k))) {
        // Ignore modifier-only presses
        if (k != ImGuiKey_LeftCtrl && k != ImGuiKey_RightCtrl &&
            k != ImGuiKey_LeftShift && k != ImGuiKey_RightShift &&
            k != ImGuiKey_LeftAlt && k != ImGuiKey_RightAlt &&
            k != ImGuiKey_LeftSuper && k != ImGuiKey_RightSuper) {
          capturedKey_ = k;
          capturedModifiers_ = mods;
        }
      }
    }

    // Buttons
    if (ImGui::Button("OK", ImVec2(120, 0))) {
      stopKeyCapture();
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      cancelKeyCapture();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void ShortcutDialog::renderConflictWarning() {
  if (!conflictWarning_.empty()) {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "! %s",
                       conflictWarning_.c_str());
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("This shortcut conflicts with another command");
    }
  }
}

void ShortcutDialog::renderFileDialogs() {
  // Import dialog
  if (showImportDialog_) {
    ImGui::OpenPopup("Import Shortcuts");
    showImportDialog_ = false;
  }

  if (ImGui::BeginPopupModal("Import Shortcuts", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("File path:");
    ImGui::InputText("##filepath", filePathBuffer_, sizeof(filePathBuffer_));

    ImGui::Spacing();

    if (ImGui::Button("Import", ImVec2(120, 0))) {
      importShortcuts(std::string(filePathBuffer_));
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }

  // Export dialog
  if (showExportDialog_) {
    ImGui::OpenPopup("Export Shortcuts");
    showExportDialog_ = false;
  }

  if (ImGui::BeginPopupModal("Export Shortcuts", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("File path:");
    ImGui::InputText("##filepath", filePathBuffer_, sizeof(filePathBuffer_));

    ImGui::Spacing();

    if (ImGui::Button("Export", ImVec2(120, 0))) {
      exportShortcuts(std::string(filePathBuffer_));
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

// ==========================================================================
// Helper Methods
// ==========================================================================

void ShortcutDialog::updateFilteredList() {
  filteredIndices_.clear();

  std::string searchLower(searchText_);
  std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
                 ::tolower);

  for (size_t i = 0; i < cachedShortcuts_.size(); ++i) {
    const auto &entry = cachedShortcuts_[i];

    // If search is empty, show all
    if (searchLower.empty()) {
      filteredIndices_.push_back(i);
      continue;
    }

    // Check if search text matches command name or display name
    std::string cmdLower = entry.command;
    std::transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(),
                   ::tolower);

    std::string nameLower = entry.displayName;
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                   ::tolower);

    if (cmdLower.find(searchLower) != std::string::npos ||
        nameLower.find(searchLower) != std::string::npos) {
      filteredIndices_.push_back(i);
    }
  }
}

void ShortcutDialog::startKeyCapture(const std::string &command) {
  isCapturingKey_ = true;
  capturingCommand_ = command;
  capturedKey_ = 0;
  capturedModifiers_ = 0;
  conflictWarning_.clear();
}

void ShortcutDialog::stopKeyCapture() {
  if (!isCapturingKey_ || capturedKey_ == 0) {
    cancelKeyCapture();
    return;
  }

  // Create shortcut key
  ShortcutKey newShortcut(capturedKey_, capturedModifiers_);

  // Check for conflicts
  if (checkConflict(newShortcut, capturingCommand_)) {
    conflictWarning_ =
        "Conflicts with: " +
        shortcutManager_->getConflictCommand(newShortcut, capturingCommand_);
    // Still allow the shortcut to be set, but warn
  } else {
    conflictWarning_.clear();
  }

  // Apply the shortcut
  applyShortcut(capturingCommand_, newShortcut);

  // Reset capture state
  isCapturingKey_ = false;
  capturingCommand_.clear();
  capturedKey_ = 0;
  capturedModifiers_ = 0;
  hasChanges_ = true;
}

void ShortcutDialog::cancelKeyCapture() {
  isCapturingKey_ = false;
  capturingCommand_.clear();
  capturedKey_ = 0;
  capturedModifiers_ = 0;
}

bool ShortcutDialog::checkConflict(const ShortcutKey &shortcut,
                                   const std::string &excludeCommand) {
  if (!shortcutManager_) {
    return false;
  }
  return shortcutManager_->hasConflict(shortcut, excludeCommand);
}

void ShortcutDialog::applyShortcut(const std::string &command,
                                   const ShortcutKey &shortcut) {
  if (!shortcutManager_) {
    return;
  }

  shortcutManager_->setShortcut(command, shortcut);

  // Update cached shortcuts
  for (auto &entry : cachedShortcuts_) {
    if (entry.command == command) {
      entry.shortcut = shortcut;
      break;
    }
  }

  spdlog::info("Set shortcut for '{}': {}", command, shortcut.toString());
}

void ShortcutDialog::clearShortcut(const std::string &command) {
  if (!shortcutManager_) {
    return;
  }

  shortcutManager_->clearShortcut(command);

  // Update cached shortcuts
  for (auto &entry : cachedShortcuts_) {
    if (entry.command == command) {
      entry.shortcut = ShortcutKey();
      break;
    }
  }

  conflictWarning_.clear();
  spdlog::info("Cleared shortcut for '{}'", command);
}

void ShortcutDialog::resetToDefault(const std::string &command) {
  if (!shortcutManager_) {
    return;
  }

  shortcutManager_->resetToDefault(command);

  // Update cached shortcuts
  cachedShortcuts_ = shortcutManager_->getAllShortcuts();
  updateFilteredList();

  conflictWarning_.clear();
  hasChanges_ = true;
}

void ShortcutDialog::saveShortcuts() {
  if (!shortcutManager_) {
    return;
  }

  if (shortcutManager_->saveShortcuts()) {
    hasChanges_ = false;
    spdlog::info("Shortcuts saved successfully");
  } else {
    spdlog::error("Failed to save shortcuts");
  }
}

void ShortcutDialog::importShortcuts(const std::string &filePath) {
  if (!shortcutManager_) {
    return;
  }

  if (shortcutManager_->importFromFile(filePath)) {
    cachedShortcuts_ = shortcutManager_->getAllShortcuts();
    updateFilteredList();
    hasChanges_ = true;
    spdlog::info("Imported shortcuts from: {}", filePath);
  } else {
    spdlog::error("Failed to import shortcuts from: {}", filePath);
  }
}

void ShortcutDialog::exportShortcuts(const std::string &filePath) {
  if (!shortcutManager_) {
    return;
  }

  if (shortcutManager_->exportToFile(filePath)) {
    spdlog::info("Exported shortcuts to: {}", filePath);
  } else {
    spdlog::error("Failed to export shortcuts to: {}", filePath);
  }
}

} // namespace view
