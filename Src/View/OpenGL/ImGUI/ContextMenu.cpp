#include "ContextMenu.hpp"

namespace view {

ContextMenu::ContextMenu(const std::string &popupId) : popupId_(popupId) {}

void ContextMenu::show(const ImVec2 &pos) {
  position_ = pos;
  isVisible_ = true;
  ImGui::OpenPopup(popupId_.c_str());
}

void ContextMenu::hide() { isVisible_ = false; }

bool ContextMenu::isVisible() const { return isVisible_; }

void ContextMenu::renderSeparator() { ImGui::Separator(); }

bool ContextMenu::renderMenuItem(const ContextMenuItem &item) {
  if (item.label.empty()) {
    // Empty label means separator
    renderSeparator();
    return false;
  }

  if (item.enabled) {
    if (ImGui::MenuItem(item.label.c_str(), item.shortcut.empty()
                                                ? nullptr
                                                : item.shortcut.c_str())) {
      if (item.action) {
        item.action();
        // Track the last executed command
        setLastCommandName(item.label);
      }
      hide();
      return true;
    }
  } else {
    // Disabled item
    ImGui::MenuItem(item.label.c_str(),
                    item.shortcut.empty() ? nullptr : item.shortcut.c_str(),
                    false, false);
  }
  return false;
}

} // namespace view
