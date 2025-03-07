#pragma once

#include <Controller/IController.hpp>
#include <Model/FlatFigure.hpp>

namespace controller {

// тут будет обработка всех действий пользователя, вызов модели
class OpenglImguiController final : public IController {
private:
	std::shared_ptr<model::FlatFigures> sp_model_;

	state::Button    prevLeftMouseButtonState_{state::Button::released};
	state::Button    prevWheelMouseButtonState_{state::Button::released};
	state::Button    prevRightMouseButtonState_{state::Button::released};
	state::Workspace prevWorkspaceHoverState_{state::Workspace::unhovered};
	glm::vec2        prevScreenspaceMousePosition_{0.0f, 0.0f};
public:
	OpenglImguiController(std::shared_ptr<model::FlatFigures>);

	// события непосредственного ввода
	void updateLeftMouseButtonState(state::Button); // обработка расположения фигур
	void updateWheelMouseButtonState(state::Button); // перемещение по пространству
	void updateRightMouseButtonState(state::Button); // отмена + перемещение по пространству
	void updateWorkspaceHoverState(state::Workspace); // Пока не придумал зачем
	void updateScreenspaceMousePosition(glm::vec2 ssp);
	void updateScroll(float);

	// события GUI
	void addLine();
	void addTriangleByCenter();
	void addTriangleByCorners();
	void addSquareByCenter();
	void addSquareByCorners();
	void addNgonByCenter();
	void addCircleByCenter();
	void removeFigure();
};

} // namespace controller