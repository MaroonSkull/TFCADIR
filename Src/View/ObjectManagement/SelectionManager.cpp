#include "SelectionManager.hpp"
#include <Model/FlatFigure.hpp>
#include <View/ObjectManagement/CanvasHitTester.hpp>
#include <algorithm>
#include <cctype>
#include <set>

namespace view {

SelectionManager::SelectionManager(UIFSMAdapter &fsmAdapter,
                                   model::FlatFigures &model)
    : fsmAdapter_(fsmAdapter), model_(model) {}

std::vector<uint32_t> SelectionManager::getSelectedFigureIds() const {
  /// Delegate to UIFSMAdapter (stateless - no local storage)
  return fsmAdapter_.getSelectedFigureIds();
}

size_t SelectionManager::getSelectionCount() const {
  /// Delegate to UIFSMAdapter
  return fsmAdapter_.getSelectedFigureIds().size();
}

bool SelectionManager::isFigureSelected(uint32_t figureId) const {
  /// Check if figure is in the selected list
  auto selectedIds = fsmAdapter_.getSelectedFigureIds();
  return std::find(selectedIds.begin(), selectedIds.end(), figureId) !=
         selectedIds.end();
}

uint32_t SelectionManager::getPrimarySelectionId() const {
  /// Delegate to UIFSMAdapter
  return fsmAdapter_.getPrimarySelectionId();
}

int SelectionManager::getPrimarySelectionIndex() const {
  /// Delegate to UIFSMAdapter
  return fsmAdapter_.getPrimarySelectionIndex();
}

std::vector<std::shared_ptr<model::IFigure>>
SelectionManager::getSelectedFigures() const {
  /// Get figure objects for the selected IDs
  std::vector<std::shared_ptr<model::IFigure>> figures;
  auto selectedIds = fsmAdapter_.getSelectedFigureIds();

  for (uint32_t id : selectedIds) {
    auto figure = model_.getFigure(id);
    if (figure) {
      figures.push_back(figure);
    }
  }

  return figures;
}

void SelectionManager::selectFigure(uint32_t figureId) {
  /// Delegate to UIFSMAdapter
  fsmAdapter_.selectFigure(figureId);
}

void SelectionManager::toggleFigureSelection(uint32_t figureId) {
  /// Delegate to UIFSMAdapter
  fsmAdapter_.toggleFigureSelection(figureId);
}

void SelectionManager::addToSelection(uint32_t figureId) {
  /// Delegate to UIFSMAdapter
  fsmAdapter_.addToSelection(figureId);
}

void SelectionManager::removeFromSelection(uint32_t figureId) {
  /// Delegate to UIFSMAdapter
  fsmAdapter_.removeFromSelection(figureId);
}

void SelectionManager::clearSelection() {
  /// Delegate to UIFSMAdapter
  fsmAdapter_.clearSelection();
}

void SelectionManager::selectAll() {
  /// Add all figures in the model to selection
  size_t figureCount = model_.getFigureCount();
  fsmAdapter_.clearSelection();

  for (size_t i = 0; i < figureCount; ++i) {
    auto figure = model_.getFigure(static_cast<uint32_t>(i));
    if (figure) {
      fsmAdapter_.addToSelection(figure->getId());
    }
  }
}

void SelectionManager::setPrimarySelection(int index) {
  /// Delegate to UIFSMAdapter
  fsmAdapter_.setPrimarySelection(index);
}

#ifndef NDEBUG
bool SelectionManager::validateSelectionState() const {
  /// Validate selection state consistency (DEBUG builds only)
  auto selectedIds = fsmAdapter_.getSelectedFigureIds();
  int primaryIndex = fsmAdapter_.getPrimarySelectionIndex();

  /// Check that primary index is valid
  if (primaryIndex >= 0) {
    if (primaryIndex >= static_cast<int>(selectedIds.size())) {
      return false; /// Primary index out of bounds
    }
  }

  /// Check that all selected IDs exist in the model
  for (uint32_t id : selectedIds) {
    auto figure = model_.getFigure(id);
    if (!figure) {
      return false; /// Selected figure not found in model
    }
  }

  return true; /// Selection state is valid
}
#endif

// ==========================================================================
// Phase 10: Advanced Selection Methods
// ==========================================================================

size_t SelectionManager::selectByBox(CanvasHitTester &hitTester,
                                     const SelectionGeometry &geometry,
                                     SelectionModifier modifier,
                                     const SelectionFilter &filter,
                                     float viewportWidth, float viewportHeight,
                                     const glm::mat4 &modelView,
                                     const glm::mat4 &projection) {
  /// Perform box selection using hit tester
  auto [boxMin, boxMax] = geometry.getBoundingBox();

  if (boxMin.x >= boxMax.x || boxMin.y >= boxMax.y) {
    return 0; /// Invalid box
  }

  /// Get hits from hit tester
  std::vector<uint32_t> hits = hitTester.hitTestBox(
      boxMin.x, boxMin.y, boxMax.x, boxMax.y, viewportWidth, viewportHeight,
      modelView, projection);

  /// Apply filter
  std::vector<uint32_t> filteredHits = filterFigures(hits, filter);

  /// Apply modifier
  applySelectionModifier(filteredHits, modifier);

  return filteredHits.size();
}

size_t SelectionManager::selectByLasso(
    CanvasHitTester &hitTester, const SelectionGeometry &geometry,
    SelectionModifier modifier, const SelectionFilter &filter,
    float viewportWidth, float viewportHeight, const glm::mat4 &modelView,
    const glm::mat4 &projection) {
  /// Perform lasso selection using polygon hit testing
  if (geometry.points.size() < 3) {
    return 0; /// Need at least 3 points for lasso
  }

  /// Get hits from hit tester (intersection mode, not containment)
  std::vector<uint32_t> hits =
      hitTester.hitTestPolygon(geometry.points, viewportWidth, viewportHeight,
                               modelView, projection, false);

  /// Apply filter
  std::vector<uint32_t> filteredHits = filterFigures(hits, filter);

  /// Apply modifier
  applySelectionModifier(filteredHits, modifier);

  return filteredHits.size();
}

size_t SelectionManager::selectByPolygon(
    CanvasHitTester &hitTester, const SelectionGeometry &geometry,
    SelectionModifier modifier, const SelectionFilter &filter,
    float viewportWidth, float viewportHeight, const glm::mat4 &modelView,
    const glm::mat4 &projection) {
  /// Perform polygon selection (uses containment by default for precision)
  if (geometry.points.size() < 3) {
    return 0; /// Need at least 3 points for polygon
  }

  /// For polygon selection, use containment mode if right-to-left
  bool requireContainment = !geometry.isLeftToRight();

  /// Get hits from hit tester
  std::vector<uint32_t> hits =
      hitTester.hitTestPolygon(geometry.points, viewportWidth, viewportHeight,
                               modelView, projection, requireContainment);

  /// Apply filter
  std::vector<uint32_t> filteredHits = filterFigures(hits, filter);

  /// Apply modifier
  applySelectionModifier(filteredHits, modifier);

  return filteredHits.size();
}

void SelectionManager::applySelectionModifier(
    const std::vector<uint32_t> &figureIds, SelectionModifier modifier) {
  /// Apply selection modifier to the given figure IDs
  switch (modifier) {
  case SelectionModifier::None:
    /// Replace selection with new figures
    fsmAdapter_.clearSelection();
    for (uint32_t id : figureIds) {
      fsmAdapter_.addToSelection(id);
    }
    break;

  case SelectionModifier::Add:
    /// Add to existing selection
    for (uint32_t id : figureIds) {
      fsmAdapter_.addToSelection(id);
    }
    break;

  case SelectionModifier::Toggle:
    /// Toggle each figure's selection state
    for (uint32_t id : figureIds) {
      fsmAdapter_.toggleFigureSelection(id);
    }
    break;

  case SelectionModifier::Remove:
    /// Remove from existing selection
    for (uint32_t id : figureIds) {
      fsmAdapter_.removeFromSelection(id);
    }
    break;
  }
}

std::vector<uint32_t>
SelectionManager::filterFigures(const std::vector<uint32_t> &figureIds,
                                const SelectionFilter &filter) const {
  /// Filter figures based on type, layer, and property criteria
  std::vector<uint32_t> result;

  for (uint32_t id : figureIds) {
    if (passesFilter(id, filter)) {
      result.push_back(id);
    }
  }

  return result;
}

bool SelectionManager::passesFilter(uint32_t figureId,
                                    const SelectionFilter &filter) const {
  /// Check if a figure passes all filter criteria
  auto figure = model_.getFigure(figureId);
  if (!figure) {
    return false;
  }

  /// Check type filter
  if (filter.figureTypes != FigureTypeFilter::None &&
      filter.figureTypes != FigureTypeFilter::All) {
    FigureTypeFilter figureType = getFigureTypeFilter(figure);
    if (!hasFigureType(filter.figureTypes, figureType)) {
      return false;
    }
  }

  /// Check layer filter
  if (filter.hasLayerFilter()) {
    if (!filter.isLayerIncluded(figure->getLayer())) {
      return false;
    }
  }

  /// Check visibility filter
  if (filter.visibleOnly) {
    if (!figure->isVisible()) {
      return false;
    }
  }

  /// Check locked filter
  if (filter.excludeLocked) {
    if (figure->isLocked()) {
      return false;
    }
  }

  /// Check area filters
  if (filter.minArea > 0.0f || filter.maxArea > 0.0f) {
    /// Calculate figure area from bounding box
    auto [minBounds, maxBounds] = figure->getBounds();
    float width = maxBounds.x - minBounds.x;
    float height = maxBounds.y - minBounds.y;
    float area = width * height;

    /// Check minimum area filter
    if (filter.minArea > 0.0f && area < filter.minArea) {
      return false;
    }

    /// Check maximum area filter
    if (filter.maxArea > 0.0f && area > filter.maxArea) {
      return false;
    }
  }

  /// Check name pattern filter (supports wildcards with * and ?)
  if (!filter.namePattern.empty()) {
    const std::string &figureName = figure->getName();
    if (!matchNamePattern(figureName, filter.namePattern)) {
      return false;
    }
  }

  return true;
}

void SelectionManager::invertSelection(const SelectionFilter &filter) {
  /// Invert the current selection (select all unselected, deselect all
  /// selected)
  std::vector<uint32_t> currentlySelected = fsmAdapter_.getSelectedFigureIds();
  std::set<uint32_t> selectedSet(currentlySelected.begin(),
                                 currentlySelected.end());

  /// Clear current selection
  fsmAdapter_.clearSelection();

  /// Select all figures that were not previously selected (and pass filter)
  size_t figureCount = model_.getFigureCount();
  for (size_t i = 0; i < figureCount; ++i) {
    auto figure = model_.getFigure(static_cast<uint32_t>(i));
    if (figure) {
      uint32_t id = figure->getId();
      if (selectedSet.find(id) == selectedSet.end()) {
        /// Check if passes filter before adding
        if (passesFilter(id, filter)) {
          fsmAdapter_.addToSelection(id);
        }
      }
    }
  }
}

size_t SelectionManager::selectSimilar(const SelectionFilter &filter) {
  /// Select all figures similar to the currently selected figures
  auto selectedFigures = getSelectedFigures();
  if (selectedFigures.empty()) {
    return 0;
  }

  std::vector<uint32_t> similarFigures;

  /// Collect properties from all selected figures
  std::set<int> selectedLayers;
  std::set<FigureTypeFilter> selectedTypes;

  for (const auto &figure : selectedFigures) {
    selectedLayers.insert(figure->getLayer());
    selectedTypes.insert(getFigureTypeFilter(figure));
  }

  /// Iterate through all figures and find similar ones
  size_t figureCount = model_.getFigureCount();
  for (size_t i = 0; i < figureCount; ++i) {
    auto figure = model_.getFigure(static_cast<uint32_t>(i));
    if (!figure || isFigureSelected(figure->getId())) {
      continue;
    }

    bool similar = true;

    /// Check type similarity
    if (filter.figureTypes != FigureTypeFilter::None) {
      FigureTypeFilter figType = getFigureTypeFilter(figure);
      if (selectedTypes.find(figType) == selectedTypes.end()) {
        similar = false;
      }
    }

    /// Check layer similarity
    if (similar && filter.hasLayerFilter()) {
      if (selectedLayers.find(figure->getLayer()) == selectedLayers.end()) {
        similar = false;
      }
    }

    if (similar) {
      similarFigures.push_back(figure->getId());
    }
  }

  /// Add to selection
  for (uint32_t id : similarFigures) {
    fsmAdapter_.addToSelection(id);
  }

  return similarFigures.size();
}

// ==========================================================================
// Private Helper Methods
// ==========================================================================

glm::vec2 SelectionManager::screenToWorld(float screenX, float screenY,
                                          float viewportWidth,
                                          float viewportHeight,
                                          const glm::mat4 &modelView,
                                          const glm::mat4 &projection) const {
  /// Convert screen coordinates to world coordinates
  float glScreenY = viewportHeight - screenY;
  glm::vec4 viewport(0.0f, 0.0f, viewportWidth, viewportHeight);
  glm::vec3 worldPos = glm::unProject(glm::vec3(screenX, glScreenY, 0.0f),
                                      modelView, projection, viewport);
  return glm::vec2(worldPos.x, worldPos.y);
}

FigureTypeFilter
SelectionManager::getFigureTypeFilter(std::shared_ptr<model::IFigure> figure) {
  /// Get the figure type filter value for a figure
  if (std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    return FigureTypeFilter::Triangle;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::Quad>>(figure)) {
    return FigureTypeFilter::Quad;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::Circle>>(figure)) {
    return FigureTypeFilter::Circle;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::Ngon>>(figure)) {
    return FigureTypeFilter::Ngon;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(figure)) {
    return FigureTypeFilter::CurveBezier3;
  }
  if (std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(figure)) {
    return FigureTypeFilter::CurveBezier4;
  }
  return FigureTypeFilter::None;
}

bool SelectionManager::matchNamePattern(const std::string &name,
                                        const std::string &pattern) const {
  /// Match name against a pattern with wildcards (* and ?)
  /// Uses dynamic programming approach for pattern matching

  const size_t nameLen = name.size();
  const size_t patternLen = pattern.size();

  /// dp[i][j] = true if name[0..i-1] matches pattern[0..j-1]
  std::vector<std::vector<bool>> dp(nameLen + 1,
                                    std::vector<bool>(patternLen + 1, false));

  /// Empty pattern matches empty name
  dp[0][0] = true;

  /// Handle patterns like "*", "**", "***" etc.
  for (size_t j = 1; j <= patternLen; ++j) {
    if (pattern[j - 1] == '*') {
      dp[0][j] = dp[0][j - 1];
    }
  }

  /// Fill the DP table
  for (size_t i = 1; i <= nameLen; ++i) {
    for (size_t j = 1; j <= patternLen; ++j) {
      if (pattern[j - 1] == '*') {
        /// '*' can match zero or more characters
        dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
      } else if (pattern[j - 1] == '?') {
        /// '?' matches exactly one character
        dp[i][j] = dp[i - 1][j - 1];
      } else {
        /// Exact character match (case-insensitive)
        char nameChar = static_cast<char>(std::tolower(name[i - 1]));
        char patternChar = static_cast<char>(std::tolower(pattern[j - 1]));
        dp[i][j] = dp[i - 1][j - 1] && (nameChar == patternChar);
      }
    }
  }

  return dp[nameLen][patternLen];
}

} // namespace view
