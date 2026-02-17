#include "ObjectContextMenu.hpp"
#include <View/UIFSMAdapter.hpp>
#include <spdlog/spdlog.h>

namespace view {

ObjectContextMenu::ObjectContextMenu(ExtendedCommandManager &commandManager,
                                     SelectionManager *selectionManager,
                                     UIFSMAdapter *uiFSMAdapter)
    : ContextMenu("ObjectContextMenu"), commandManager_(commandManager),
      selectionManager_(selectionManager), uiFSMAdapter_(uiFSMAdapter) {}

void ObjectContextMenu::setSelectedObjects(
    const std::vector<uint32_t> &objectIds) {
  selectedObjectIds_ = objectIds;
}

const std::vector<uint32_t> &ObjectContextMenu::getSelectedObjects() const {
  return selectedObjectIds_;
}

void ObjectContextMenu::buildMenuItems() {
  items_.clear();

  bool hasSelection = !selectedObjectIds_.empty();

  // Select section
  items_.push_back(ContextMenuItem(
      "Select", "",
      [this]() { spdlog::info("Select action triggered from context menu"); },
      true));

  // Separator
  items_.push_back(ContextMenuItem());

  // Duplicate and Delete section
  items_.push_back(ContextMenuItem(
      "Duplicate", "Ctrl+D",
      [this]() {
        if (onDuplicate_) {
          onDuplicate_(selectedObjectIds_);
          spdlog::info("Duplicate executed from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onDuplicate_)));

  items_.push_back(ContextMenuItem(
      "Delete", "Del",
      [this]() {
        if (onDelete_) {
          onDelete_(selectedObjectIds_);
          spdlog::info("Delete executed from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onDelete_)));

  // Separator
  items_.push_back(ContextMenuItem());

  // Transform section
  items_.push_back(ContextMenuItem(
      "Move", "",
      [this]() {
        if (onMove_) {
          onMove_(selectedObjectIds_);
          spdlog::info("Move activated from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onMove_)));

  items_.push_back(ContextMenuItem(
      "Rotate", "",
      [this]() {
        if (onRotate_) {
          onRotate_(selectedObjectIds_);
          spdlog::info("Rotate activated from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onRotate_)));

  items_.push_back(ContextMenuItem(
      "Scale", "",
      [this]() {
        if (onScale_) {
          onScale_(selectedObjectIds_);
          spdlog::info("Scale activated from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onScale_)));

  // Separator
  items_.push_back(ContextMenuItem());

  // Properties and Layers section
  items_.push_back(ContextMenuItem(
      "Properties...", "Ctrl+P",
      [this]() {
        if (onOpenProperties_) {
          onOpenProperties_();
          spdlog::info("Properties dialog opened from context menu");
        }
      },
      static_cast<bool>(onOpenProperties_)));

  items_.push_back(ContextMenuItem(
      "Layers...", "Ctrl+L",
      [this]() {
        if (onOpenLayers_) {
          onOpenLayers_();
          spdlog::info("Layers dialog opened from context menu");
        }
      },
      static_cast<bool>(onOpenLayers_)));

  // Separator
  items_.push_back(ContextMenuItem());

  // Z-order section
  items_.push_back(ContextMenuItem(
      "Bring to Front", "",
      [this]() {
        if (onBringToFront_) {
          onBringToFront_(selectedObjectIds_);
          spdlog::info(
              "Bring to Front executed from context menu for {} objects",
              selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onBringToFront_)));

  items_.push_back(ContextMenuItem(
      "Send to Back", "",
      [this]() {
        if (onSendToBack_) {
          onSendToBack_(selectedObjectIds_);
          spdlog::info("Send to Back executed from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onSendToBack_)));

  // Separator
  items_.push_back(ContextMenuItem());

  // Copy and Mirror section
  items_.push_back(ContextMenuItem(
      "Create Copy", "",
      [this]() {
        if (onCreateCopy_) {
          onCreateCopy_(selectedObjectIds_);
          spdlog::info("Create Copy executed from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onCreateCopy_)));

  items_.push_back(ContextMenuItem(
      "Mirror", "",
      [this]() {
        if (onMirror_) {
          onMirror_(selectedObjectIds_);
          spdlog::info("Mirror executed from context menu for {} objects",
                       selectedObjectIds_.size());
        }
      },
      hasSelection && static_cast<bool>(onMirror_)));
}

void ObjectContextMenu::render() {
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

void ObjectContextMenu::setOnDuplicate(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onDuplicate_ = std::move(callback);
}

void ObjectContextMenu::setOnDelete(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onDelete_ = std::move(callback);
}

void ObjectContextMenu::setOnMove(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onMove_ = std::move(callback);
}

void ObjectContextMenu::setOnRotate(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onRotate_ = std::move(callback);
}

void ObjectContextMenu::setOnScale(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onScale_ = std::move(callback);
}

void ObjectContextMenu::setOnOpenProperties(std::function<void()> callback) {
  onOpenProperties_ = std::move(callback);
}

void ObjectContextMenu::setOnOpenLayers(std::function<void()> callback) {
  onOpenLayers_ = std::move(callback);
}

void ObjectContextMenu::setOnBringToFront(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onBringToFront_ = std::move(callback);
}

void ObjectContextMenu::setOnSendToBack(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onSendToBack_ = std::move(callback);
}

void ObjectContextMenu::setOnCreateCopy(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onCreateCopy_ = std::move(callback);
}

void ObjectContextMenu::setOnMirror(
    std::function<void(const std::vector<uint32_t> &)> callback) {
  onMirror_ = std::move(callback);
}

} // namespace view
