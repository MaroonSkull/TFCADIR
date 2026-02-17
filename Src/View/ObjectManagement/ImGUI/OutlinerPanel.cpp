#include "OutlinerPanel.hpp"

#include <algorithm>
#include <cctype>

namespace view {

/**
 * @brief Constructor for OutlinerPanel
 * @param fsmAdapter Reference to the UIFSMAdapter for state access
 * @param selectionManager Reference to the SelectionManager for selection
 * operations
 * @param model Reference to the FlatFigures model for figure access
 */
OutlinerPanel::OutlinerPanel(UIFSMAdapter &fsmAdapter,
                             SelectionManager &selectionManager,
                             model::FlatFigures &model)
    : fsmAdapter_(fsmAdapter), selectionManager_(selectionManager),
      model_(model), treeCache_{}, filterText_{}, renameBuffer_{},
      renamingFigureId_(0), showContextMenu_(false) {
  // Register callbacks for state change notifications
  fsmAdapter_.setSelectionChangedCallback(
      [this](const std::vector<uint32_t> &ids) { this->onSelectionChanged(); });

  fsmAdapter_.setPropertyChangedCallback(
      [this](uint32_t id, const std::string &path) {
        this->onPropertyChanged(id, path);
      });
}

/**
 * @brief Renders the outliner panel
 */
void OutlinerPanel::render() {
  if (!ImGui::Begin("Outliner")) {
    ImGui::End();
    return;
  }

  // Render filter text box
  renderFilter();

  // Update cached tree data if needed (100ms rate limiting)
  updateCacheIfNeeded();

  // Render figure list
  renderFigureList();

  // Render context menu if needed
  if (showContextMenu_) {
    ImGui::OpenPopup("OutlinerContextMenu");
    showContextMenu_ = false;
  }

  // Handle context menu rendering
  if (ImGui::BeginPopup("OutlinerContextMenu")) {
    auto selectedIds = selectionManager_.getSelectedFigureIds();
    if (!selectedIds.empty()) {
      renderContextMenu(selectedIds[0]);
    }
    ImGui::EndPopup();
  }

  ImGui::End();
}

/**
 * @brief Invalidates the cached tree data
 */
void OutlinerPanel::invalidateCache() { treeCache_.isValid = false; }

/**
 * @brief Updates the cached tree data if needed
 * @details Only rebuilds cache if invalid or 100ms has elapsed since last
 * update
 */
void OutlinerPanel::updateCacheIfNeeded() {
  auto now = std::chrono::steady_clock::now();

  // Check if cache needs updating (invalid or 100ms elapsed)
  bool needsUpdate = !treeCache_.isValid;
  if (treeCache_.isValid) {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - treeCache_.lastUpdate);
    needsUpdate = elapsed >= CACHE_UPDATE_INTERVAL;
  }

  if (needsUpdate) {
    treeCache_.treeNodes = buildTreeData();
    treeCache_.lastUpdate = now;
    treeCache_.isValid = true;
  }
}

/**
 * @brief Gets the figure type name using dynamic_cast
 * @param figure The figure to check
 * @return The type name string
 */
std::string getFigureTypeName(std::shared_ptr<model::IFigure> figure) {
  if (std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    return model::Triangle::name;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
    return model::Quad::name;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
    return model::Circle::name;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
    return model::Ngon::name;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(figure)) {
    return model::CurveBezier3::name;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(figure)) {
    return model::CurveBezier4::name;
  }
  return "Unknown";
}

/**
 * @brief Builds the tree data from the current scene state
 * @return Vector of TreeNodeData representing all figures in the scene
 */
std::vector<OutlinerPanel::TreeNodeData> OutlinerPanel::buildTreeData() {
  std::vector<TreeNodeData> nodes;
  auto selectedIds = selectionManager_.getSelectedFigureIds();
  uint32_t primaryId = selectionManager_.getPrimarySelectionId();

  size_t figureCount = model_.getFigureCount();
  for (size_t i = 0; i < figureCount; ++i) {
    auto figure = model_.getFigure(i);
    if (!figure) {
      continue;
    }

    TreeNodeData node;
    node.figureId =
        i; // Use index as ID for compatibility with selection manager
    node.name = figure->getName();
    node.typeName = getFigureTypeName(figure);
    node.isSelected = std::find(selectedIds.begin(), selectedIds.end(), i) !=
                      selectedIds.end();
    node.isPrimarySelection = (i == primaryId);

    nodes.push_back(std::move(node));
  }

  return nodes;
}

/**
 * @brief Renders the filter text input
 */
void OutlinerPanel::renderFilter() {
  ImGui::Text("Filter:");
  ImGui::SameLine();

  char buffer[256];
  std::strncpy(buffer, filterText_.c_str(), sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  if (ImGui::InputText("##Filter", buffer, sizeof(buffer))) {
    filterText_ = buffer;
    invalidateCache(); // Filter changed, rebuild cache
  }
}

/**
 * @brief Renders the tree/list view of figures
 */
void OutlinerPanel::renderFigureList() {
  if (ImGui::BeginChild("FigureList", ImVec2(0, 0), true)) {
    for (const auto &node : treeCache_.treeNodes) {
      if (!matchesFilter(node.name)) {
        continue;
      }

      bool isRenaming = (renamingFigureId_ == node.figureId);

      if (isRenaming) {
        // Render rename input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##Rename", renameBuffer_, sizeof(renameBuffer_),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
          // Apply rename
          auto figure = model_.getFigure(node.figureId);
          if (figure) {
            figure->setName(renameBuffer_);
          }
          renamingFigureId_ = 0;
          invalidateCache();
        }

        // Cancel rename on focus loss or Escape
        if (!ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
          renamingFigureId_ = 0;
        }
      } else {
        // Render selectable item
        renderFigureItem(node);
      }
    }
  }
  ImGui::EndChild();
}

/**
 * @brief Renders the context menu for a figure
 * @param figureId The ID of the figure to show context menu for
 */
void OutlinerPanel::renderContextMenu(uint32_t figureId) {
  auto figure = model_.getFigure(figureId);
  if (!figure) {
    return;
  }

  bool selected = selectionManager_.isFigureSelected(figureId);

  if (ImGui::MenuItem("Select", nullptr, selected, !selected)) {
    selectionManager_.selectFigure(figureId);
  }

  if (ImGui::MenuItem("Delete")) {
    handleDelete();
  }

  if (ImGui::MenuItem("Rename")) {
    handleRename(figureId);
  }

  // Properties will be handled by PropertyInspectorPanel
  if (ImGui::MenuItem("Properties")) {
    // Ensure the figure is selected so properties panel shows it
    if (!selected) {
      selectionManager_.selectFigure(figureId);
    }
  }
}

/**
 * @brief Renders a single selectable item for a figure
 * @param node The tree node data to render
 * @return true if the item was clicked
 */
bool OutlinerPanel::renderFigureItem(const TreeNodeData &node) {
  ImGui::PushID(static_cast<int>(node.figureId));

  // Get icon for figure type
  const char *icon = getTypeIcon(node.typeName);

  // Build label text
  std::string label = std::string(icon) + " " + node.name;

  // Render selectable
  int flags = ImGuiSelectableFlags_None;
  if (node.isPrimarySelection) {
    flags |=
        ImGuiSelectableFlags_AllowDoubleClick; // Enable double-click for rename
  }

  bool clicked = ImGui::Selectable(label.c_str(), node.isSelected, flags);

  // Handle double-click for rename
  if (clicked && ImGui::IsMouseDoubleClicked(0) && node.isPrimarySelection) {
    handleRename(node.figureId);
  }

  // Handle selection
  if (clicked) {
    bool isCtrlPressed = ImGui::GetIO().KeyCtrl;
    bool isShiftPressed = ImGui::GetIO().KeyShift;
    handleSelection(node.figureId, isCtrlPressed, isShiftPressed);
  }

  // Handle right-click for context menu
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    if (!node.isSelected) {
      selectionManager_.selectFigure(node.figureId);
    }
    showContextMenu_ = true;
  }

  ImGui::PopID();
  return clicked;
}

/**
 * @brief Gets the icon for a figure type
 * @param typeName The name of the figure type
 * @return Icon character or emoji for the figure type
 */
const char *OutlinerPanel::getTypeIcon(const std::string &typeName) const {
  if (typeName == "Triangle")
    return "🔺";
  if (typeName == "Quad")
    return "🔲";
  if (typeName == "Circle")
    return "⭕";
  if (typeName == "Ngon")
    return "⬡";
  if (typeName == "CurveBezier3")
    return "〰️";
  if (typeName == "CurveBezier4")
    return "〰️";
  return "📦"; // Default icon
}

/**
 * @brief Checks if a figure matches the current filter
 * @param name The figure name to check
 * @return true if the figure matches the filter text
 */
bool OutlinerPanel::matchesFilter(const std::string &name) const {
  if (filterText_.empty()) {
    return true;
  }

  // Case-insensitive substring match
  std::string lowerName = name;
  std::string lowerFilter = filterText_;

  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  return lowerName.find(lowerFilter) != std::string::npos;
}

/**
 * @brief Handles selection state changes
 * @param figureId The ID of the figure that was clicked
 * @param isCtrlPressed Whether Ctrl key is held (for multi-selection)
 * @param isShiftPressed Whether Shift key is held (for range selection)
 */
void OutlinerPanel::handleSelection(uint32_t figureId, bool isCtrlPressed,
                                    bool isShiftPressed) {
  if (isCtrlPressed) {
    // Toggle selection
    selectionManager_.toggleFigureSelection(figureId);
  } else if (isShiftPressed) {
    // Range selection (not fully implemented for MVP)
    // For now, just add to selection
    selectionManager_.addToSelection(figureId);
  } else {
    // Single selection
    selectionManager_.selectFigure(figureId);
  }
}

/**
 * @brief Handles double-click to rename a figure
 * @param figureId The ID of the figure to rename
 */
void OutlinerPanel::handleRename(uint32_t figureId) {
  auto figure = model_.getFigure(figureId);
  if (!figure) {
    return;
  }

  renamingFigureId_ = figureId;
  std::strncpy(renameBuffer_, figure->getName().c_str(),
               sizeof(renameBuffer_) - 1);
  renameBuffer_[sizeof(renameBuffer_) - 1] = '\0';
}

/**
 * @brief Handles delete operation on selected figures
 */
void OutlinerPanel::handleDelete() {
  auto selectedIds = selectionManager_.getSelectedFigureIds();

  // Delete in reverse order to avoid index shifting issues
  std::vector<uint32_t> sortedIds(selectedIds.begin(), selectedIds.end());
  std::sort(sortedIds.rbegin(), sortedIds.rend());

  for (uint32_t id : sortedIds) {
    model_.removeFigure(id);
  }

  // Clear selection after delete
  selectionManager_.clearSelection();
  invalidateCache();
}

/**
 * @brief Callback for when selection changes
 */
void OutlinerPanel::onSelectionChanged() {
  invalidateCache(); // Selection changed, need to rebuild cache
}

/**
 * @brief Callback for when properties change
 * @param figureId The ID of the figure that changed
 * @param propertyPath The path of the property that changed
 */
void OutlinerPanel::onPropertyChanged(uint32_t figureId,
                                      const std::string &propertyPath) {
  // If name changed, invalidate cache to update display
  if (propertyPath == "name") {
    invalidateCache();
  }
}

} // namespace view
