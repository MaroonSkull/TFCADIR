#include "UIFSMAdapter.hpp"

namespace view {

UIFSMAdapter::UIFSMAdapter(fsm::Machine &fsm,
                           CameraController &cameraController,
                           std::shared_ptr<spdlog::logger> logger)
    : fsm_(fsm), cameraController_(cameraController),
      logger_(std::move(logger)), currentState_(fsm.get_current_state()) {
  logger_->info("UIFSMAdapter initialized");
}

UIFSMAdapter::~UIFSMAdapter() { logger_->info("UIFSMAdapter destroyed"); }

void UIFSMAdapter::setShowPlaneSelectionCallback(
    ShowPlaneSelectionCallback callback) {
  showPlaneSelectionCallback_ = std::move(callback);
}

void UIFSMAdapter::setUpdateStatusCallback(UpdateStatusCallback callback) {
  updateStatusCallback_ = std::move(callback);
}

void UIFSMAdapter::onStateChanged(fsm::State newState) {
  currentState_ = newState;
  logger_->debug("UIFSMAdapter: State changed to {}",
                 static_cast<int>(newState));

  switch (newState) {
  case fsm::State::PlaneSelection:
    /// Show plane selection UI
    if (showPlaneSelectionCallback_) {
      showPlaneSelectionCallback_(
          [this](int planeIndex) { this->selectPlane(planeIndex); });
    }
    /// Update status bar
    if (updateStatusCallback_) {
      updateStatusCallback_("Select a sketch plane (Right, Top, or Front)");
    }
    break;

  case fsm::State::SketchEdit:
    /// Update status bar
    if (updateStatusCallback_) {
      updateStatusCallback_("Sketch Mode: 2D drawing on sketch plane");
    }
    break;

  case fsm::State::Idle:
    /// Update status bar
    if (updateStatusCallback_) {
      updateStatusCallback_("Idle");
    }
    break;

  default:
    /// Other states - update with state name
    if (updateStatusCallback_) {
      updateStatusCallback_("Drawing");
    }
    break;
  }
}

void UIFSMAdapter::enterSketchMode() {
  logger_->info("Entering sketch mode");
  fsm_.process_event(fsm::events::OnEnterSketchMode());
}

void UIFSMAdapter::selectPlane(int planeIndex) {
  logger_->info("Selecting sketch plane: {}", planeIndex);

  /// Create the selected sketch plane
  auto planes = getAvailablePlanes();
  if (planeIndex >= 0 && planeIndex < static_cast<int>(planes.size())) {
    currentSketchPlane_ =
        std::make_unique<model::SketchPlane>(planes[planeIndex]);

    /// Send event to FSM with plane index parameter
    fsm::events::OnPlaneSelected event(planeIndex);
    fsm_.process_event(event);

    /// Position camera for the selected plane
    auto cameraState =
        cameraController_.setCameraForPlane(*currentSketchPlane_);
    /// Apply camera state to the actual camera (will be done in GUI
    /// integration)
    logger_->info("Camera positioned for sketch plane: {}",
                  currentSketchPlane_->getName());
  } else {
    logger_->error("Invalid plane index: {}", planeIndex);
  }
}

void UIFSMAdapter::exitSketchMode() {
  logger_->info("Exiting sketch mode");
  fsm_.process_event(fsm::events::OnExitSketchMode());

  /// Clear current sketch plane
  currentSketchPlane_.reset();

  /// Restore camera to previous state
  cameraController_.restoreCameraState();
}

model::SketchPlane *UIFSMAdapter::getCurrentSketchPlane() {
  return currentSketchPlane_.get();
}

bool UIFSMAdapter::isInSketchMode() const {
  return currentState_ == fsm::State::PlaneSelection ||
         currentState_ == fsm::State::SketchEdit;
}

std::vector<model::SketchPlane> UIFSMAdapter::getAvailablePlanes() const {
  /// Create preset sketch planes
  std::vector<model::SketchPlane> planes;
  planes.push_back(
      model::SketchPlane(model::SketchPlane::PresetPlane::XY)); // Front plane
  planes.push_back(
      model::SketchPlane(model::SketchPlane::PresetPlane::XZ)); // Top plane
  planes.push_back(
      model::SketchPlane(model::SketchPlane::PresetPlane::YZ)); // Right plane
  return planes;
}

} // namespace view
