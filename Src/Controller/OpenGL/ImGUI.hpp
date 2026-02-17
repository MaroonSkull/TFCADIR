#pragma once

#include <Controller/IController.hpp>
#include <Model/FlatFigure.hpp>

namespace controller {

// this is where all user actions will be handled, model calls
class OpenglImguiController final : public IController {
private:
  std::shared_ptr<model::FlatFigures> sp_model_;

  state::Button prevLeftMouseButtonState_{state::Button::released};
  state::Button prevWheelMouseButtonState_{state::Button::released};
  state::Button prevRightMouseButtonState_{state::Button::released};
  state::Workspace prevWorkspaceHoverState_{state::Workspace::unhovered};
  glm::vec2 prevScreenspaceMousePosition_{0.0f, 0.0f};

public:
  OpenglImguiController(std::shared_ptr<model::FlatFigures>);

  // direct input events
  void updateLeftMouseButtonState(state::Button);  // handling figure placement
  void updateWheelMouseButtonState(state::Button); // navigation through space
  void updateRightMouseButtonState(
      state::Button); // cancel + navigation through space
  void updateWorkspaceHoverState(
      state::Workspace); // haven't figured out purpose yet
  void updateScreenspaceMousePosition(glm::vec2 ssp);
  void updateScroll(float);

  // GUI events
  void addLine();
  void addTriangleByCenter();
  void addTriangleByCorners();
  void addSquareByCenter();
  void addSquareByCorners();
  void addNgonByCenter();
  void addCircleByCenter();
  void removeFigure();

  // Phase 3: Access to model for UI components
  std::shared_ptr<model::FlatFigures> getModel() const { return sp_model_; }
};

} // namespace controller