#pragma once

#include "../SelectionManager.hpp"
#include "../../UIFSMAdapter.hpp"
#include "../../../Model/FlatFigure.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

namespace view {

/**
 * @brief Panel for displaying and managing scene hierarchy
 * @details Provides a flat list view of all figures in the scene with
 *          selection management, filtering, and context menu operations.
 *          Uses performance caching with dirty flag + 100ms rate limiting.
 */
class OutlinerPanel {
public:
  /**
   * @brief Constructs an OutlinerPanel
   * @param fsmAdapter Reference to the UIFSMAdapter for state access
   * @param selectionManager Reference to the SelectionManager for selection operations
   * @param model Reference to the FlatFigures model for figure access
   */
  OutlinerPanel(UIFSMAdapter& fsmAdapter, SelectionManager& selectionManager, model::FlatFigures& model);

  /**
   * @brief Renders the outliner panel
   */
  void render();

  /**
   * @brief Invalidates the cached tree data
   * @details Called when the scene changes to force a refresh on next render
   */
  void invalidateCache();

private:
  /**
   * @brief Data structure for tree node information
   */
  struct TreeNodeData {
    uint32_t figureId;
    std::string name;
    std::string typeName;
    bool isSelected;
    bool isPrimarySelection;
  };

  /**
   * @brief Cache structure for performance optimization
   * @details v1.3 fix: Added lastUpdate field for 100ms rate limiting
   */
  struct OutlinerCache {
    std::vector<TreeNodeData> treeNodes;
    std::chrono::steady_clock::time_point lastUpdate; // v1.3 FIX
    bool isValid = false;
  };

  /**
   * @brief Updates the cached tree data if needed
   * @details Only rebuilds cache if invalid or 100ms has elapsed since last update
   */
  void updateCacheIfNeeded();

  /**
   * @brief Builds the tree data from the current scene state
   * @return Vector of TreeNodeData representing all figures in the scene
   */
  std::vector<TreeNodeData> buildTreeData();

  /**
   * @brief Renders the filter text input
   */
  void renderFilter();

  /**
   * @brief Renders the tree/list view of figures
   */
  void renderFigureList();

  /**
   * @brief Renders the context menu for a figure
   * @param figureId The ID of the figure to show context menu for
   */
  void renderContextMenu(uint32_t figureId);

  /**
   * @brief Renders a single selectable item for a figure
   * @param node The tree node data to render
   * @return true if the item was clicked
   */
  bool renderFigureItem(const TreeNodeData& node);

  /**
   * @brief Gets the icon for a figure type
   * @param typeName The name of the figure type
   * @return Icon character or emoji for the figure type
   */
  const char* getTypeIcon(const std::string& typeName) const;

  /**
   * @brief Checks if a figure matches the current filter
   * @param name The figure name to check
   * @return true if the figure matches the filter text
   */
  bool matchesFilter(const std::string& name) const;

  /**
   * @brief Handles selection state changes
   * @param figureId The ID of the figure that was clicked
   * @param isCtrlPressed Whether Ctrl key is held (for multi-selection)
   * @param isShiftPressed Whether Shift key is held (for range selection)
   */
  void handleSelection(uint32_t figureId, bool isCtrlPressed, bool isShiftPressed);

  /**
   * @brief Handles double-click to rename a figure
   * @param figureId The ID of the figure to rename
   */
  void handleRename(uint32_t figureId);

  /**
   * @brief Handles delete operation on selected figures
   */
  void handleDelete();

  /**
   * @brief Callback for when selection changes
   */
  void onSelectionChanged();

  /**
   * @brief Callback for when properties change
   * @param figureId The ID of the figure that changed
   * @param propertyPath The path of the property that changed
   */
  void onPropertyChanged(uint32_t figureId, const std::string& propertyPath);

  UIFSMAdapter& fsmAdapter_;
  SelectionManager& selectionManager_;
  model::FlatFigures& model_;

  OutlinerCache treeCache_;
  std::string filterText_;
  char renameBuffer_[256];
  uint32_t renamingFigureId_;
  bool showContextMenu_;

  static constexpr std::chrono::milliseconds CACHE_UPDATE_INTERVAL = std::chrono::milliseconds(100);
};

} // namespace view
