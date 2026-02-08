#include "FSM.hpp"

#include <Controller/FigureCreator.hpp>

#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace fsm {

namespace {
/// Returns the path to the FSM configuration file
/// Searches in multiple locations to support different deployment scenarios
std::string getConfigPath() {
  // List of paths to search, in order of priority
  const std::vector<std::string> search_paths = {
      // Current directory (for development)
      "Src/Controller/fsm_config.yaml",
      // Executable directory (for installed builds)
      std::filesystem::path(std::filesystem::current_path() / "fsm_config.yaml")
          .string(),
      // Parent directory (for build directory layouts)
      std::filesystem::path(std::filesystem::current_path().parent_path() /
                            "Src" / "Controller" / "fsm_config.yaml")
          .string(),
  };

  for (const auto &path : search_paths) {
    if (std::filesystem::exists(path)) {
      spdlog::info("Found FSM configuration at: {}", path);
      return path;
    }
  }

  // If no config file found, throw an exception with helpful message
  std::string error_msg = "FSM configuration file not found. Searched paths:\n";
  for (const auto &path : search_paths) {
    error_msg += "  - " + path + "\n";
  }
  error_msg +=
      "Please ensure fsm_config.yaml exists in one of these locations.";
  throw std::runtime_error(error_msg);
}
} // namespace

Machine::Machine() : model_(nullptr) {
  try {
    // Initialize FSMConfig with the configuration file
    std::string config_path = getConfigPath();
    fsm_ = std::make_unique<fsmconfig::StateMachine>(config_path);

    // Register callbacks for state-specific behavior
    registerCallbacks();

    // Start the state machine
    fsm_->start();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize FSM: {}", e.what());
    throw;
  }
}

Machine::~Machine() = default;

void Machine::registerCallbacks() {
  // Register state callbacks
  fsm_->registerStateCallback("Idle", "on_enter", &Machine::on_idle_enter,
                              this);
  fsm_->registerStateCallback("Idle", "on_exit", &Machine::on_idle_exit, this);
  fsm_->registerStateCallback("DrawingProcessing", "on_enter",
                              &Machine::on_drawing_enter, this);
  fsm_->registerStateCallback("DrawingProcessing", "on_exit",
                              &Machine::on_drawing_exit, this);
  fsm_->registerStateCallback("MoveFirstPoint", "on_enter",
                              &Machine::on_move_first_point_enter, this);
  fsm_->registerStateCallback("MoveFirstPoint", "on_exit",
                              &Machine::on_move_first_point_exit, this);
  fsm_->registerStateCallback("PlaneSelection", "on_enter",
                              &Machine::on_plane_selection_enter, this);
  fsm_->registerStateCallback("PlaneSelection", "on_exit",
                              &Machine::on_plane_selection_exit, this);
  fsm_->registerStateCallback("SketchEdit", "on_enter",
                              &Machine::on_sketch_edit_enter, this);
  fsm_->registerStateCallback("SketchEdit", "on_exit",
                              &Machine::on_sketch_edit_exit, this);

  // Register action callbacks
  fsm_->registerAction("log_idle_state", &Machine::log_idle_state, this);
  fsm_->registerAction("log_drawing_state", &Machine::log_drawing_state, this);
  fsm_->registerAction("log_move_first_point_state",
                       &Machine::log_move_first_point_state, this);
  fsm_->registerAction("log_plane_selection_state",
                       &Machine::log_plane_selection_state, this);
  fsm_->registerAction("log_sketch_edit_state", &Machine::log_sketch_edit_state,
                       this);

  // Register transition callbacks
  fsm_->registerTransitionCallback("Idle", "DrawingProcessing",
                                   &Machine::on_start_drawing, this);
  fsm_->registerTransitionCallback("DrawingProcessing", "MoveFirstPoint",
                                   &Machine::on_move_to_first_point, this);
  fsm_->registerTransitionCallback("MoveFirstPoint", "DrawingProcessing",
                                   &Machine::on_update_drawing, this);
  fsm_->registerTransitionCallback("DrawingProcessing", "Idle",
                                   &Machine::on_complete_figure, this);
  fsm_->registerTransitionCallback("MoveFirstPoint", "Idle",
                                   &Machine::on_cancel_figure, this);
  fsm_->registerTransitionCallback("Idle", "PlaneSelection",
                                   &Machine::on_enter_sketch_mode, this);
  fsm_->registerTransitionCallback("PlaneSelection", "SketchEdit",
                                   &Machine::on_select_plane, this);
  fsm_->registerTransitionCallback("SketchEdit", "Idle",
                                   &Machine::on_exit_sketch_mode, this);
}

// State callback implementations

void Machine::on_idle_enter() { spdlog::debug("Entering Idle state"); }

void Machine::on_idle_exit() { spdlog::debug("Exiting Idle state"); }

void Machine::on_drawing_enter() {
  spdlog::debug("Entering DrawingProcessing state");
}

void Machine::on_drawing_exit() {
  spdlog::debug("Exiting DrawingProcessing state");
}

void Machine::on_move_first_point_enter() {
  spdlog::debug("Entering MoveFirstPoint state");
}

void Machine::on_move_first_point_exit() {
  spdlog::debug("Exiting MoveFirstPoint state");
}

// Action callback implementations

void Machine::log_idle_state() { spdlog::info("Current state: Idle"); }

void Machine::log_drawing_state() {
  spdlog::info("Current state: DrawingProcessing");
}

void Machine::log_move_first_point_state() {
  spdlog::info("Current state: MoveFirstPoint");
}

/// Transition callback implementations

void Machine::on_start_drawing(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  /// Create the appropriate FigureCreator based on the event name
  currentCreator_ = createFigureCreator(event.event_name);
  if (currentCreator_) {
    collectedPoints_.clear();
    spdlog::info("Started drawing figure: {}", currentCreator_->displayName());
  }
}

void Machine::on_move_to_first_point(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  /// Extract mouse position from event data and collect the first point
  auto x_it = event.data.find("x");
  auto y_it = event.data.find("y");
  if (x_it != event.data.end() && y_it != event.data.end()) {
    float x = x_it->second.asFloat();
    float y = y_it->second.asFloat();
    glm::vec2 point(x, y);

    if (currentCreator_) {
      if (currentCreator_->validatePoint(collectedPoints_, point)) {
        collectedPoints_.push_back(point);
        spdlog::info("Collected point {}/{}: ({}, {})", collectedPoints_.size(),
                     currentCreator_->getRequiredPoints(), x, y);
      }
    }
  }
}

void Machine::on_update_drawing(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  /// Extract mouse position and add to collected points
  auto x_it = event.data.find("x");
  auto y_it = event.data.find("y");
  if (x_it != event.data.end() && y_it != event.data.end() && currentCreator_) {
    float x = x_it->second.asFloat();
    float y = y_it->second.asFloat();
    glm::vec2 point(x, y);

    /// Validate and add point
    if (currentCreator_->validatePoint(collectedPoints_, point)) {
      collectedPoints_.push_back(point);
      spdlog::info("Collected point {}/{}: ({}, {})", collectedPoints_.size(),
                   currentCreator_->getRequiredPoints(), x, y);

      /// Update preview if we have enough points
      currentCreator_->updatePreview(collectedPoints_);
    }
  }
}

void Machine::on_complete_figure(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  /// Check if we can complete the figure and create it
  if (currentCreator_ && currentCreator_->canComplete(collectedPoints_)) {
    try {
      currentCreator_->createFigure(collectedPoints_);
      spdlog::info("Successfully created figure: {}",
                   currentCreator_->displayName());
    } catch (const std::exception &e) {
      spdlog::error("Failed to create figure: {}", e.what());
    }
  } else if (currentCreator_) {
    spdlog::warn("Cannot complete figure: need {} points, have {}",
                 currentCreator_->getRequiredPoints(), collectedPoints_.size());
  }

  /// Reset drawing state
  currentCreator_.reset();
  collectedPoints_.clear();
}

void Machine::on_cancel_figure(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  /// Clean up drawing state on cancellation
  currentCreator_.reset();
  collectedPoints_.clear();
  spdlog::info("Figure drawing cancelled");
}

/**
 * @brief Creates a FigureCreator based on the event name
 * @param eventName The name of the event that triggered the transition
 * @return Unique pointer to the created FigureCreator, or nullptr if unknown
 * @throws std::runtime_error if model_ is not set
 */
std::unique_ptr<controller::FigureCreator>
Machine::createFigureCreator(const std::string &eventName) {
  if (!model_) {
    throw std::runtime_error(
        "Model not set. Call setModel() before drawing figures.");
  }

  if (eventName == "OnAddLine") {
    return std::make_unique<controller::LineCreator>(model_);
  } else if (eventName == "OnAddTriangleByCenter") {
    return std::make_unique<controller::TriangleByCenterCreator>(model_);
  } else if (eventName == "OnAddTriangleByCorners") {
    return std::make_unique<controller::TriangleByCornersCreator>(model_);
  } else if (eventName == "OnAddSquareByCenter") {
    return std::make_unique<controller::SquareByCenterCreator>(model_);
  } else if (eventName == "OnAddSquareByCorners") {
    return std::make_unique<controller::SquareByCornersCreator>(model_);
  } else if (eventName == "OnAddNgonByCenter") {
    return std::make_unique<controller::NgonByCenterCreator>(model_, 6);
  } else if (eventName == "OnAddCircleByCenter") {
    return std::make_unique<controller::CircleByCenterCreator>(model_);
  }

  spdlog::warn("Unknown event name for FigureCreator creation: {}", eventName);
  return nullptr;
}

/// Sketch mode state callback implementations

void Machine::on_plane_selection_enter() {
  spdlog::debug("Entering PlaneSelection state");
}

void Machine::on_plane_selection_exit() {
  spdlog::debug("Exiting PlaneSelection state");
}

void Machine::on_sketch_edit_enter() {
  spdlog::debug("Entering SketchEdit state");
}

void Machine::on_sketch_edit_exit() {
  spdlog::debug("Exiting SketchEdit state");
}

/// Sketch mode action callback implementations

void Machine::log_plane_selection_state() {
  spdlog::info("Current state: PlaneSelection (Select a sketch plane)");
}

void Machine::log_sketch_edit_state() {
  spdlog::info("Current state: SketchEdit (2D drawing on sketch plane)");
}

/// Sketch mode transition callback implementations

void Machine::on_enter_sketch_mode(const fsmconfig::TransitionEvent&event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);
}

void Machine::on_select_plane(const fsmconfig::TransitionEvent&event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  /// Extract plane index from event data
  auto planeIndex_it = event.data.find("planeIndex");
  if (planeIndex_it != event.data.end()) {
    int planeIndex = planeIndex_it->second.asInt();
    spdlog::info("Selected sketch plane: {}", planeIndex);
  }
}

void Machine::on_exit_sketch_mode(const fsmconfig::TransitionEvent&event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);
}

} // namespace fsm
