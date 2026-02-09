#pragma once

#include <Model/IModel.hpp>
#include <View/UIFSMAdapter.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace view {

/**
 * @brief Stateless coordinator for figure selection management
 * @details SelectionManager provides a convenient API for selection operations
 *          without storing any local state. All selection data is queried from
 *          and managed by UIFSMAdapter, which serves as the single source of
 *          truth for selection state. This implements the Stateless Coordinator
 *          pattern from Phase 3 architecture.
 */
class SelectionManager {
public:
  /**
   * @brief Construct a SelectionManager
   * @param fsmAdapter Reference to UIFSMAdapter for selection state queries
   * @param model Reference to the figure model for figure lookups
   */
  SelectionManager(UIFSMAdapter &fsmAdapter, model::FlatFigures &model);

  /**
   * @brief Destructor
   */
  ~SelectionManager() = default;

  // ==========================================================================
  // Query Methods (stateless - read from UIFSMAdapter)
  // ==========================================================================

  /**
   * @brief Get the list of selected figure IDs
   * @return Vector of selected figure IDs
   * @note Queries UIFSMAdapter for the current selection state
   */
  std::vector<uint32_t> getSelectedFigureIds() const;

  /**
   * @brief Get the count of selected figures
   * @return Number of selected figures
   */
  size_t getSelectionCount() const;

  /**
   * @brief Check if a specific figure is selected
   * @param figureId The figure ID to check
   * @return true if the figure is selected, false otherwise
   */
  bool isFigureSelected(uint32_t figureId) const;

  /**
   * @brief Get the primary selection figure ID
   * @return Primary selection ID or 0 if no selection
   * @note The primary selection is the first figure selected
   */
  uint32_t getPrimarySelectionId() const;

  /**
   * @brief Get the primary selection index
   * @return Index of primary selection in the selected figures list, or -1 if no
   * selection
   */
  int getPrimarySelectionIndex() const;

  /**
   * @brief Get the list of selected figure objects
   * @return Vector of shared pointers to selected figures
   * @note This method queries the model to retrieve figure objects for the
   *       selected IDs
   */
  std::vector<std::shared_ptr<model::IFigure>> getSelectedFigures() const;

  // ==========================================================================
  // Action Methods (delegate to UIFSMAdapter)
  // ==========================================================================

  /**
   * @brief Select a single figure (replaces current selection)
   * @param figureId The figure to select
   * @note Clears any existing selection and selects only the specified figure
   */
  void selectFigure(uint32_t figureId);

  /**
   * @brief Toggle selection state of a figure
   * @param figureId The figure to toggle
   * @note Adds the figure to selection if not selected, removes it if selected
   */
  void toggleFigureSelection(uint32_t figureId);

  /**
   * @brief Add a figure to the current selection
   * @param figureId The figure to add
   * @note Keeps existing selection and adds the specified figure
   */
  void addToSelection(uint32_t figureId);

  /**
   * @brief Remove a figure from the current selection
   * @param figureId The figure to remove
   */
  void removeFromSelection(uint32_t figureId);

  /**
   * @brief Clear all selections
   */
  void clearSelection();

  /**
   * @brief Select all figures in the model
   */
  void selectAll();

  /**
   * @brief Set the primary selection by index
   * @param index The index in the selected figures list to set as primary
   * @note This allows changing which selected figure is primary without changing
   *       the selection
   */
  void setPrimarySelection(int index);

  // ==========================================================================
  // Validation (DEBUG builds only)
  // ==========================================================================

#ifndef NDEBUG
  /**
   * @brief Validate selection state consistency
   * @return true if selection state is valid, false otherwise
   * @note In DEBUG builds, checks that selected figure IDs exist in the model
   *       and that primary selection is consistent
   */
  bool validateSelectionState() const;
#endif

private:
  /// Reference to UIFSMAdapter (no ownership)
  UIFSMAdapter &fsmAdapter_;

  /// Reference to the figure model (no ownership)
  model::FlatFigures &model_;
};

} // namespace view
