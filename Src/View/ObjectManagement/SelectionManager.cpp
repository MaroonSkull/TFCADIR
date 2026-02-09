#include "SelectionManager.hpp"
#include <Model/FlatFigure.hpp>
#include <algorithm>

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

} // namespace view
