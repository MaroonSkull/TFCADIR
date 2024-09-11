#pragma once

#include <Controller/IController.hpp>
#include <Model/FlatFigure.hpp>

#include <queue>



// тут будет обработка всех действий пользователя, вызов модели
class OpenglImguiController final : public IController {
private:
	std::shared_ptr<model::FlatFigures> sp_model_;
	std::queue<model::Memento> modelMementoQueue_;

	float mousePositionNormalizedX_{};
	float mousePositionNormalizedY_{};
	InputState currentStateLeftMouseButton_{ InputState::undefined };
	InputState currentStateWheelMouseButton_{ InputState::undefined };
	InputState currentStateRightMouseButton_{ InputState::undefined };
	InputState currentStateMouseHover_{ InputState::undefined };
public:
	OpenglImguiController(std::shared_ptr<model::FlatFigures>);

	// события непосредственного ввода
	void onLeftMouseButton(InputState); // обработка расположения фигур
	void onWheelMouseButton(InputState); // ресайз + перемещение по пространству
	void onRightMouseButton(InputState); // перемещение по пространству
	void onMouseHover(InputState, float x, float y);
	void onScroll(float);

	// события GUI
	void addTriangleByCenter();
	void addTriangleByCorners();
	void addSquareByCenter();
	void addSquareByCorners();
	void addNgonByCenter();
	void addCircleByCenter();
	void removeFigure();
};