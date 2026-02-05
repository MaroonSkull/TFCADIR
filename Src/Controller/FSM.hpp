#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <variant>

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
}; // namespace events

// State classes - must be defined before Machine for std::variant
class Idle {
public:
  Idle() = default;
};

class DrawingProcessing {
public:
  DrawingProcessing() = default;
};

class MoveFirstPoint {
public:
  MoveFirstPoint() = default;
};

// Simple State Machine using variant
class Machine {
private:
  std::variant<std::monostate, Idle, DrawingProcessing, MoveFirstPoint>
      current_state_;

public:
  Machine() : current_state_(Idle{}) {}

  template <typename Event> void process_event(const Event &event) {
    // Simple event processing - log the event
    // State transitions can be implemented here as needed
    (void)event; // Suppress unused parameter warning
  }
};

} // namespace fsm
