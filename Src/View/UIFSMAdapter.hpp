#pragma once

#include <Controller/FSM.hpp>
#include <Model/SketchPlane.hpp>
#include <View/CameraController.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <functional>

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
  using ShowPlaneSelectionCallback = std::function<void(std::function<void(int)>)>;

  /**
   * @brief Callback function type for updating status bar text
   * @param text The status text to display
   */
  using UpdateStatusCallback = std::function<void(const std::string&)>;

  /**
   * @brief Construct a UIFSMAdapter
   * @param fsm Reference to the finite state machine
   * @param cameraController Reference to the camera controller
   * @param logger Logger for diagnostic output
   */
  UIFSMAdapter(fsm::Machine& fsm, CameraController& cameraController,
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
   * Called automatically when the FSM state changes to update the UI accordingly.
   * Triggers plane selection UI or status updates based on the new state.
   */
  void onStateChanged(fsm::State newState);

  /**
   * @brief Enter sketch mode from UI
   *
   * Sends the OnEnterSketchMode event to the FSM to transition to PlaneSelection state.
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
  model::SketchPlane* getCurrentSketchPlane();

  /**
   * @brief Check if currently in sketch mode
   * @return true if in PlaneSelection or SketchEdit state, false otherwise
   */
  bool isInSketchMode() const;

private:
  /// Reference to the finite state machine
  fsm::Machine& fsm_;

  /// Reference to the camera controller
  CameraController& cameraController_;

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

  /**
   * @brief Get available sketch planes
   * @return Vector of preset sketch planes
   */
  std::vector<model::SketchPlane> getAvailablePlanes() const;
};

} // namespace view
