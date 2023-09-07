#pragma once
#include <queue>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <IController.hpp>
#include <flatFigureModel.hpp>

// тут будет обработка всех действий пользователя, вызов модели
class OpenglImguiController final : public IController {
private:
	model::FlatFigures* pModel_;
	std::queue<model::FlatFigures::Memento> modelMementoQueue_;

	float mousePositionNormalizedX_{};
	float mousePositionNormalizedY_{};
	InputState currentStateLeftMouseButton_{ InputState::undefined };
	InputState currentStateWheelMouseButton_{ InputState::undefined };
	InputState currentStateRightMouseButton_{ InputState::undefined };
	InputState currentStateMouseHover_{ InputState::undefined };
public:
	OpenglImguiController(model::FlatFigures *);

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