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
  auto it =
      std::find(selectedFigureIds_.begin(), selectedFigureIds_.end(), figureId);

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
  auto it =
      std::find(selectedFigureIds_.begin(), selectedFigureIds_.end(), figureId);

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
  auto it =
      std::find(selectedFigureIds_.begin(), selectedFigureIds_.end(), figureId);

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

  logger_->info("Updating property '{}' for figure {}", propertyPath, figureId);

  /// Parse property path (e.g., "center.x", "radius")
  size_t dotPos = propertyPath.find('.');

  /// Notify listeners of property change
  if (propertyChangedCallback_) {
    propertyChangedCallback_(figureId, propertyPath);
  }
}

// ==========================================================================
// Figure Grouping Methods (STUB - Not fully implemented)
// These methods are stubs to allow compilation of GroupFiguresCommand
// and UngroupFiguresCommand. Full implementation is pending.
// ==========================================================================

uint32_t UIFSMAdapter::groupFigures(const std::vector<uint32_t> &figureIds) {
  /// Stub implementation - returns 0 to indicate failure
  /// Full implementation requires access to the model's grouping functionality
  logger_->warn("groupFigures called with {} figures - STUB (not implemented)",
                figureIds.size());
  return 0; /// Return 0 to indicate grouping failed
}

std::vector<uint32_t> UIFSMAdapter::ungroupFigures(uint32_t groupId) {
  /// Stub implementation - returns empty vector to indicate failure
  /// Full implementation requires access to the model's ungrouping
  /// functionality
  logger_->warn("ungroupFigures called for group {} - STUB (not implemented)",
                groupId);
  return std::vector<uint32_t>(); /// Return empty vector to indicate ungrouping
                                  /// failed
}

// ==========================================================================
// Phase 4: Command History Management Methods
// UIFSMAdapter is the single source of truth for command history storage
// ==========================================================================

void UIFSMAdapter::executeCommand(std::unique_ptr<ICommand> command) {
  /// Execute the command
  if (command->execute()) {
    /// Remove any commands after the current index (clear redo chain)
    if (currentCommandIndex_ < commandHistory_.size()) {
      commandHistory_.resize(currentCommandIndex_);
    }

    /// Add the command to history
    commandHistory_.push_back(std::move(command));
    currentCommandIndex_ = commandHistory_.size();

    /// Enforce maximum history size
    if (commandHistory_.size() > MAX_HISTORY_SIZE) {
      commandHistory_.erase(commandHistory_.begin());
      currentCommandIndex_ = commandHistory_.size();
    }

    logger_->info("Executed command, history size: {}, index: {}",
                  commandHistory_.size(), currentCommandIndex_);
  } else {
    logger_->warn("Command execution failed");
  }
}

bool UIFSMAdapter::undoCommand() {
  if (!canUndo()) {
    logger_->warn("Cannot undo: no command to undo");
    return false;
  }

  /// Decrement index and undo the command
  currentCommandIndex_--;
  if (commandHistory_[currentCommandIndex_]->undo()) {
    logger_->info("Undone command, new index: {}", currentCommandIndex_);
    return true;
  } else {
    logger_->error("Undo failed for command at index: {}",
                   currentCommandIndex_);
    currentCommandIndex_++; /// Restore index on failure
    return false;
  }
}

bool UIFSMAdapter::redoCommand() {
  if (!canRedo()) {
    logger_->warn("Cannot redo: no command to redo");
    return false;
  }

  /// Redo the command at current index (execute again) and increment
  if (commandHistory_[currentCommandIndex_]->execute()) {
    currentCommandIndex_++;
    logger_->info("Redone command, new index: {}", currentCommandIndex_);
    return true;
  } else {
    logger_->error("Redo failed for command at index: {}",
                   currentCommandIndex_);
    return false;
  }
}

void UIFSMAdapter::clearCommandHistory() {
  commandHistory_.clear();
  currentCommandIndex_ = 0;
  logger_->info("Command history cleared");
}

bool UIFSMAdapter::canUndo() const { return currentCommandIndex_ > 0; }

bool UIFSMAdapter::canRedo() const {
  return currentCommandIndex_ < commandHistory_.size();
}

std::string UIFSMAdapter::getUndoDescription() const {
  if (canUndo()) {
    return commandHistory_[currentCommandIndex_ - 1]->getDescription();
  }
  return "";
}

std::string UIFSMAdapter::getRedoDescription() const {
  if (canRedo()) {
    return commandHistory_[currentCommandIndex_]->getDescription();
  }
  return "";
}

size_t UIFSMAdapter::getHistorySize() const { return commandHistory_.size(); }

size_t UIFSMAdapter::getCurrentCommandIndex() const {
  return currentCommandIndex_;
}

const ICommand *UIFSMAdapter::getCommandAt(size_t index) const {
  if (index < commandHistory_.size()) {
    return commandHistory_[index].get();
  }
  return nullptr;
}

// ==========================================================================
// Phase 5: Navigation State Management Methods
// UIFSMAdapter is the single source of truth for navigation domain state
// ==========================================================================

void UIFSMAdapter::setOrbitCenter(OrbitCenter center) {
  if (orbitCenter_ != center) {
    orbitCenter_ = center;
    logger_->info("Orbit center changed to: {}", static_cast<int>(center));

    /// Notify listeners of orbit center change
    if (onOrbitCenterChanged_) {
      onOrbitCenterChanged_();
    }
  }
}

OrbitCenter UIFSMAdapter::getOrbitCenter() const { return orbitCenter_; }

void UIFSMAdapter::setCustomOrbitCenter(const glm::vec3 &center) {
  customOrbitCenter_ = center;
  logger_->info("Custom orbit center set to: ({}, {}, {})", center.x, center.y,
                center.z);
}

glm::vec3 UIFSMAdapter::getCustomOrbitCenter() const {
  return customOrbitCenter_;
}

void UIFSMAdapter::setCurrentViewPreset(ViewPreset preset) {
  if (currentViewPreset_ != preset) {
    currentViewPreset_ = preset;
    logger_->info("View preset changed to: {}", static_cast<int>(preset));

    /// Notify listeners of view preset change
    if (onViewPresetChanged_) {
      onViewPresetChanged_();
    }
  }
}

ViewPreset UIFSMAdapter::getCurrentViewPreset() const {
  return currentViewPreset_;
}

void UIFSMAdapter::setIsTransitioning(bool transitioning) {
  isTransitioning_ = transitioning;
}

bool UIFSMAdapter::isTransitioning() const { return isTransitioning_; }

void UIFSMAdapter::setViewPresetChangedCallback(NavigationCallback callback) {
  onViewPresetChanged_ = std::move(callback);
}

void UIFSMAdapter::setOrbitCenterChangedCallback(NavigationCallback callback) {
  onOrbitCenterChanged_ = std::move(callback);
}

// ==========================================================================
// Phase 6: Precision & Snapping Methods
// These methods provide access to grid and snap settings
// ==========================================================================

GridSettings UIFSMAdapter::getGridSettings() const {
  /// Return the current grid settings from local storage
  return gridSettings_;
}

void UIFSMAdapter::setGridSettings(const GridSettings &settings) {
  /// Update grid settings and notify listeners
  if (gridSettings_ != settings) {
    gridSettings_ = settings;
    logger_->info("Grid settings updated");

    /// Notify listeners that grid geometry needs regeneration
    if (onGridGeometryDirty_) {
      onGridGeometryDirty_();
    }

    /// Trigger FSM event to notify components
    fsm_.process_event(fsm::events::OnGridSettingsChanged{});

    /// Notify listeners of grid settings change
    if (onGridSettingsChanged_) {
      onGridSettingsChanged_();
    }
  }
}

SnapSettings UIFSMAdapter::getSnapSettings() const {
  /// Return the current snap settings from local storage
  return snapSettings_;
}

void UIFSMAdapter::setSnapSettings(const SnapSettings &settings) {
  /// Update snap settings and notify listeners
  if (snapSettings_ != settings) {
    snapSettings_ = settings;
    logger_->info("Snap settings updated");

    /// Trigger FSM event to notify components
    fsm_.process_event(fsm::events::OnSnapSettingsChanged{});

    /// Notify listeners of snap settings change
    if (onSnapSettingsChanged_) {
      onSnapSettingsChanged_();
    }
  }
}

void UIFSMAdapter::setGridSettingsChangedCallback(
    GridSettingsCallback callback) {
  onGridSettingsChanged_ = std::move(callback);
}

void UIFSMAdapter::setSnapSettingsChangedCallback(
    SnapSettingsCallback callback) {
  onSnapSettingsChanged_ = std::move(callback);
}

void UIFSMAdapter::setFigureChangedCallback(FigureChangedCallback callback) {
  onFigureChanged_ = std::move(callback);
}

void UIFSMAdapter::setCameraChangedCallback(CameraChangedCallback callback) {
  onCameraChanged_ = std::move(callback);
}

void UIFSMAdapter::setGridGeometryDirtyCallback(
    GridGeometryDirtyCallback callback) {
  onGridGeometryDirty_ = std::move(callback);
}

// ==========================================================================
// Grid State Query Methods (for GridManager)
// These methods provide access to individual grid settings properties
// ==========================================================================

bool UIFSMAdapter::isGridEnabled() const {
  /// Return the grid visibility from local storage
  return gridSettings_.visible;
}

float UIFSMAdapter::getGridMajorSpacing() const {
  /// Return the major grid spacing from local storage
  return gridSettings_.majorSpacing;
}

float UIFSMAdapter::getGridMinorSpacing() const {
  /// Calculate and return the minor grid spacing
  if (gridSettings_.showMinorLines && gridSettings_.minorDivisions > 0) {
    return gridSettings_.majorSpacing / gridSettings_.minorDivisions;
  }
  return gridSettings_.majorSpacing;
}

glm::vec4 UIFSMAdapter::getGridMajorColor() const {
  /// Return the major grid color from local storage
  return gridSettings_.color;
}

glm::vec4 UIFSMAdapter::getGridMinorColor() const {
  /// Return the minor grid color from local storage
  return gridSettings_.minorColor;
}

float UIFSMAdapter::getGridOpacity() const {
  /// Return the grid opacity from local storage
  return gridSettings_.opacity;
}

bool UIFSMAdapter::getGridShowAxes() const {
  /// Return the axes visibility from local storage
  return gridSettings_.showAxes;
}

bool UIFSMAdapter::getGridShowOrigin() const {
  /// Return the origin visibility from local storage
  return gridSettings_.showOrigin;
}

bool UIFSMAdapter::getGridShowMinorLines() const {
  /// Return the minor lines visibility from local storage
  return gridSettings_.showMinorLines;
}

int UIFSMAdapter::getGridMinorDivisions() const {
  /// Return the minor divisions from local storage
  return gridSettings_.minorDivisions;
}

// ==========================================================================
// Snap State Query Methods (for SnapManager)
// These methods provide access to individual snap settings properties
// ==========================================================================

bool UIFSMAdapter::isSnapGridEnabled() const {
  /// Return the grid snap enabled flag from local storage
  return snapSettings_.gridEnabled;
}

bool UIFSMAdapter::isSnapEndpointEnabled() const {
  /// Return the endpoint snap enabled flag from local storage
  return snapSettings_.endpointEnabled;
}

bool UIFSMAdapter::isSnapMidpointEnabled() const {
  /// Return the midpoint snap enabled flag from local storage
  return snapSettings_.midpointEnabled;
}

bool UIFSMAdapter::isSnapCenterEnabled() const {
  /// Return the center snap enabled flag from local storage
  return snapSettings_.centerEnabled;
}

bool UIFSMAdapter::isSnapIntersectionEnabled() const {
  /// Return the intersection snap enabled flag from local storage
  return snapSettings_.intersectionEnabled;
}

bool UIFSMAdapter::isSnapNearestEnabled() const {
  /// Return the nearest point snap enabled flag from local storage
  return snapSettings_.nearestEnabled;
}

bool UIFSMAdapter::isSnapTangentEnabled() const {
  /// Return the tangent snap enabled flag from local storage
  return snapSettings_.tangentEnabled;
}

bool UIFSMAdapter::isSnapPerpendicularEnabled() const {
  /// Return the perpendicular snap enabled flag from local storage
  return snapSettings_.perpendicularEnabled;
}

float UIFSMAdapter::getSnapTolerance() const {
  /// Return the snap tolerance from local storage
  return snapSettings_.tolerancePixels;
}

bool UIFSMAdapter::getSnapShowIndicators() const {
  /// Return the snap indicators visibility from local storage
  return snapSettings_.showIndicators;
}

glm::vec4 UIFSMAdapter::getSnapIndicatorColor() const {
  /// Return the snap indicator color from local storage
  return snapSettings_.indicatorColor;
}

} // namespace view
