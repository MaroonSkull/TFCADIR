#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace view {

// Forward declaration to avoid including UIFSMAdapter.hpp in header
class UIFSMAdapter;

/**
 * @brief Manages tool activation and state for Phase 2 drawing tools
 *
 * ToolManager provides a stateless interface for activating, deactivating,
 * and querying the current tool state. All tool state is stored in the FSM
 * as the single source of truth, ensuring consistency across the application.
 *
 * This class queries UIFSMAdapter which in turn queries the FSM for all
 * state information, maintaining the Phase 1 architectural pattern.
 */
class ToolManager {
public:
  /**
   * @brief Construct a ToolManager
   * @param adapter Reference to the UIFSMAdapter for FSM queries
   */
  explicit ToolManager(UIFSMAdapter &adapter);

  /**
   * @brief Destructor
   */
  ~ToolManager() = default;

  // Copy prohibition
  ToolManager(const ToolManager &) = delete;
  ToolManager &operator=(const ToolManager &) = delete;

  // Move permission
  ToolManager(ToolManager &&) noexcept = default;
  ToolManager &operator=(ToolManager &&) noexcept = default;

  /**
   * @brief Activate a tool by triggering the appropriate FSM event
   * @param toolId The tool identifier to activate (e.g., "Line3D", "Circle3D")
   *
   * Maps tool IDs to their corresponding FSM activation events:
   * - "Line3D" → OnActivateLine3D
   * - "Circle3D" → OnActivateCircle3D
   * - "Arc3D" → OnActivateArc3D
   * - "Rectangle3D" → OnActivateRectangle3D
   * - "Polygon3D" → OnActivatePolygon3D
   * - "NGon3D" → OnActivateNGon3D
   * - "LineInSketch" → OnActivateLineInSketch
   * - "CircleInSketch" → OnActivateCircleInSketch
   *
   * @throws std::runtime_error if toolId is not recognized
   */
  void activateTool(const std::string &toolId);

  /**
   * @brief Deactivate the currently active tool
   *
   * Triggers the OnDeactivateTool FSM event to clear the active tool state.
   */
  void deactivateTool();

  /**
   * @brief Get the currently active tool ID
   * @return Current tool ID or empty string if no tool is active
   *
   * Queries the FSM state data for the "activeTool" variable.
   */
  std::string getActiveTool() const;

  /**
   * @brief Get points collected during the current drawing operation
   * @return Vector of collected point coordinates
   *
   * Queries the FSM state data for the "collectedPoints" variable.
   * Returns an empty vector if no points have been collected or no tool is
   * active.
   */
  std::vector<glm::vec3> getCollectedPoints() const;

private:
  /// Reference to the UIFSMAdapter for FSM queries (non-owning)
  UIFSMAdapter &adapter_;

  /**
   * @brief Map tool ID to corresponding FSM activation event name
   * @param toolId The tool identifier
   * @return The FSM event name for activating this tool
   * @throws std::runtime_error if toolId is not recognized
   */
  std::string toolIdToEvent(const std::string &toolId) const;
};

} // namespace view
