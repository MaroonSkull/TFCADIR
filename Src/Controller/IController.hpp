#pragma once

#include <glm/vec2.hpp>
#include <FSM.hpp>
// Uncomment this line when i have the IModel interface implemented :D
// #include <Model/IModel.hpp>

namespace controller {

namespace state {

/**
 * @brief Binary state of any button.
 */
enum class Button {
	down,
	released
};

/**
 * @brief Is cursor over the workspace?
 */
enum class Workspace {
	unhovered,
	hovered
};

} // namespace state

class IController {
	public:
		fsm::state::Machine fsm_;

		virtual void updateLeftMouseButtonState(state::Button) = 0;
		virtual void updateWheelMouseButtonState(state::Button) = 0;
		virtual void updateRightMouseButtonState(state::Button) = 0;
		virtual void updateWorkspaceHoverState(state::Workspace) = 0;

		/**
		 * @brief Create OnMouseMove event in model FSM, if position changed. 
		 * @param screenspaceMousePosition - new position of mouse in screenspace.
		 */
		virtual void updateScreenspaceMousePosition(glm::vec2) = 0;
		virtual void updateScroll(float) = 0;

		//
		virtual void addTriangleByCenter() = 0;
		virtual void addTriangleByCorners() = 0;
		virtual void addSquareByCenter() = 0;
		virtual void addSquareByCorners() = 0;
		virtual void addNgonByCenter() = 0;
		virtual void addCircleByCenter() = 0;
		virtual void addLine() = 0;
		virtual void removeFigure() = 0;        
};

} // namespace controller