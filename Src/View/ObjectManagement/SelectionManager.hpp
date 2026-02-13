#pragma once

#include <Model/IModel.hpp>
#include <View/ObjectManagement/SelectionTypes.hpp>
#include <View/UIFSMAdapter.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace view {

class CanvasHitTester;

/**
 * @brief Stateless coordinator for figure selection management
 * @details SelectionManager provides a convenient API for selection operations
 *          without storing any local state. All selection data is queried from
 *          and managed by UIFSMAdapter, which serves as the single source of
 *          truth for selection state. This implements the Stateless Coordinator
 *          pattern from Phase 3 architecture.
 *
 * Phase 10 extends this with advanced selection features:
 * - Box selection (rectangle drag)
 * - Lasso selection (freeform)
 * - Polygon selection (click to add vertices)
 * - Selection filters (by type, layer, properties)
 * - Selection modifiers (Shift, Ctrl, Alt)
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
   * @return Index of primary selection in the selected figures list, or -1 if
   * no selection
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
   * @note This allows changing which selected figure is primary without
   * changing the selection
   */
  void setPrimarySelection(int index);

  // ==========================================================================
  // Phase 10: Advanced Selection Methods
  // ==========================================================================

  /**
   * @brief Select figures within a rectangular box
   * @param hitTester Reference to CanvasHitTester for hit testing
   * @param geometry Selection geometry containing box coordinates
   * @param modifier Selection modifier (None, Add, Toggle, Remove)
   * @param filter Selection filter criteria
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @return Number of figures selected
   * @details Box selection supports two modes:
   *          - Left-to-right: selects figures that intersect the box
   *          - Right-to-left: selects figures fully contained in the box
   */
  size_t selectByBox(CanvasHitTester &hitTester,
                     const SelectionGeometry &geometry,
                     SelectionModifier modifier, const SelectionFilter &filter,
                     float viewportWidth, float viewportHeight,
                     const glm::mat4 &modelView, const glm::mat4 &projection);

  /**
   * @brief Select figures within a lasso region
   * @param hitTester Reference to CanvasHitTester for hit testing
   * @param geometry Selection geometry containing lasso points
   * @param modifier Selection modifier (None, Add, Toggle, Remove)
   * @param filter Selection filter criteria
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @return Number of figures selected
   * @details Lasso selection uses point-in-polygon testing for each figure
   */
  size_t selectByLasso(CanvasHitTester &hitTester,
                       const SelectionGeometry &geometry,
                       SelectionModifier modifier,
                       const SelectionFilter &filter, float viewportWidth,
                       float viewportHeight, const glm::mat4 &modelView,
                       const glm::mat4 &projection);

  /**
   * @brief Select figures within a polygon region
   * @param hitTester Reference to CanvasHitTester for hit testing
   * @param geometry Selection geometry containing polygon vertices
   * @param modifier Selection modifier (None, Add, Toggle, Remove)
   * @param filter Selection filter criteria
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @return Number of figures selected
   * @details Polygon selection is similar to lasso but with discrete vertices
   */
  size_t selectByPolygon(CanvasHitTester &hitTester,
                         const SelectionGeometry &geometry,
                         SelectionModifier modifier,
                         const SelectionFilter &filter, float viewportWidth,
                         float viewportHeight, const glm::mat4 &modelView,
                         const glm::mat4 &projection);

  /**
   * @brief Apply selection modifier to a set of figure IDs
   * @param figureIds Figure IDs to apply the modifier to
   * @param modifier Selection modifier (None, Add, Toggle, Remove)
   * @details Handles the interaction between new selection and existing:
   *          - None: Replace current selection with new IDs
   *          - Add: Add new IDs to current selection
   *          - Toggle: Toggle selection state of each ID
   *          - Remove: Remove new IDs from current selection
   */
  void applySelectionModifier(const std::vector<uint32_t> &figureIds,
                              SelectionModifier modifier);

  /**
   * @brief Filter a set of figure IDs by selection criteria
   * @param figureIds Figure IDs to filter
   * @param filter Selection filter criteria
   * @return Filtered vector of figure IDs
   */
  std::vector<uint32_t> filterFigures(const std::vector<uint32_t> &figureIds,
                                      const SelectionFilter &filter) const;

  /**
   * @brief Check if a figure passes the selection filter
   * @param figureId Figure ID to check
   * @param filter Selection filter criteria
   * @return true if the figure passes the filter
   */
  bool passesFilter(uint32_t figureId, const SelectionFilter &filter) const;

  /**
   * @brief Invert the current selection
   * @param filter Optional filter to apply to inverted selection
   * @details Selects all unselected figures and deselects all selected figures
   */
  void invertSelection(const SelectionFilter &filter = SelectionFilter{});

  /**
   * @brief Select figures similar to the currently selected figures
   * @param filter Criteria for similarity (type, layer, etc.)
   * @return Number of figures added to selection
   */
  size_t selectSimilar(const SelectionFilter &filter);

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

  /**
   * @brief Convert screen coordinates to world coordinates
   * @param screenX Screen X coordinate in pixels
   * @param screenY Screen Y coordinate in pixels
   * @param viewportWidth Viewport width in pixels
   * @param viewportHeight Viewport height in pixels
   * @param modelView Model-view matrix
   * @param projection Projection matrix
   * @return World position as vec2
   */
  glm::vec2 screenToWorld(float screenX, float screenY, float viewportWidth,
                          float viewportHeight, const glm::mat4 &modelView,
                          const glm::mat4 &projection) const;

  /**
   * @brief Test if a point is inside a polygon using ray casting
   * @param point Point to test in world coordinates
   * @param polygon Polygon vertices in world coordinates
   * @return true if point is inside the polygon
   */
  static bool isPointInPolygon(const glm::vec2 &point,
                               const std::vector<glm::vec2> &polygon);

  /**
   * @brief Test if a figure's bounding box intersects a polygon
   * @param figure Figure to test
   * @param polygonWorld Polygon vertices in world coordinates
   * @return true if figure intersects the polygon
   */
  bool
  doesFigureIntersectPolygon(std::shared_ptr<model::IFigure> figure,
                             const std::vector<glm::vec2> &polygonWorld) const;

  /**
   * @brief Get figure type filter from a figure
   * @param figure Figure to get type from
   * @return FigureTypeFilter corresponding to the figure type
   */
  static FigureTypeFilter
  getFigureTypeFilter(std::shared_ptr<model::IFigure> figure);
};

} // namespace view
