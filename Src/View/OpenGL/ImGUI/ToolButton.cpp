#include "ToolButton.hpp"

namespace view::ImGUI {

ToolButton::ToolButton(const std::string &id, const std::string &label,
                       const std::string &icon, const std::string &shortcut,
                       const std::string &tooltip, ClickCallback callback)
    : id_(id), label_(label), icon_(icon), shortcut_(shortcut),
      tooltip_(tooltip), callback_(std::move(callback)), active_(false) {}

bool ToolButton::render() {
  // Push unique ID for this button
  ImGui::PushID(id_.c_str());

  // Apply active state styling
  if (active_) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.35f, 0.67f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.16f, 0.42f, 0.82f, 1.0f));
  } else {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.61f, 0.63f, 0.66f, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.68f, 0.70f, 0.74f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.47f, 0.54f, 0.58f, 0.64f));
  }

  // Render icon if present
  if (!icon_.empty()) {
    ImGui::TextUnformatted(icon_.c_str());
    ImGui::SameLine(0.0f, ImGui::GetFontSize());
  }

  // Render label if present
  if (!label_.empty()) {
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextUnformatted(label_.c_str());
  }

  // Render shortcut if present
  if (!shortcut_.empty()) {
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.8f));
    ImGui::TextUnformatted(shortcut_.c_str());
    ImGui::PopStyleColor();
  }

  // Restore styling
  ImGui::PopStyleColor(3);

  // Show tooltip on hover
  if (!tooltip_.empty() && ImGui::IsItemHovered()) {
    ImGui::SetTooltip(tooltip_.c_str());
  }

  // Execute callback if clicked
  bool clicked = ImGui::IsItemClicked();
  if (clicked && callback_) {
    callback_();
  }

  ImGui::PopID();

  return clicked;
}

void ToolButton::setActive(bool active) { active_ = active; }

bool ToolButton::isActive() const { return active_; }

const std::string &ToolButton::getId() const { return id_; }

const std::string &ToolButton::getLabel() const { return label_; }

void ToolButton::setCallback(ClickCallback callback) {
  callback_ = std::move(callback);
}

void ToolButton::renderContent() {
  // Render icon if present
  if (!icon_.empty()) {
    ImGui::TextUnformatted(icon_.c_str());
    ImGui::SameLine(0.0f, ImGui::GetFontSize());
  }

  // Render label if present
  if (!label_.empty()) {
    ImGui::TextUnformatted(label_.c_str());
  }

  // Render shortcut if present
  if (!shortcut_.empty()) {
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.8f));
    ImGui::TextUnformatted(shortcut_.c_str());
    ImGui::PopStyleColor();
  }
}

} // namespace view::ImGUI
