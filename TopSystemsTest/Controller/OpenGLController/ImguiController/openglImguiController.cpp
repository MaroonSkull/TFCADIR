﻿#include <openglImguiController.hpp>

OpenglImguiController::OpenglImguiController(model::FlatFigures* pModel) : pModel_(pModel) {
	if (pModel_ == nullptr)
		throw std::exception{ "pModel cannot be nullptr in controller constructor!" };
}

// Обновлять состояние текущей фигуры при ЛКМ
// currentControllerState_
void OpenglImguiController::onLeftMouseButton(InputState state) {
	if (state == currentStateLeftMouseButton_)
		return;
	// todo можно switch-case сменить на std::map и набор лямбд.
	// Или вообще на универсальную шаблонную функцию (с TAD?)
	switch (currentStateLeftMouseButton_ = state) {
	case InputState::down:
		spdlog::info("onLeftMouseButton pressed");
		switch (currentControllerState_) {
		case ControllerState::triangleByCorners:
			spdlog::info("triangleByCorners selected");
			pModel_->createFigure<model::Triangle>(mousePositionNormalizedX_, mousePositionNormalizedY_);
			break;

		}
		break;

	case InputState::released:
		spdlog::info("onLeftMouseButton released");
		break;

	default:
	case InputState::undefined:
		spdlog::error("onLeftMouseButton undefined");
		throw std::exception{ "undefined state have been recieved in OpenglImguiController::onLeftMouseButton" };
	}
}

// На тачпадах возможно отсутствие функционала нажатия на СКМ
// Можно повесить в 3д-пространстве выбор какой-то фиксированной конфигурации камеры
// (позиция + направление взгляда, мб привязка к осям)
void OpenglImguiController::onWheelMouseButton(InputState state) {
	if (state == currentStateWheelMouseButton_)
		return;
	switch (currentStateWheelMouseButton_ = state) {
	case InputState::down:
		spdlog::info("onWheelMouseButton pressed");
		break;

	case InputState::released:
		spdlog::info("onWheelMouseButton released");
		break;

	default:
	case InputState::undefined:
		spdlog::error("onWheelMouseButton undefined");
		throw std::exception{ "undefined state have been recieved in OpenglImguiController::onWheelMouseButton" };
	}
}

// ПКМ отвечает за перемещение
// При зажатой пкм следовать за позицией указателя, (смещать на дельту?)
void OpenglImguiController::onRightMouseButton(InputState state) {
	if (state == currentStateRightMouseButton_)
		return;
	switch (currentStateRightMouseButton_ = state) {
	case InputState::down:
		spdlog::info("onRightMouseButton pressed");
		break;

	case InputState::released:
		spdlog::info("onRightMouseButton released");
		break;

	default:
	case InputState::undefined:
		spdlog::error("onRightMouseButton undefined");
		throw std::exception{ "undefined state have been recieved in OpenglImguiController::onRightMouseButton" };
	}
}

// Обновляем текущие xy указателя, записываем дельты 
void OpenglImguiController::onMouseHover(InputState state, float x, float y) {
	// todo тут проблема при активном перемещении мыши или при низком фпс:
	// окно с координатами мыши перехватывает событие hover и срабатывает unhovered/hovered
	if (state == currentStateMouseHover_)
		return;
	switch (currentStateMouseHover_ = state) {
	case InputState::hovered:
		spdlog::info("onMouseHover hovered");
		break;

	case InputState::released:
		spdlog::info("onMouseHover unhovered");
		break;

	default:
	case InputState::undefined:
		spdlog::error("onMouseHover undefined");
		throw std::exception{ "undefined state have been recieved in OpenglImguiController::onMouseHover" };
	}
}

// Изменение масштаба видимой области
void OpenglImguiController::onScroll(float momentWheel) {
	if (momentWheel == 0.0f)
		return;

	if (momentWheel > 0.0f)
		spdlog::info("onScroll up, {}", momentWheel);
	else
		spdlog::info("onScroll down, {}", momentWheel);
}

void OpenglImguiController::addTriangleByCenter() {
	currentControllerState_ = ControllerState::triangleByCenter;
}

void OpenglImguiController::addTriangleByCorners() {
	currentControllerState_ = ControllerState::triangleByCorners;
}

void OpenglImguiController::addSquareByCenter() {
	currentControllerState_ = ControllerState::squareByCenter;
}

void OpenglImguiController::addSquareByCorners() {
	currentControllerState_ = ControllerState::squareByCorners;
}

void OpenglImguiController::addNgonByCenter() {
	currentControllerState_ = ControllerState::ngonByCenter;
}

void OpenglImguiController::addCircleByCenter() {
	currentControllerState_ = ControllerState::circleByCenter;
}

// на будущее, сначала надо научиться определять коллизии
void OpenglImguiController::removeFigure() {}