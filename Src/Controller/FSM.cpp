#include "FSM.hpp"

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

Machine::Machine() {
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

  // Register action callbacks
  fsm_->registerAction("log_idle_state", &Machine::log_idle_state, this);
  fsm_->registerAction("log_drawing_state", &Machine::log_drawing_state, this);
  fsm_->registerAction("log_move_first_point_state",
                       &Machine::log_move_first_point_state, this);

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

// Transition callback implementations

void Machine::on_start_drawing(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);
}

void Machine::on_move_to_first_point(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);

  // Extract mouse position from event data if available
  auto x_it = event.data.find("x");
  auto y_it = event.data.find("y");
  if (x_it != event.data.end() && y_it != event.data.end()) {
    float x = x_it->second.asFloat();
    float y = y_it->second.asFloat();
    spdlog::debug("Mouse position: ({}, {})", x, y);
  }
}

void Machine::on_update_drawing(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);
}

void Machine::on_complete_figure(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);
}

void Machine::on_cancel_figure(const fsmconfig::TransitionEvent &event) {
  spdlog::info("Transition: {} -> {} on event {}", event.from_state,
               event.to_state, event.event_name);
}

} // namespace fsm
