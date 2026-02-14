#include "Layer.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>

namespace view {

// ============================================================================
// Layer Implementation
// ============================================================================

Layer::Layer(const std::string &name, int index)
    : name_(name), index_(index), color_(1.0f, 1.0f, 1.0f), visible_(true),
      locked_(false), lineWeight_(1.0f), lineStyle_(model::LineStyle::Solid),
      figureCount_(0) {
  // Assign a default color based on index for visual distinction
  // Use a simple color palette
  static const glm::vec3 colorPalette[] = {
      glm::vec3(1.0f, 1.0f, 1.0f), // White (Default)
      glm::vec3(1.0f, 0.0f, 0.0f), // Red
      glm::vec3(0.0f, 1.0f, 0.0f), // Green
      glm::vec3(0.0f, 0.0f, 1.0f), // Blue
      glm::vec3(1.0f, 1.0f, 0.0f), // Yellow
      glm::vec3(1.0f, 0.0f, 1.0f), // Magenta
      glm::vec3(0.0f, 1.0f, 1.0f), // Cyan
      glm::vec3(1.0f, 0.5f, 0.0f), // Orange
      glm::vec3(0.5f, 0.0f, 1.0f), // Purple
      glm::vec3(0.0f, 0.5f, 0.0f), // Dark Green
  };

  if (index >= 0 && index < 10) {
    color_ = colorPalette[index];
  } else {
    // Generate a pseudo-random color for indices beyond the palette
    float hue = static_cast<float>(index % 360) / 360.0f;
    // Simple HSV to RGB conversion (S=0.7, V=1.0)
    float s = 0.7f;
    float v = 1.0f;
    int i = static_cast<int>(hue * 6.0f);
    float f = hue * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    switch (i % 6) {
    case 0:
      color_ = glm::vec3(v, t, p);
      break;
    case 1:
      color_ = glm::vec3(q, v, p);
      break;
    case 2:
      color_ = glm::vec3(p, v, t);
      break;
    case 3:
      color_ = glm::vec3(p, q, v);
      break;
    case 4:
      color_ = glm::vec3(t, p, v);
      break;
    case 5:
      color_ = glm::vec3(v, p, q);
      break;
    }
  }
}

// ============================================================================
// LayerManager Implementation
// ============================================================================

LayerManager::LayerManager() : activeLayerIndex_(0), nextLayerIndex_(0) {
  // Create the default layer
  auto defaultLayer = std::make_shared<Layer>("Default", 0);
  layers_.push_back(defaultLayer);
  nextLayerIndex_ = 1;

  spdlog::info("[LayerManager] Created with default layer");
}

std::shared_ptr<Layer> LayerManager::createLayer(const std::string &name) {
  // Check if name already exists
  if (hasLayer(name)) {
    spdlog::warn("[LayerManager] Layer '{}' already exists", name);
    return nullptr;
  }

  std::string uniqueName = name.empty() ? generateUniqueName("Layer") : name;

  auto layer = std::make_shared<Layer>(uniqueName, nextLayerIndex_++);
  layers_.push_back(layer);

  spdlog::info("[LayerManager] Created layer '{}' with index {}", uniqueName,
               layer->getIndex());
  return layer;
}

bool LayerManager::deleteLayer(const std::string &name) {
  // Cannot delete the default layer
  if (name == "Default") {
    spdlog::warn("[LayerManager] Cannot delete the default layer");
    return false;
  }

  auto it = std::find_if(layers_.begin(), layers_.end(),
                         [&name](const std::shared_ptr<Layer> &layer) {
                           return layer->getName() == name;
                         });

  if (it == layers_.end()) {
    spdlog::warn("[LayerManager] Layer '{}' not found", name);
    return false;
  }

  int deletedIndex = (*it)->getIndex();

  // If this was the active layer, switch to default
  if (activeLayerIndex_ == deletedIndex) {
    activeLayerIndex_ = 0;
  }

  layers_.erase(it);

  spdlog::info("[LayerManager] Deleted layer '{}'", name);
  return true;
}

bool LayerManager::renameLayer(const std::string &oldName,
                               const std::string &newName) {
  if (newName.empty()) {
    spdlog::warn("[LayerManager] New name cannot be empty");
    return false;
  }

  // Check if new name already exists
  if (hasLayer(newName)) {
    spdlog::warn("[LayerManager] Layer '{}' already exists", newName);
    return false;
  }

  auto it = std::find_if(layers_.begin(), layers_.end(),
                         [&oldName](const std::shared_ptr<Layer> &layer) {
                           return layer->getName() == oldName;
                         });

  if (it == layers_.end()) {
    spdlog::warn("[LayerManager] Layer '{}' not found", oldName);
    return false;
  }

  (*it)->setName(newName);

  spdlog::info("[LayerManager] Renamed layer '{}' to '{}'", oldName, newName);
  return true;
}

std::shared_ptr<Layer> LayerManager::getLayer(const std::string &name) const {
  auto it = std::find_if(layers_.begin(), layers_.end(),
                         [&name](const std::shared_ptr<Layer> &layer) {
                           return layer->getName() == name;
                         });

  return it != layers_.end() ? *it : nullptr;
}

std::shared_ptr<Layer> LayerManager::getLayer(int index) const {
  auto it = std::find_if(layers_.begin(), layers_.end(),
                         [index](const std::shared_ptr<Layer> &layer) {
                           return layer->getIndex() == index;
                         });

  return it != layers_.end() ? *it : nullptr;
}

std::vector<std::string> LayerManager::getLayerNames() const {
  std::vector<std::string> names;
  names.reserve(layers_.size());

  for (const auto &layer : layers_) {
    names.push_back(layer->getName());
  }

  return names;
}

bool LayerManager::setActiveLayer(const std::string &name) {
  auto layer = getLayer(name);
  if (!layer) {
    spdlog::warn("[LayerManager] Layer '{}' not found", name);
    return false;
  }

  activeLayerIndex_ = layer->getIndex();
  spdlog::info("[LayerManager] Active layer set to '{}'", name);
  return true;
}

bool LayerManager::setActiveLayer(int index) {
  auto layer = getLayer(index);
  if (!layer) {
    spdlog::warn("[LayerManager] Layer with index {} not found", index);
    return false;
  }

  activeLayerIndex_ = index;
  spdlog::info("[LayerManager] Active layer set to index {}", index);
  return true;
}

std::shared_ptr<Layer> LayerManager::getActiveLayer() const {
  return getLayer(activeLayerIndex_);
}

bool LayerManager::moveLayerUp(int index) {
  if (index <= 0 || index >= static_cast<int>(layers_.size())) {
    return false;
  }

  // Find the layer at this position
  auto it = std::find_if(layers_.begin(), layers_.end(),
                         [index](const std::shared_ptr<Layer> &layer) {
                           return layer->getIndex() == index;
                         });

  if (it == layers_.end() || it == layers_.begin()) {
    return false;
  }

  // Swap with previous layer
  std::iter_swap(it, std::prev(it));

  spdlog::info("[LayerManager] Moved layer at index {} up", index);
  return true;
}

bool LayerManager::moveLayerDown(int index) {
  if (index < 0 || index >= static_cast<int>(layers_.size()) - 1) {
    return false;
  }

  // Find the layer at this position
  auto it = std::find_if(layers_.begin(), layers_.end(),
                         [index](const std::shared_ptr<Layer> &layer) {
                           return layer->getIndex() == index;
                         });

  if (it == layers_.end() || std::next(it) == layers_.end()) {
    return false;
  }

  // Swap with next layer
  std::iter_swap(it, std::next(it));

  spdlog::info("[LayerManager] Moved layer at index {} down", index);
  return true;
}

bool LayerManager::hasLayer(const std::string &name) const {
  return std::any_of(layers_.begin(), layers_.end(),
                     [&name](const std::shared_ptr<Layer> &layer) {
                       return layer->getName() == name;
                     });
}

std::shared_ptr<Layer> LayerManager::getDefaultLayer() const {
  return getLayer(0);
}

void LayerManager::updateFigureCounts(
    const std::vector<int> &figureLayerIndices) {
  // Reset all counts
  for (auto &layer : layers_) {
    layer->setFigureCount(0);
  }

  // Count figures per layer
  for (int layerIndex : figureLayerIndices) {
    auto layer = getLayer(layerIndex);
    if (layer) {
      layer->setFigureCount(layer->getFigureCount() + 1);
    }
  }
}

std::string
LayerManager::generateUniqueName(const std::string &baseName) const {
  std::string name = baseName;
  int counter = 1;

  while (hasLayer(name)) {
    name = baseName + " " + std::to_string(counter);
    counter++;
  }

  return name;
}

} // namespace view
