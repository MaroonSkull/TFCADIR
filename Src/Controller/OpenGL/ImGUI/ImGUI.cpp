// #include <glm/ext/matrix_transform.hpp>
// #include <View/IView.hpp>
// #include <View/OpenGL/ImGUI.hpp> // breaks SOLID
#include <Controller/FSM.hpp>
#include <Controller/OpenGL/ImGUI.hpp>

// #include <boost/signals2.hpp>

#include <algorithm>
#include <spdlog/spdlog.h>
#include <stdexcept>

using namespace std::string_literals;

namespace controller {

OpenglImguiController::OpenglImguiController(
    std::shared_ptr<model::FlatFigures> sp_model)
    : sp_model_(sp_model) {
  if (sp_model_ == nullptr)
    throw std::invalid_argument{
        "pModel cannot be nullptr in controller constructor!"};
}

/// Updates the current figure state on left mouse button click
void OpenglImguiController::updateLeftMouseButtonState(state::Button state) {
  if (state == prevLeftMouseButtonState_)
    return;

  // todo can replace switch-case with a generic template function (with TAD?)
  switch (prevLeftMouseButtonState_ = state) {
  case state::Button::down:
    spdlog::info("onLeftMouseButton pressed");
    break;

  case state::Button::released:
    spdlog::info("onLeftMouseButton released");
    break;

  default:
    using namespace std::string_literals;
    throw std::runtime_error{"Unknown state have been recieved in "s +
                             __PRETTY_FUNCTION__ + "!"};
  }

  // Here we can emit some signal to redraw scene or update view,
  // but it depends from IView...
  // mustBeRedrawSignal();
}

/// Touchpads may lack middle mouse button functionality
/// Can bind a fixed camera configuration in 3D space
/// (position + view direction, possibly axis-bound)
void OpenglImguiController::updateWheelMouseButtonState(state::Button state) {
  if (state == prevWheelMouseButtonState_)
    return;
  switch (prevWheelMouseButtonState_ = state) {
  case state::Button::down:
    spdlog::info("onWheelMouseButton pressed");

    sp_model_->camera_.position = {0.0f, 0.0f, -1.0f};

    break;

  case state::Button::released:
    spdlog::info("onWheelMouseButton released");
    break;

  default:
    using namespace std::string_literals;
    throw std::runtime_error{"Unknown state have been recieved in "s +
                             __PRETTY_FUNCTION__ + "!"};
  }
}

/// Right mouse button handles movement
/// Follow the pointer position when RMB is pressed (shift by delta?)
void OpenglImguiController::updateRightMouseButtonState(state::Button state) {
  if (state == prevRightMouseButtonState_)
    return;
  switch (prevRightMouseButtonState_ = state) {
  case state::Button::down:
    spdlog::info("onRightMouseButton pressed");
    break;

  case state::Button::released:
    spdlog::info("onRightMouseButton released");
    break;

  default:
    using namespace std::string_literals;
    throw std::runtime_error{"Unknown state have been recieved in "s +
                             __PRETTY_FUNCTION__ + "!"};
  }
}

/// Updates current pointer XY coordinates, records deltas
void OpenglImguiController::updateWorkspaceHoverState(state::Workspace state) {
  // todo there's a problem during active mouse movement or low FPS:
  // the mouse coordinate window intercepts the hover event and triggers
  // unhovered/hovered
  if (state == prevWorkspaceHoverState_)
    return;
  switch (prevWorkspaceHoverState_ = state) {
  case state::Workspace::hovered:
    spdlog::info("onMouseHover hovered");
    break;

  case state::Workspace::unhovered:
    spdlog::info("onMouseHover unhovered");
    break;

  default:
    using namespace std::string_literals;
    throw std::runtime_error{"Unknown state have been recieved in "s +
                             __PRETTY_FUNCTION__ + "!"};
  }
}

void OpenglImguiController::updateScreenspaceMousePosition(
    glm::vec2 screenspacePosition) {
  if (prevScreenspaceMousePosition_ == screenspacePosition)
    return;

  prevScreenspaceMousePosition_ = screenspacePosition;

  spdlog::info("{}: normalizedPosition = ({}, {})", __PRETTY_FUNCTION__,
               screenspacePosition.x, screenspacePosition.y);

  fsm_.process_event(fsm::events::OnMouseMove(screenspacePosition));
}

// Changing the scale of the visible area
void OpenglImguiController::updateScroll(float momentWheel) {
  if (momentWheel == 0.0f)
    return;

  // TODO: Move reaction to fsm
  sp_model_->camera_.position.z +=
      0.1f * momentWheel * std::abs(sp_model_->camera_.position.z);
  sp_model_->camera_.position.z =
      std::clamp(sp_model_->camera_.position.z, -100.0f, -1e-6f);

  const auto scrollDirection = (momentWheel > 0.0f ? "up" : "down");
  spdlog::info("onScroll {}, moment = {}, z = {}", scrollDirection, momentWheel,
               sp_model_->camera_.position.z);
}

void OpenglImguiController::addLine() {
  fsm_.process_event(fsm::events::OnAddLine());
}

void OpenglImguiController::addTriangleByCenter() {
  fsm_.process_event(fsm::events::OnAddTriangleByCenter());
}

void OpenglImguiController::addTriangleByCorners() {
  fsm_.process_event(fsm::events::OnAddTriangleByCorners());
}

void OpenglImguiController::addSquareByCenter() {
  fsm_.process_event(fsm::events::OnAddSquareByCenter());
}

void OpenglImguiController::addSquareByCorners() {
  fsm_.process_event(fsm::events::OnAddSquareByCorners());
}

void OpenglImguiController::addNgonByCenter() {
  fsm_.process_event(fsm::events::OnAddNgonByCenter());
}

void OpenglImguiController::addCircleByCenter() {
  fsm_.process_event(fsm::events::OnAddCircleByCenter());
}

// For future, need to learn how to detect collisions first
void OpenglImguiController::removeFigure() {}

} // namespace controller