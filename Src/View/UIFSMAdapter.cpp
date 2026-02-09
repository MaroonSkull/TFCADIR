#include "UIFSMAdapter.hpp"

namespace view {

UIFSMAdapter::UIFSMAdapter(fsm::Machine &fsm,
                           CameraController &cameraController,
                           std::shared_ptr<spdlog::logger> logger)
    : fsm_(fsm), cameraController_(cameraController),
      logger_(std::move(logger)), currentState_(fsm.get_current_state()),
      primarySelectionIndex_(-1) {
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

void UIFSMAdapter::setSelectionChangedCallback(
    SelectionChangedCallback callback) {
  selectionChangedCallback_ = std::move(callback);
}

void UIFSMAdapter::setPropertyChangedCallback(
    PropertyChangedCallback callback) {
  propertyChangedCallback_ = std::move(callback);
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

// ==========================================================================
// Phase 2: Tool Management Methods
// These methods use local member variables because FSMConfig's VariableValue
// only supports simple types (int, float, string, bool), not complex types
// like std::map or std::vector<glm::vec3>.
// ==========================================================================

std::string UIFSMAdapter::getActiveTool() const {
  /// Return the currently active tool ID from local storage
  return activeTool_;
}

std::map<std::string, std::any> UIFSMAdapter::getToolOptions() const {
  /// Return the tool options from local storage
  return toolOptions_;
}

std::vector<glm::vec3> UIFSMAdapter::getCollectedPoints() const {
  /// Return the collected points from local storage
  return collectedPoints_;
}

void UIFSMAdapter::activateTool(const std::string &toolId) {
  /// Activate the specified tool by:
  /// 1. Updating local state
  /// 2. Triggering FSM event (if event is defined in FSM configuration)

  // Update local state
  activeTool_ = toolId;
  collectedPoints_.clear();

  // Map tool IDs to their corresponding FSM event names
  std::string eventName;
  if (toolId == "Line3D") {
    eventName = "OnActivateLine3D";
  } else if (toolId == "Circle3D") {
    eventName = "OnActivateCircle3D";
  } else if (toolId == "Arc3D") {
    eventName = "OnActivateArc3D";
  } else if (toolId == "Rectangle3D") {
    eventName = "OnActivateRectangle3D";
  } else if (toolId == "Polygon3D") {
    eventName = "OnActivatePolygon3D";
  } else if (toolId == "NGon3D") {
    eventName = "OnActivateNGon3D";
  } else if (toolId == "LineInSketch") {
    eventName = "OnActivateLineInSketch";
  } else if (toolId == "CircleInSketch") {
    eventName = "OnActivateCircleInSketch";
  } else {
    logger_->warn("Unknown tool ID requested: {}", toolId);
    return;
  }

  /// Trigger the FSM event via FSMConfig's string-based event system
  /// Note: These events need to be defined in the FSM YAML configuration
  if (fsmconfig::StateMachine *fsm = fsm_.get_fsm()) {
    try {
      fsm->triggerEvent(eventName);
      logger_->info("Activated tool: {} (event: {})", toolId, eventName);
    } catch (const fsmconfig::StateException &e) {
      logger_->warn("Failed to trigger event {}: {}", eventName, e.what());
      // Continue anyway - tool is still activated locally
    }
  }
}

void UIFSMAdapter::deactivateTool() {
  /// Deactivate the current tool by:
  /// 1. Clearing local state
  /// 2. Triggering FSM event (if event is defined in FSM configuration)

  activeTool_.clear();
  toolOptions_.clear();
  collectedPoints_.clear();

  /// Trigger the OnDeactivateTool FSM event
  /// Note: This event needs to be defined in the FSM YAML configuration
  if (fsmconfig::StateMachine *fsm = fsm_.get_fsm()) {
    try {
      fsm->triggerEvent("OnDeactivateTool");
      logger_->info("Deactivated tool");
    } catch (const fsmconfig::StateException &e) {
      logger_->warn("Failed to trigger event OnDeactivateTool: {}", e.what());
      // Continue anyway - tool is still deactivated locally
    }
  }
}

// ==========================================================================
// Phase 3: Object Management Methods
// These methods manage selection state for figures
// ==========================================================================

std::vector<uint32_t> UIFSMAdapter::getSelectedFigureIds() const {
  /// Return the selected figure IDs from local storage
  return selectedFigureIds_;
}

int UIFSMAdapter::getPrimarySelectionIndex() const {
  /// Return the primary selection index
  return primarySelectionIndex_;
}

uint32_t UIFSMAdapter::getPrimarySelectionId() const {
  /// Return the primary selection ID or 0 if no selection
  if (primarySelectionIndex_ >= 0 &&
      primarySelectionIndex_ < static_cast<int>(selectedFigureIds_.size())) {
    return selectedFigureIds_[primarySelectionIndex_];
  }
  return 0;
}

void UIFSMAdapter::selectFigure(uint32_t figureId) {
  /// Select a single figure (replaces current selection)
  selectedFigureIds_.clear();
  selectedFigureIds_.push_back(figureId);
  primarySelectionIndex_ = 0;

  logger_->info("Selected figure: {}", figureId);

  /// Notify listeners of selection change
  if (selectionChangedCallback_) {
    selectionChangedCallback_(selectedFigureIds_);
  }
}

void UIFSMAdapter::toggleFigureSelection(uint32_t figureId) {
  /// Toggle selection state of a figure
  auto it = std::find(selectedFigureIds_.begin(), selectedFigureIds_.end(),
                      figureId);

  if (it != selectedFigureIds_.end()) {
    /// Figure is selected - remove it
    size_t index = std::distance(selectedFigureIds_.begin(), it);
    selectedFigureIds_.erase(it);

    /// Adjust primary selection index if needed
    if (primarySelectionIndex_ >= static_cast<int>(selectedFigureIds_.size())) {
      primarySelectionIndex_ = static_cast<int>(selectedFigureIds_.size()) - 1;
    }

    logger_->info("Deselected figure: {}", figureId);
  } else {
    /// Figure is not selected - add it
    selectedFigureIds_.push_back(figureId);
    logger_->info("Added to selection: {}", figureId);
  }

  /// Notify listeners of selection change
  if (selectionChangedCallback_) {
    selectionChangedCallback_(selectedFigureIds_);
  }
}

void UIFSMAdapter::addToSelection(uint32_t figureId) {
  /// Add a figure to the current selection if not already selected
  auto it = std::find(selectedFigureIds_.begin(), selectedFigureIds_.end(),
                      figureId);

  if (it == selectedFigureIds_.end()) {
    selectedFigureIds_.push_back(figureId);
    logger_->info("Added to selection: {}", figureId);

    /// Notify listeners of selection change
    if (selectionChangedCallback_) {
      selectionChangedCallback_(selectedFigureIds_);
    }
  }
}

void UIFSMAdapter::removeFromSelection(uint32_t figureId) {
  /// Remove a figure from the current selection
  auto it = std::find(selectedFigureIds_.begin(), selectedFigureIds_.end(),
                      figureId);

  if (it != selectedFigureIds_.end()) {
    size_t index = std::distance(selectedFigureIds_.begin(), it);
    selectedFigureIds_.erase(it);

    /// Adjust primary selection index if needed
    if (primarySelectionIndex_ >= static_cast<int>(selectedFigureIds_.size())) {
      primarySelectionIndex_ = static_cast<int>(selectedFigureIds_.size()) - 1;
    }

    logger_->info("Removed from selection: {}", figureId);

    /// Notify listeners of selection change
    if (selectionChangedCallback_) {
      selectionChangedCallback_(selectedFigureIds_);
    }
  }
}

void UIFSMAdapter::clearSelection() {
  /// Clear all selections
  if (!selectedFigureIds_.empty()) {
    selectedFigureIds_.clear();
    primarySelectionIndex_ = -1;

    logger_->info("Cleared selection");

    /// Notify listeners of selection change
    if (selectionChangedCallback_) {
      selectionChangedCallback_(selectedFigureIds_);
    }
  }
}

void UIFSMAdapter::setPrimarySelection(int index) {
  /// Set the primary selection by index
  if (index >= 0 && index < static_cast<int>(selectedFigureIds_.size())) {
    primarySelectionIndex_ = index;
    logger_->info("Primary selection set to index: {}", index);
  } else {
    logger_->warn("Invalid primary selection index: {}", index);
  }
}

void UIFSMAdapter::updateFigureProperty(uint32_t figureId,
                                       const std::string &propertyPath,
                                       const std::any &value) {
  /// Update a property of a specific figure
  /// Note: This method requires access to the model which is passed in through
  /// the constructor or set via a separate setter. For now, this is a
  /// placeholder that logs the update. The actual property update will be
  /// implemented by the PropertyInspectorPanel which has direct access to the
  /// model.

  logger_->info("Updating property '{}' for figure {}", propertyPath,
                figureId);

  /// Parse property path (e.g., "center.x", "radius")
  size_t dotPos = propertyPath.find('.');

  /// Notify listeners of property change
  if (propertyChangedCallback_) {
    propertyChangedCallback_(figureId, propertyPath);
  }
}

} // namespace view
