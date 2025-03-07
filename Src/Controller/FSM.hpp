#pragma once

#include <boost/mpl/list.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/transition.hpp>

#include <glm/glm.hpp>

#include <memory>
#include <spdlog/spdlog.h>

#include <Model/FlatFigure.hpp>

namespace fsm {
namespace sc = boost::statechart;

namespace events {

	/**
	* @brief Event triggered when the mouse moves.
	*
	* This event is used to notify the state machine that the mouse has moved,
	* and provides the new position of the mouse cursor.
	* @todo add NDC coords and screenspace coords and coords of other types maybe.
	*/
	struct OnMouseMove : sc::event<OnMouseMove> {
		glm::vec2 screenspacePosition_; // screenspace coords
		glm::vec2 ndcPosition_; // NDC coords
		OnMouseMove(glm::vec2 ssp) : screenspacePosition_(ssp) {}
	};

	struct OnAddLine : sc::event<OnAddLine> {};

	struct OnAddTriangleByCenter : sc::event<OnAddTriangleByCenter> {};

	struct OnAddTriangleByCorners : sc::event<OnAddTriangleByCorners> {};

	struct OnAddSquareByCenter : sc::event<OnAddSquareByCenter> {};

	struct OnAddSquareByCorners : sc::event<OnAddSquareByCorners> {};

	struct OnAddNgonByCenter : sc::event<OnAddNgonByCenter> {};
	
	struct OnAddCircleByCenter : sc::event<OnAddCircleByCenter> {};

} // namespace events

namespace state {

	/// Forward declaration for initial state of `Machine` statemachine.
	struct Idle;

	/**
	* @brief Finite State Machine definition.
	* @details This class defines the main state machine with an initial state
	* of `Idle`. It uses Boost.Statechart to manage states and transitions.
	*/
	struct Machine : sc::state_machine<Machine, Idle> {
			typedef boost::mpl::list<
				sc::transition<events::OnMouseMove, Idle, >
			> reactions;
	};

	/**
	* @brief Initial state of the Machine.
	* @details The `Idle` state is the starting point for the Machine.
	* It logs a message when entered to indicate that the system is in an idle state.
	*/
	struct Idle : sc::simple_state<Idle, Machine> {
		Idle() {
			spdlog::debug("In state => Idle");
		}

		void OnMouseMove(const events::OnMouseMove& event) {
			// Используем координаты мыши для вычислений
			spdlog::debug("Mouse moved to: ({}, {})", event.screenspacePosition_.x, event.screenspacePosition_.y);
			// Здесь можно выполнить необходимые действия с координатами мыши
		}

		typedef boost::mpl::list<
			sc::transition<events::OnMouseMove, Idle>
		> reactions;
	};


	/// Forward declaration for init state of `DrawingProcessing` metastate
	struct MoveFirstPoint;

	/**
	* @brief State for processing drawing commands.
	* @details The `DrawingProcessing` state is used to handle the logic
	* related to processing drawing commands. It logs a message when entered.
	*/
	struct DrawingProcessing : sc::simple_state<DrawingProcessing, Machine, MoveFirstPoint> {
		DrawingProcessing() {
			spdlog::debug("In metastate => DrawingProcessing");
		}

		// Action to add triangle by center
		void onAddTriangleByCenter(const events::OnAddTriangleByCenter&) {
			// Assuming you have a method in FlatFigures to add a triangle
			sp_model_->addTriangleByCenter(); // Call the appropriate method
		}

		typedef boost::mpl::list<
			sc::transition<events::OnAddTriangleByCenter, MoveFirstPoint, DrawingProcessing, &DrawingProcessing::onAddTriangleByCenter>
		> reactions;
	};

	/**
	* @brief Inner state for moving the first point in a drawing.
	* @details The `MoveFirstPoint` state is an inner state within the `DrawingProcessing`
	* metastate. It logs a message when entered to indicate that the system is in the process
	* of moving the first point during drawing operations.
	* @note This state is part of the `DrawingProcessing` metastate.
	* @see DrawingProcessing
	*/
	struct MoveFirstPoint : sc::simple_state<MoveFirstPoint, DrawingProcessing> {
		MoveFirstPoint() {
			spdlog::debug("In inner state of metastate DrawingProcessing => MoveFirstPoint");
		}
	};

	/**
	* @brief Inner state for moving the second point in a drawing.
	* @details The `MoveSecondPoint` state is an inner state within the `DrawingProcessing`
	* metastate. It logs a message when entered to indicate that the system is in the process
	* of moving the second point during drawing operations.
	* @note This state is part of the `DrawingProcessing` metastate.
	* @see DrawingProcessing
	*/
	struct MoveSecondPoint : sc::simple_state<MoveSecondPoint, DrawingProcessing> {
		MoveSecondPoint() {
			spdlog::debug("In inner state of metastate DrawingProcessing => MoveSecondPoint");
		}
	};


} // namespace state

} // namespace fsm