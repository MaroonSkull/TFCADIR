#pragma once

#include <Controller/FSM.hpp>
#include <Model/SketchPlane.hpp>
#include <View/CameraController.hpp>
#include <any>
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Adapter class that bridges FSM state changes with UI updates
 *
 * UIFSMAdapter connects the finite state machine to the user interface,
 * triggering UI updates when the FSM state changes. It handles plane
 * selection UI display and mode status updates.
 */
class UIFSMAdapter {
public:
  /**
   * @brief Callback function type for showing plane selection UI
   * @param onPlaneSelected Function to call when user selects a plane
   */
  using ShowPlaneSelectionCallback =
      std::function<void(std::function<void(int)>)>;

  /**
   * @brief Callback function type for updating status bar text
   * @param text The status text to display
   */
  using UpdateStatusCallback = std::function<void(const std::string &)>;

  /**
   * @brief Construct a UIFSMAdapter
   * @param fsm Reference to the finite state machine
   * @param cameraController Reference to the camera controller
   * @param logger Logger for diagnostic output
   */
  UIFSMAdapter(fsm::Machine &fsm, CameraController &cameraController,
               std::shared_ptr<spdlog::logger> logger);

  /**
   * @brief Destructor
   */
  ~UIFSMAdapter();

  /**
   * @brief Set callback for showing plane selection UI
   * @param callback Function to invoke when plane selection UI should be shown
   */
  void setShowPlaneSelectionCallback(ShowPlaneSelectionCallback callback);

  /**
   * @brief Set callback for updating status bar
   * @param callback Function to invoke when status text should be updated
   */
  void setUpdateStatusCallback(UpdateStatusCallback callback);

  /**
   * @brief Handle FSM state change
   * @param newState The new FSM state
   *
   * Called automatically when the FSM state changes to update the UI
   * accordingly. Triggers plane selection UI or status updates based on the new
   * state.
   */
  void onStateChanged(fsm::State newState);

  /**
   * @brief Enter sketch mode from UI
   *
   * Sends the OnEnterSketchMode event to the FSM to transition to
   * PlaneSelection state.
   */
  void enterSketchMode();

  /**
   * @brief Select a sketch plane
   * @param planeIndex Index of the selected plane (0=Right, 1=Top, 2=Front)
   *
   * Sends the OnPlaneSelected event to the FSM with the selected plane index.
   */
  void selectPlane(int planeIndex);

  /**
   * @brief Exit sketch mode from UI
   *
   * Sends the OnExitSketchMode event to the FSM to return to Idle state.
   */
  void exitSketchMode();

  /**
   * @brief Get the currently selected sketch plane
   * @return Reference to the current sketch plane, or nullptr if none selected
   */
  model::SketchPlane *getCurrentSketchPlane();

  /**
   * @brief Check if currently in sketch mode
   * @return true if in PlaneSelection or SketchEdit state, false otherwise
   */
  bool isInSketchMode() const;

  // ==========================================================================
  // Phase 2: Tool Management Methods
  // These methods provide access to tool state stored in the FSM
  // ==========================================================================

  /**
   * @brief Get the currently active tool ID
   * @return Current tool ID or empty string if no tool active
   *
   * Queries the FSM state data for the activeTool field.
   * Returns the tool identifier (e.g., "Line3D", "Circle3D") or empty string
   * if no tool is currently active.
   */
  std::string getActiveTool() const;

  /**
   * @brief Get options for the active tool
   * @return Map of option names to values
   *
   * Queries the FSM state data for the toolOptions field.
   * Returns a map of option names to their current values for the
   * currently active tool. Options are tool-specific configuration
   * parameters (e.g., radius for Circle3D, sides for Polygon3D).
   */
  std::map<std::string, std::any> getToolOptions() const;

  /**
   * @brief Get collected points for current drawing operation
   * @return Vector of collected point coordinates
   *
   * Queries the FSM state data for the collectedPoints field.
   * Returns the list of 3D points collected so far in the current
   * drawing operation. The number of points depends on the tool:
   * - Line3D: 2 points (start, end)
   * - Circle3D: 2 points (center, edge)
   * - Arc3D: 3 points (start, control, end)
   * - Rectangle3D: 2 points (first corner, opposite corner)
   * - Polygon3D: N points (vertices in order)
   */
  std::vector<glm::vec3> getCollectedPoints() const;

  /**
   * @brief Activate a tool
   * @param toolId Tool identifier to activate
   *
   * Sends the appropriate tool activation event to the FSM:
   * - "Line3D" → OnActivateLine3D event
   * - "Circle3D" → OnActivateCircle3D event
   * - "Arc3D" → OnActivateArc3D event
   * - "Rectangle3D" → OnActivateRectangle3D event
   * - "Polygon3D" → OnActivatePolygon3D event
   * - "NGon3D" → OnActivateNGon3D event
   * - "LineInSketch" → OnActivateLineInSketch event
   * - "CircleInSketch" → OnActivateCircleInSketch event
   *
   * The tool remains active until deactivated or a drawing operation completes.
   */
  void activateTool(const std::string &toolId);

  /**
   * @brief Deactivate the current tool
   *
   * Sends the OnDeactivateTool event to the FSM, clearing the activeTool
   * field in state data. This cancels any pending drawing operation.
   */
  void deactivateTool();

private:
  /// Reference to the finite state machine
  fsm::Machine &fsm_;

  /// Reference to the camera controller
  CameraController &cameraController_;

  /// Logger for diagnostic output
  std::shared_ptr<spdlog::logger> logger_;

  /// Callback for showing plane selection UI
  ShowPlaneSelectionCallback showPlaneSelectionCallback_;

  /// Callback for updating status bar
  UpdateStatusCallback updateStatusCallback_;

  /// Current sketch plane
  std::unique_ptr<model::SketchPlane> currentSketchPlane_;

  /// Current FSM state
  fsm::State currentState_;

  // ==========================================================================
  // Phase 2: Tool State Storage
  // These member variables store tool state that cannot be stored in FSM
  // because FSMConfig's VariableValue only supports simple types (int, float,
  // string, bool), not complex types like std::map or std::vector<glm::vec3>.
  // ==========================================================================

  /// Currently active tool ID (e.g., "Line3D", "Circle3D")
  std::string activeTool_;

  /// Options for the active tool (e.g., radius, sides, creation method)
  std::map<std::string, std::any> toolOptions_;

  /// Points collected during the current drawing operation
  std::vector<glm::vec3> collectedPoints_;

  /**
   * @brief Get available sketch planes
   * @return Vector of preset sketch planes
   */
  std::vector<model::SketchPlane> getAvailablePlanes() const;
};

} // namespace view
