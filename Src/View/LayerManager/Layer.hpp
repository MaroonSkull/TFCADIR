#pragma once

#include <Model/IModel.hpp>
#include <glm/glm.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace view {

/**
 * @brief Represents a single layer in the CAD application
 * @details A layer contains properties for visibility, lock state, color,
 *          line weight, and line style. Figures can be assigned to layers.
 */
class Layer {
public:
  /**
   * @brief Constructs a Layer with the given name and index
   * @param name The display name of the layer
   * @param index The unique index of the layer
   */
  Layer(const std::string &name, int index);

  /// Gets the layer name
  const std::string &getName() const { return name_; }

  /// Sets the layer name
  void setName(const std::string &name) { name_ = name; }

  /// Gets the layer index
  int getIndex() const { return index_; }

  /// Sets the layer index
  void setIndex(int index) { index_ = index; }

  /// Checks if the layer is visible
  bool isVisible() const { return visible_; }

  /// Sets the layer visibility
  void setVisible(bool visible) { visible_ = visible; }

  /// Checks if the layer is locked
  bool isLocked() const { return locked_; }

  /// Sets the layer lock state
  void setLocked(bool locked) { locked_ = locked; }

  /// Gets the layer color
  const glm::vec3 &getColor() const { return color_; }

  /// Sets the layer color
  void setColor(const glm::vec3 &color) { color_ = color; }

  /// Gets the line weight
  float getLineWeight() const { return lineWeight_; }

  /// Sets the line weight
  void setLineWeight(float weight) { lineWeight_ = weight; }

  /// Gets the line style
  model::LineStyle getLineStyle() const { return lineStyle_; }

  /// Sets the line style
  void setLineStyle(model::LineStyle style) { lineStyle_ = style; }

  /// Gets the number of figures in this layer
  size_t getFigureCount() const { return figureCount_; }

  /// Sets the number of figures in this layer
  void setFigureCount(size_t count) { figureCount_ = count; }

private:
  std::string name_;           ///< Display name of the layer
  int index_;                  ///< Unique index of the layer
  bool visible_;               ///< Whether the layer is visible
  bool locked_;                ///< Whether the layer is locked
  glm::vec3 color_;            ///< RGB color for the layer
  float lineWeight_;           ///< Line thickness (0.5-5.0)
  model::LineStyle lineStyle_; ///< Line style (solid, dashed, etc.)
  size_t figureCount_;         ///< Number of figures in this layer
};

/**
 * @brief Manages all layers in the CAD application
 * @details Provides CRUD operations for layers, tracks the active layer,
 *          and manages layer-figure associations.
 */
class LayerManager {
public:
  /**
   * @brief Constructs a LayerManager with a default layer
   */
  LayerManager();

  /**
   * @brief Creates a new layer with the given name
   * @param name The name for the new layer
   * @return Shared pointer to the created layer, or nullptr if name exists
   */
  std::shared_ptr<Layer> createLayer(const std::string &name);

  /**
   * @brief Deletes a layer by name
   * @param name The name of the layer to delete
   * @return true if the layer was deleted, false if not found or is default
   */
  bool deleteLayer(const std::string &name);

  /**
   * @brief Renames a layer
   * @param oldName The current name of the layer
   * @param newName The new name for the layer
   * @return true if renamed successfully, false if not found or name exists
   */
  bool renameLayer(const std::string &oldName, const std::string &newName);

  /**
   * @brief Gets a layer by name
   * @param name The name of the layer
   * @return Shared pointer to the layer, or nullptr if not found
   */
  std::shared_ptr<Layer> getLayer(const std::string &name) const;

  /**
   * @brief Gets a layer by index
   * @param index The index of the layer
   * @return Shared pointer to the layer, or nullptr if not found
   */
  std::shared_ptr<Layer> getLayer(int index) const;

  /**
   * @brief Gets all layers
   * @return Vector of all layers
   */
  const std::vector<std::shared_ptr<Layer>> &getAllLayers() const {
    return layers_;
  }

  /**
   * @brief Gets the active layer index
   * @return The index of the active layer
   */
  int getActiveLayerIndex() const { return activeLayerIndex_; }

  /**
   * @brief Sets the active layer by name
   * @param name The name of the layer to set as active
   * @return true if successful, false if layer not found
   */
  bool setActiveLayer(const std::string &name);

  /**
   * @brief Sets the active layer by index
   * @param index The index of the layer to set as active
   * @return true if successful, false if layer not found
   */
  bool setActiveLayer(int index);

  /**
   * @brief Gets the active layer
   * @return Shared pointer to the active layer
   */
  std::shared_ptr<Layer> getActiveLayer() const;

  /**
   * @brief Gets all layer names
   * @return Vector of layer names
   */
  std::vector<std::string> getLayerNames() const;

  /**
   * @brief Updates figure counts for all layers
   * @param layerIndices Vector of layer indices from all figures
   */
  void updateFigureCounts(const std::vector<int> &layerIndices);

  /**
   * @brief Gets the number of layers
   * @return The total number of layers
   */
  size_t getLayerCount() const { return layers_.size(); }

  /**
   * @brief Moves a layer up in the order
   * @param index The index of the layer to move
   * @return true if successful, false if already at top or not found
   */
  bool moveLayerUp(int index);

  /**
   * @brief Moves a layer down in the order
   * @param index The index of the layer to move
   * @return true if successful, false if already at bottom or not found
   */
  bool moveLayerDown(int index);

  /**
   * @brief Checks if a layer with the given name exists
   * @param name The name to check
   * @return true if layer exists, false otherwise
   */
  bool hasLayer(const std::string &name) const;

  /**
   * @brief Gets the default layer
   * @return Shared pointer to the default layer
   */
  std::shared_ptr<Layer> getDefaultLayer() const;

private:
  /**
   * @brief Generates a unique layer name from a base name
   * @param baseName The base name to use
   * @return A unique layer name
   */
  std::string generateUniqueName(const std::string &baseName) const;

  std::vector<std::shared_ptr<Layer>> layers_; ///< All layers
  int activeLayerIndex_;                       ///< Currently active layer index
  int nextLayerIndex_;                         ///< Next unique layer index
};

} // namespace view
