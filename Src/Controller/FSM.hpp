#pragma once

#include <fsmconfig/state_machine.hpp>
#include <glm/glm.hpp>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>

namespace fsm {
namespace state {
enum class Button { down, released };

enum class Workspace { unhovered, hovered };
} // namespace state

// Events
namespace events {
struct OnMouseMove {
  glm::vec2 position;
  OnMouseMove(glm::vec2 pos) : position(pos) {}
};

struct OnAddTriangleByCenter {};
struct OnAddTriangleByCorners {};
struct OnAddSquareByCenter {};
struct OnAddSquareByCorners {};
struct OnAddNgonByCenter {};
struct OnAddCircleByCenter {};
struct OnAddLine {};
struct OnFigureComplete {};
struct OnFigureCancel {};
} // namespace events

// State enumeration (kept for backward compatibility)
enum class State { Idle, DrawingProcessing, MoveFirstPoint };

/**
 * @brief State Machine wrapper using FSMConfig library
 *
 * This class provides a backward-compatible interface to FSMConfig,
 * maintaining the original template-based `process_event<Event>` API
 * while internally using FSMConfig's string-based event system.
 */
class Machine {
private:
  std::unique_ptr<fsmconfig::StateMachine> fsm_;

  /**
   * @brief Converts glm::vec2 to VariableValue map
   * @param position The position to convert
   * @return Map containing x and y coordinates as VariableValue
   */
  std::map<std::string, fsmconfig::VariableValue>
  vec2ToVariableMap(const glm::vec2 &position) const {
    std::map<std::string, fsmconfig::VariableValue> data;
    data["x"] = fsmconfig::VariableValue(static_cast<float>(position.x));
    data["y"] = fsmconfig::VariableValue(static_cast<float>(position.y));
    return data;
  }

  /**
   * @brief Converts VariableValue map to glm::vec2
   * @param data The map containing x and y coordinates
   * @return glm::vec2 with the coordinates from the map
   */
  glm::vec2 variableMapToVec2(
      const std::map<std::string, fsmconfig::VariableValue> &data) const {
    glm::vec2 result(0.0f, 0.0f);
    auto x_it = data.find("x");
    auto y_it = data.find("y");
    if (x_it != data.end()) {
      result.x = static_cast<float>(x_it->second.asFloat());
    }
    if (y_it != data.end()) {
      result.y = static_cast<float>(y_it->second.asFloat());
    }
    return result;
  }

  /**
   * @brief Registers callbacks for state-specific behavior
   */
  void registerCallbacks();

public:
  /**
   * @brief Constructor - initializes FSMConfig with configuration file
   * @throws fsmconfig::ConfigException if configuration file cannot be loaded
   */
  Machine();

  /**
   * @brief Destructor
   */
  ~Machine() = default;

  // Copy prohibition
  Machine(const Machine &) = delete;
  Machine &operator=(const Machine &) = delete;

  // Move permission
  Machine(Machine &&) noexcept = default;
  Machine &operator=(Machine &&) noexcept = default;

  /**
   * @brief Processes an event and performs state transitions
   * @tparam Event Type of event to process
   * @param event The event to process
   *
   * Maintains backward compatibility with the original template-based API.
   * Internally converts the event to FSMConfig's string-based event system.
   */
  template <typename Event> void process_event(const Event &event);

  /**
   * @brief Returns the current state type name (for debugging)
   * @return Current state name as string
   */
  std::string get_current_state_name() const {
    return fsm_ ? fsm_->getCurrentState() : "Unknown";
  }

  /**
   * @brief Gets the current state as enum (for backward compatibility)
   * @return Current state enum value
   */
  State get_current_state() const;

  /**
   * @brief Sets the current state (for backward compatibility)
   * @param new_state The new state to set
   * @note This is a no-op in FSMConfig as state transitions are event-driven
   */
  void set_state(State new_state);

  /**
   * @brief Gets the underlying FSMConfig StateMachine
   * @return Pointer to the FSMConfig StateMachine
   */
  fsmconfig::StateMachine *get_fsm() { return fsm_.get(); }
  const fsmconfig::StateMachine *get_fsm() const { return fsm_.get(); }

  // Callback methods for state-specific behavior
  void on_idle_enter();
  void on_idle_exit();
  void on_drawing_enter();
  void on_drawing_exit();
  void on_move_first_point_enter();
  void on_move_first_point_exit();
  void log_idle_state();
  void log_drawing_state();
  void log_move_first_point_state();
  void on_start_drawing(const fsmconfig::TransitionEvent &event);
  void on_move_to_first_point(const fsmconfig::TransitionEvent &event);
  void on_update_drawing(const fsmconfig::TransitionEvent &event);
  void on_complete_figure(const fsmconfig::TransitionEvent &event);
  void on_cancel_figure(const fsmconfig::TransitionEvent &event);
};

// Event handler implementations

template <typename Event> void Machine::process_event(const Event &event) {
  if (!fsm_) {
    return;
  }

  // Handle events based on type
  if constexpr (std::is_same_v<Event, events::OnMouseMove>) {
    // Mouse movement event - trigger with position data
    auto data = vec2ToVariableMap(event.position);
    fsm_->triggerEvent("OnMouseMove", data);
  } else if constexpr (std::is_same_v<Event, events::OnAddTriangleByCenter>) {
    fsm_->triggerEvent("OnAddTriangleByCenter");
  } else if constexpr (std::is_same_v<Event, events::OnAddTriangleByCorners>) {
    fsm_->triggerEvent("OnAddTriangleByCorners");
  } else if constexpr (std::is_same_v<Event, events::OnAddSquareByCenter>) {
    fsm_->triggerEvent("OnAddSquareByCenter");
  } else if constexpr (std::is_same_v<Event, events::OnAddSquareByCorners>) {
    fsm_->triggerEvent("OnAddSquareByCorners");
  } else if constexpr (std::is_same_v<Event, events::OnAddNgonByCenter>) {
    fsm_->triggerEvent("OnAddNgonByCenter");
  } else if constexpr (std::is_same_v<Event, events::OnAddCircleByCenter>) {
    fsm_->triggerEvent("OnAddCircleByCenter");
  } else if constexpr (std::is_same_v<Event, events::OnAddLine>) {
    fsm_->triggerEvent("OnAddLine");
  } else if constexpr (std::is_same_v<Event, events::OnFigureComplete>) {
    fsm_->triggerEvent("OnFigureComplete");
  } else if constexpr (std::is_same_v<Event, events::OnFigureCancel>) {
    fsm_->triggerEvent("OnFigureCancel");
  }
}

inline State Machine::get_current_state() const {
  if (!fsm_) {
    return State::Idle;
  }
  std::string state_name = fsm_->getCurrentState();
  if (state_name == "Idle") {
    return State::Idle;
  } else if (state_name == "DrawingProcessing") {
    return State::DrawingProcessing;
  } else if (state_name == "MoveFirstPoint") {
    return State::MoveFirstPoint;
  }
  return State::Idle;
}

inline void Machine::set_state(State new_state) {
  // Note: FSMConfig is event-driven, so direct state setting is not supported
  // This method is kept for backward compatibility but does nothing
  spdlog::warn(
      "set_state() called but FSMConfig does not support direct state setting. "
      "Use triggerEvent() to perform state transitions.");
  (void)new_state;
}

} // namespace fsm
