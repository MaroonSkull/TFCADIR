#pragma once

#include "Layer.hpp"
#include <Model/FlatFigure.hpp>
#include <View/UIFSMAdapter.hpp>

#include <memory>
#include <string>

#include <imgui.h>

namespace view {

/**
 * @brief ImGUI-based panel for layer management
 * @details Provides a UI for creating, deleting, renaming layers,
 *          setting layer properties, and managing the active layer.
 */
class LayerManagerPanel {
public:
  /**
   * @brief Constructs a LayerManagerPanel
   * @param layerManager Reference to the LayerManager for layer operations
   * @param model Reference to the FlatFigures model for figure access
   */
  LayerManagerPanel(LayerManager &layerManager, model::FlatFigures &model);

  /**
   * @brief Renders the layer manager panel
   */
  void render();

  /**
   * @brief Invalidates the cached layer data
   */
  void invalidateCache() { cacheValid_ = false; }

private:
  /**
   * @brief Renders the layer list with visibility/lock toggles
   */
  void renderLayerList();

  /**
   * @brief Renders the layer properties section
   */
  void renderLayerProperties();

  /**
   * @brief Renders the layer action buttons (create, delete, rename)
   */
  void renderLayerActions();

  /**
   * @brief Renders a single layer item in the list
   * @param layer The layer to render
   * @return true if this layer is selected
   */
  bool renderLayerItem(const std::shared_ptr<Layer> &layer);

  /**
   * @brief Renders a color picker for layer color
   * @param layer The layer to edit
   */
  void renderColorPicker(const std::shared_ptr<Layer> &layer);

  /**
   * @brief Renders line weight slider
   * @param layer The layer to edit
   */
  void renderLineWeightSlider(const std::shared_ptr<Layer> &layer);

  /**
   * @brief Renders line style combo box
   * @param layer The layer to edit
   */
  void renderLineStyleCombo(const std::shared_ptr<Layer> &layer);

  /**
   * @brief Updates the figure counts for all layers
   */
  void updateFigureCounts();

  /**
   * @brief Gets the line style name as a string
   * @param style The line style
   * @return String representation of the line style
   */
  const char *getLineStyleName(model::LineStyle style) const;

  LayerManager &layerManager_;
  model::FlatFigures &model_;

  int selectedLayerIndex_;   ///< Index of the selected layer in the UI
  bool cacheValid_;          ///< Whether the cache is valid
  char renameBuffer_[256];   ///< Buffer for layer rename input
  char newLayerBuffer_[256]; ///< Buffer for new layer name input
  bool showRenameInput_;     ///< Whether to show rename input
  bool showNewLayerInput_;   ///< Whether to show new layer input
  bool scrollToSelected_;    ///< Whether to scroll to selected layer

  /// Predefined colors for quick selection
  static constexpr glm::vec3 predefinedColors_[] = {
      glm::vec3(1.0f, 1.0f, 1.0f), // White
      glm::vec3(1.0f, 0.0f, 0.0f), // Red
      glm::vec3(0.0f, 1.0f, 0.0f), // Green
      glm::vec3(0.0f, 0.0f, 1.0f), // Blue
      glm::vec3(1.0f, 1.0f, 0.0f), // Yellow
      glm::vec3(1.0f, 0.0f, 1.0f), // Magenta
      glm::vec3(0.0f, 1.0f, 1.0f), // Cyan
      glm::vec3(1.0f, 0.5f, 0.0f), // Orange
      glm::vec3(0.5f, 0.0f, 1.0f), // Purple
      glm::vec3(0.5f, 0.5f, 0.5f), // Gray
  };
};

} // namespace view
