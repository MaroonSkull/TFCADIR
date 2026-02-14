#include "CanvasContextMenu.hpp"
#include <View/UIFSMAdapter.hpp>
#include <spdlog/spdlog.h>

namespace view {

CanvasContextMenu::CanvasContextMenu(ExtendedCommandManager &commandManager,
                                     NavigationManager &navigationManager,
                                     UIFSMAdapter *uiFSMAdapter)
    : ContextMenu("CanvasContextMenu"), commandManager_(commandManager),
      navigationManager_(navigationManager), uiFSMAdapter_(uiFSMAdapter) {}

void CanvasContextMenu::buildMenuItems() {
  items_.clear();

  // Undo/Redo section
  items_.push_back(ContextMenuItem(
      "Undo", "Ctrl+Z",
      [this]() {
        if (commandManager_.canUndo()) {
          commandManager_.undo();
          spdlog::info("Undo executed from context menu");
        }
      },
      commandManager_.canUndo()));

  items_.push_back(ContextMenuItem(
      "Redo", "Ctrl+Y",
      [this]() {
        if (commandManager_.canRedo()) {
          commandManager_.redo();
          spdlog::info("Redo executed from context menu");
        }
      },
      commandManager_.canRedo()));

  // Separator
  items_.push_back(ContextMenuItem());

  // Cut/Copy/Paste section
  items_.push_back(ContextMenuItem(
      "Cut", "Ctrl+X",
      [this]() {
        if (onCut_) {
          onCut_();
          spdlog::info("Cut executed from context menu");
        }
      },
      static_cast<bool>(onCut_)));

  items_.push_back(ContextMenuItem(
      "Copy", "Ctrl+C",
      [this]() {
        if (onCopy_) {
          onCopy_();
          spdlog::info("Copy executed from context menu");
        }
      },
      static_cast<bool>(onCopy_)));

  items_.push_back(ContextMenuItem(
      "Paste", "Ctrl+V",
      [this]() {
        if (onPaste_) {
          onPaste_();
          spdlog::info("Paste executed from context menu");
        }
      },
      static_cast<bool>(onPaste_)));

  // Separator
  items_.push_back(ContextMenuItem());

  // Repeat last command
  std::string repeatLabel = "Repeat " + getLastCommandName();
  items_.push_back(ContextMenuItem(
      repeatLabel, "",
      [this]() {
        spdlog::info("Repeat last command: {}", getLastCommandName());
        // The actual repeat logic would be implemented by the command manager
      },
      !getLastCommandName().empty()));

  // Separator
  items_.push_back(ContextMenuItem());

  // Navigation section
  items_.push_back(ContextMenuItem(
      "Pan", "",
      [this]() {
        if (onPan_) {
          onPan_();
          spdlog::info("Pan activated from context menu");
        }
      },
      static_cast<bool>(onPan_)));

  items_.push_back(ContextMenuItem(
      "Zoom to Fit", "",
      [this]() {
        if (onZoomToFit_) {
          onZoomToFit_();
          spdlog::info("Zoom to Fit executed from context menu");
        }
      },
      static_cast<bool>(onZoomToFit_)));

  items_.push_back(ContextMenuItem(
      "Zoom to Selection", "",
      [this]() {
        if (onZoomToSelection_) {
          onZoomToSelection_();
          spdlog::info("Zoom to Selection executed from context menu");
        }
      },
      static_cast<bool>(onZoomToSelection_)));

  // Separator
  items_.push_back(ContextMenuItem());

  // Settings section
  items_.push_back(ContextMenuItem(
      "Snap Options...", "",
      [this]() {
        if (onOpenSnapSettings_) {
          onOpenSnapSettings_();
          spdlog::info("Snap Options dialog opened from context menu");
        }
      },
      static_cast<bool>(onOpenSnapSettings_)));

  items_.push_back(ContextMenuItem(
      "Grid Settings...", "",
      [this]() {
        if (onOpenGridSettings_) {
          onOpenGridSettings_();
          spdlog::info("Grid Settings dialog opened from context menu");
        }
      },
      static_cast<bool>(onOpenGridSettings_)));
}

void CanvasContextMenu::render() {
  if (!isVisible_) {
    return;
  }

  // Build menu items based on current state
  buildMenuItems();

  // Set position and open popup
  ImGui::SetNextWindowPos(position_, ImGuiCond_Appearing);

  if (ImGui::BeginPopup(popupId_.c_str())) {
    for (const auto &item : items_) {
      renderMenuItem(item);
    }
    ImGui::EndPopup();
  } else {
    // Popup was closed
    isVisible_ = false;
  }
}

void CanvasContextMenu::setOnOpenSnapSettings(std::function<void()> callback) {
  onOpenSnapSettings_ = std::move(callback);
}

void CanvasContextMenu::setOnOpenGridSettings(std::function<void()> callback) {
  onOpenGridSettings_ = std::move(callback);
}

void CanvasContextMenu::setOnPan(std::function<void()> callback) {
  onPan_ = std::move(callback);
}

void CanvasContextMenu::setOnZoomToFit(std::function<void()> callback) {
  onZoomToFit_ = std::move(callback);
}

void CanvasContextMenu::setOnZoomToSelection(std::function<void()> callback) {
  onZoomToSelection_ = std::move(callback);
}

void CanvasContextMenu::setOnCut(std::function<void()> callback) {
  onCut_ = std::move(callback);
}

void CanvasContextMenu::setOnCopy(std::function<void()> callback) {
  onCopy_ = std::move(callback);
}

void CanvasContextMenu::setOnPaste(std::function<void()> callback) {
  onPaste_ = std::move(callback);
}

} // namespace view
