#include "SelectionManager.hpp"
#include <Model/FlatFigure.hpp>
#include <View/ObjectManagement/CanvasHitTester.hpp>
#include <algorithm>
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

bool SelectionManager::isPointInPolygon(const glm::vec2 &point,
                                        const std::vector<glm::vec2> &polygon) {
  /// Ray casting algorithm for point-in-polygon test
  if (polygon.size() < 3) {
    return false;
  }

  int intersections = 0;
  size_t n = polygon.size();

  for (size_t i = 0; i < n; ++i) {
    const glm::vec2 &v1 = polygon[i];
    const glm::vec2 &v2 = polygon[(i + 1) % n];

    if ((v1.y > point.y) != (v2.y > point.y)) {
      float xIntersection =
          (v2.x - v1.x) * (point.y - v1.y) / (v2.y - v1.y) + v1.x;
      if (point.x < xIntersection) {
        ++intersections;
      }
    }
  }

  return (intersections % 2) == 1;
}

bool SelectionManager::doesFigureIntersectPolygon(
    std::shared_ptr<model::IFigure> figure,
    const std::vector<glm::vec2> &polygonWorld) const {
  /// Check if figure intersects with polygon (simplified check)
  if (!figure || polygonWorld.size() < 3) {
    return false;
  }

  /// Get figure vertices based on type
  std::vector<glm::vec2> figurePoints;

  if (auto tri =
          std::dynamic_pointer_cast<model::Figure<model::Triangle>>(figure)) {
    figurePoints = {glm::vec2(tri->first.x, tri->first.y),
                    glm::vec2(tri->second.x, tri->second.y),
                    glm::vec2(tri->third.x, tri->third.y)};
  } else if (auto quad = std::dynamic_pointer_cast<model::Figure<model::Quad>>(
                 figure)) {
    figurePoints = {glm::vec2(quad->first.x, quad->first.y),
                    glm::vec2(quad->second.x, quad->second.y),
                    glm::vec2(quad->third.x, quad->third.y),
                    glm::vec2(quad->fourth.x, quad->fourth.y)};
  } else if (auto circle =
                 std::dynamic_pointer_cast<model::Figure<model::Circle>>(
                     figure)) {
    glm::vec2 center(circle->center.x, circle->center.y);
    figurePoints.push_back(center);
    /// Add points on circumference
    for (int i = 0; i < 8; ++i) {
      float angle = i * 3.14159f / 4.0f;
      figurePoints.emplace_back(center.x + circle->radius * std::cos(angle),
                                center.y + circle->radius * std::sin(angle));
    }
  } else if (auto ngon = std::dynamic_pointer_cast<model::Figure<model::Ngon>>(
                 figure)) {
    glm::vec2 center(ngon->center.x, ngon->center.y);
    figurePoints.push_back(center);
    int numSides = static_cast<int>(ngon->n);
    float angle = std::atan2(ngon->first.y - ngon->center.y,
                             ngon->first.x - ngon->center.x);
    float angleStep = 2.0f * 3.14159f / numSides;
    for (int i = 0; i < numSides; ++i) {
      float a = angle + i * angleStep;
      figurePoints.emplace_back(ngon->center.x + ngon->radius * std::cos(a),
                                ngon->center.y + ngon->radius * std::sin(a));
    }
  } else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier3>>(
                     figure)) {
    figurePoints = {glm::vec2(curve->start.x, curve->start.y),
                    glm::vec2(curve->end.x, curve->end.y),
                    glm::vec2(curve->first.x, curve->first.y)};
  } else if (auto curve =
                 std::dynamic_pointer_cast<model::Figure<model::CurveBezier4>>(
                     figure)) {
    figurePoints = {glm::vec2(curve->start.x, curve->start.y),
                    glm::vec2(curve->end.x, curve->end.y),
                    glm::vec2(curve->first.x, curve->first.y),
                    glm::vec2(curve->second.x, curve->second.y)};
  }

  /// Check if any figure point is inside the polygon
  for (const auto &point : figurePoints) {
    if (isPointInPolygon(point, polygonWorld)) {
      return true;
    }
  }

  return false;
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

} // namespace view
