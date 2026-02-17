#include "LayerManagerPanel.hpp"
#include <spdlog/spdlog.h>

namespace view {

// Static member definition
constexpr glm::vec3 LayerManagerPanel::predefinedColors_[];

LayerManagerPanel::LayerManagerPanel(LayerManager &layerManager,
                                     model::FlatFigures &model)
    : layerManager_(layerManager), model_(model), selectedLayerIndex_(0),
      cacheValid_(false), showRenameInput_(false), showNewLayerInput_(false),
      scrollToSelected_(false) {
  renameBuffer_[0] = '\0';
  newLayerBuffer_[0] = '\0';
}

void LayerManagerPanel::render() {
  ImGui::Begin("Layer Manager");

  // Update figure counts if cache is invalid
  if (!cacheValid_) {
    updateFigureCounts();
    cacheValid_ = true;
  }

  // Render layer list
  renderLayerList();

  ImGui::Separator();

  // Render layer properties
  renderLayerProperties();

  ImGui::Separator();

  // Render layer actions
  renderLayerActions();

  ImGui::End();
}

void LayerManagerPanel::renderLayerList() {
  ImGui::Text("Layers");
  ImGui::SameLine(ImGui::GetWindowWidth() - 60);
  if (ImGui::Button("+ Add")) {
    showNewLayerInput_ = true;
    snprintf(newLayerBuffer_, sizeof(newLayerBuffer_), "New Layer");
  }

  ImGui::BeginChild("LayerList", ImVec2(0, 150), true);

  const auto &layers = layerManager_.getAllLayers();
  int index = 0;

  for (const auto &layer : layers) {
    if (renderLayerItem(layer)) {
      selectedLayerIndex_ = index;
    }
    index++;
  }

  // Scroll to selected layer if requested
  if (scrollToSelected_) {
    ImGui::SetScrollHereY(0.5f);
    scrollToSelected_ = false;
  }

  ImGui::EndChild();
}

void LayerManagerPanel::renderLayerProperties() {
  auto selectedLayer = layerManager_.getLayer(selectedLayerIndex_);

  if (!selectedLayer) {
    ImGui::Text("No layer selected");
    return;
  }

  ImGui::Text("Properties: %s", selectedLayer->getName().c_str());

  // Color picker
  renderColorPicker(selectedLayer);

  // Line weight slider
  renderLineWeightSlider(selectedLayer);

  // Line style combo
  renderLineStyleCombo(selectedLayer);

  // Visibility and lock toggles
  bool visible = selectedLayer->isVisible();
  if (ImGui::Checkbox("Visible", &visible)) {
    selectedLayer->setVisible(visible);
    spdlog::info("[LayerManagerPanel] Layer '{}' visibility: {}",
                 selectedLayer->getName(), visible);
  }

  ImGui::SameLine();

  bool locked = selectedLayer->isLocked();
  if (ImGui::Checkbox("Locked", &locked)) {
    selectedLayer->setLocked(locked);
    spdlog::info("[LayerManagerPanel] Layer '{}' locked: {}",
                 selectedLayer->getName(), locked);
  }

  // Figure count
  ImGui::Text("Figures: %zu", selectedLayer->getFigureCount());
}

void LayerManagerPanel::renderLayerActions() {
  auto selectedLayer = layerManager_.getLayer(selectedLayerIndex_);

  // Set Active button
  if (selectedLayer &&
      selectedLayer->getIndex() != layerManager_.getActiveLayerIndex()) {
    if (ImGui::Button("Set Active")) {
      layerManager_.setActiveLayer(selectedLayer->getIndex());
      spdlog::info("[LayerManagerPanel] Active layer set to: {}",
                   selectedLayer->getName());
    }
    ImGui::SameLine();
  } else if (selectedLayer) {
    ImGui::Text("Active Layer");
    ImGui::SameLine();
  }

  // Rename button
  if (selectedLayer && selectedLayer->getName() != "Default") {
    if (!showRenameInput_) {
      if (ImGui::Button("Rename")) {
        showRenameInput_ = true;
        snprintf(renameBuffer_, sizeof(renameBuffer_), "%s",
                 selectedLayer->getName().c_str());
        ImGui::SetKeyboardFocusHere();
      }
    } else {
      ImGui::SetNextItemWidth(150);
      if (ImGui::InputText("##Rename", renameBuffer_, sizeof(renameBuffer_),
                           ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (strlen(renameBuffer_) > 0) {
          layerManager_.renameLayer(selectedLayer->getName(), renameBuffer_);
          showRenameInput_ = false;
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("OK")) {
        if (strlen(renameBuffer_) > 0) {
          layerManager_.renameLayer(selectedLayer->getName(), renameBuffer_);
          showRenameInput_ = false;
        }
      }
    }
    ImGui::SameLine();
  }

  // Delete button
  if (selectedLayer && selectedLayer->getName() != "Default") {
    if (ImGui::Button("Delete")) {
      std::string name = selectedLayer->getName();
      if (layerManager_.deleteLayer(name)) {
        selectedLayerIndex_ = 0; // Select default layer
      }
    }
  }

  // New layer input
  if (showNewLayerInput_) {
    ImGui::Separator();
    ImGui::Text("New Layer Name:");
    ImGui::SetNextItemWidth(150);
    if (ImGui::InputText("##NewLayer", newLayerBuffer_, sizeof(newLayerBuffer_),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      if (strlen(newLayerBuffer_) > 0) {
        auto newLayer = layerManager_.createLayer(newLayerBuffer_);
        if (newLayer) {
          // Select the new layer
          const auto &layers = layerManager_.getAllLayers();
          for (size_t i = 0; i < layers.size(); ++i) {
            if (layers[i] == newLayer) {
              selectedLayerIndex_ = static_cast<int>(i);
              break;
            }
          }
        }
        showNewLayerInput_ = false;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Create")) {
      if (strlen(newLayerBuffer_) > 0) {
        auto newLayer = layerManager_.createLayer(newLayerBuffer_);
        if (newLayer) {
          const auto &layers = layerManager_.getAllLayers();
          for (size_t i = 0; i < layers.size(); ++i) {
            if (layers[i] == newLayer) {
              selectedLayerIndex_ = static_cast<int>(i);
              break;
            }
          }
        }
        showNewLayerInput_ = false;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      showNewLayerInput_ = false;
    }
  }
}

bool LayerManagerPanel::renderLayerItem(const std::shared_ptr<Layer> &layer) {
  bool isSelected = (layer->getIndex() == selectedLayerIndex_);
  bool isActive = (layer->getIndex() == layerManager_.getActiveLayerIndex());

  // Build the label with icons
  std::string label;

  // Active indicator
  if (isActive) {
    label += "* ";
  }

  // Visibility icon
  label += layer->isVisible() ? "[V] " : "[ ] ";

  // Lock icon
  label += layer->isLocked() ? "[L] " : "    ";

  // Layer name
  label += layer->getName();

  // Figure count
  label += " (" + std::to_string(layer->getFigureCount()) + ")";

  // Color indicator
  glm::vec3 color = layer->getColor();
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1.0f));

  if (ImGui::Selectable(label.c_str(), isSelected)) {
    ImGui::PopStyleColor();
    return true;
  }

  ImGui::PopStyleColor();

  // Context menu on right-click
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Set Active")) {
      layerManager_.setActiveLayer(layer->getIndex());
    }
    if (layer->getName() != "Default") {
      if (ImGui::MenuItem("Rename")) {
        showRenameInput_ = true;
        snprintf(renameBuffer_, sizeof(renameBuffer_), "%s",
                 layer->getName().c_str());
      }
      if (ImGui::MenuItem("Delete")) {
        layerManager_.deleteLayer(layer->getName());
      }
    }
    ImGui::EndPopup();
  }

  return false;
}

void LayerManagerPanel::renderColorPicker(const std::shared_ptr<Layer> &layer) {
  glm::vec3 color = layer->getColor();
  float colorArray[3] = {color.r, color.g, color.b};

  ImGui::Text("Color:");
  ImGui::SameLine();

  // Color button
  ImGui::PushStyleColor(ImGuiCol_Button,
                        ImVec4(color.r, color.g, color.b, 1.0f));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonHovered,
      ImVec4(color.r * 1.1f, color.g * 1.1f, color.b * 1.1f, 1.0f));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonActive,
      ImVec4(color.r * 0.9f, color.g * 0.9f, color.b * 0.9f, 1.0f));

  if (ImGui::Button("##ColorButton", ImVec2(30, 20))) {
    ImGui::OpenPopup("ColorPicker");
  }

  ImGui::PopStyleColor(3);

  // Color picker popup
  if (ImGui::BeginPopup("ColorPicker")) {
    if (ImGui::ColorPicker3("##ColorPicker3", colorArray)) {
      layer->setColor(glm::vec3(colorArray[0], colorArray[1], colorArray[2]));
    }

    // Predefined colors
    ImGui::Text("Quick Colors:");
    for (size_t i = 0; i < 10; ++i) {
      const auto &pc = predefinedColors_[i];
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(pc.r, pc.g, pc.b, 1.0f));
      ImGui::PushID(static_cast<int>(i));
      if (ImGui::Button("", ImVec2(20, 20))) {
        layer->setColor(pc);
      }
      ImGui::PopID();
      ImGui::PopStyleColor();

      if ((i + 1) % 5 != 0) {
        ImGui::SameLine();
      }
    }

    ImGui::EndPopup();
  }
}

void LayerManagerPanel::renderLineWeightSlider(
    const std::shared_ptr<Layer> &layer) {
  float weight = layer->getLineWeight();

  ImGui::Text("Line Weight:");
  ImGui::SameLine();

  if (ImGui::SliderFloat("##LineWeight", &weight, 0.5f, 5.0f, "%.1f")) {
    layer->setLineWeight(weight);
  }
}

void LayerManagerPanel::renderLineStyleCombo(
    const std::shared_ptr<Layer> &layer) {
  model::LineStyle style = layer->getLineStyle();
  int currentStyle = static_cast<int>(style);

  ImGui::Text("Line Style:");
  ImGui::SameLine();

  const char *styleNames[] = {"Solid",    "Dashed", "Dotted",
                              "Dash-Dot", "Center", "Hidden"};

  if (ImGui::Combo("##LineStyle", &currentStyle, styleNames,
                   IM_ARRAYSIZE(styleNames))) {
    layer->setLineStyle(static_cast<model::LineStyle>(currentStyle));
  }
}

void LayerManagerPanel::updateFigureCounts() {
  // Collect layer indices from all figures
  std::vector<int> layerIndices;
  size_t figureCount = model_.getFigureCount();

  for (size_t i = 0; i < figureCount; ++i) {
    auto figure = model_.getFigure(static_cast<uint32_t>(i));
    if (figure) {
      layerIndices.push_back(figure->getLayer());
    }
  }

  layerManager_.updateFigureCounts(layerIndices);
}

const char *LayerManagerPanel::getLineStyleName(model::LineStyle style) const {
  switch (style) {
  case model::LineStyle::Solid:
    return "Solid";
  case model::LineStyle::Dashed:
    return "Dashed";
  case model::LineStyle::Dotted:
    return "Dotted";
  case model::LineStyle::DashDot:
    return "Dash-Dot";
  case model::LineStyle::Center:
    return "Center";
  case model::LineStyle::Hidden:
    return "Hidden";
  default:
    return "Unknown";
  }
}

} // namespace view
