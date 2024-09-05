#pragma once

class IController {
protected:
	enum class ControllerState {
		none,
		triangleByCenter,
		triangleByCenter_centerSelected,
		triangleByCorners,
		triangleByCorners_firstCornerSelected,
		triangleByCorners_secondCornerSelected,
		squareByCenter,
		squareByCenter_centerSelected,
		squareByCorners,
		squareByCorners_firstCornerSelected,
		squareByCorners_secondCornerSelected,
		ngonByCenter,
		ngonByCenter_centerSelected,
		circleByCenter,
		circleByCenter_centerSelected,
		// кривые тут же
	};
	ControllerState currentControllerState_{ ControllerState::none };
public:
	// события непосредственного ввода
	enum class InputState {
		undefined,
		// текущее состояние клавиши
		down,
		released,
		// алиасы для позиции указателя
		hovered = down,
		unhovered = released
	};
	virtual void onLeftMouseButton(InputState) = 0;
	virtual void onWheelMouseButton(InputState) = 0;
	virtual void onRightMouseButton(InputState) = 0;
	virtual void onMouseHover(InputState, float x, float y) = 0; // x, y - нормализованные координаты (0..1)
	virtual void onScroll(float) = 0;

	// события GUI
	virtual void addTriangleByCenter() = 0;
	virtual void addTriangleByCorners() = 0;
	virtual void addSquareByCenter() = 0;
	virtual void addSquareByCorners() = 0;
	virtual void addNgonByCenter() = 0;
	virtual void addCircleByCenter() = 0;
	virtual void removeFigure() = 0;        
};