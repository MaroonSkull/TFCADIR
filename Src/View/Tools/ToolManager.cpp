/**
 * @file ToolManager.cpp
 * @brief Implementation of ToolManager for Phase 2 drawing tools
 *
 * This file implements the ToolManager class which provides a stateless
 * interface for activating, deactivating, and querying the current tool state.
 * All tool state is stored in the FSM as the single source of truth.
 *
 * Tool activation workflow:
 * 1. User clicks tool button in UI
 * 2. ToolManager::activateTool() triggered
 * 3. FSM event OnActivateTool* triggered
 * 4. FSM transitions to DrawingProcessing state
 * 5. Actor system handles drawing operations
 * 6. OnFigureComplete event triggered when drawing done
 * 7. FSM transitions back to Idle
 */

#include "ToolManager.hpp"
#include "UIFSMAdapter.hpp"
#include <stdexcept>

namespace view {

ToolManager::ToolManager(UIFSMAdapter &adapter) : adapter_(adapter) {}

void ToolManager::activateTool(const std::string &toolId) {
  /// Map tool ID to FSM event and trigger the activation
  std::string eventName = toolIdToEvent(toolId);
  adapter_.activateTool(toolId);
}

void ToolManager::deactivateTool() {
  /// Trigger the OnDeactivateTool FSM event to clear the active tool state
  adapter_.deactivateTool();
}

std::string ToolManager::getActiveTool() const {
  /// Query the FSM for the current active tool
  return adapter_.getActiveTool();
}

std::vector<glm::vec3> ToolManager::getCollectedPoints() const {
  /// Query the FSM for points collected during the current drawing operation
  return adapter_.getCollectedPoints();
}

std::string ToolManager::toolIdToEvent(const std::string &toolId) const {
  /// Map tool IDs to their corresponding FSM activation events
  if (toolId == "Line3D") {
    return "OnActivateLine3D";
  }
  if (toolId == "Circle3D") {
    return "OnActivateCircle3D";
  }
  if (toolId == "Arc3D") {
    return "OnActivateArc3D";
  }
  if (toolId == "Rectangle3D") {
    return "OnActivateRectangle3D";
  }
  if (toolId == "Polygon3D") {
    return "OnActivatePolygon3D";
  }
  if (toolId == "NGon3D") {
    return "OnActivateNGon3D";
  }
  if (toolId == "LineInSketch") {
    return "OnActivateLineInSketch";
  }
  if (toolId == "CircleInSketch") {
    return "OnActivateCircleInSketch";
  }
  throw std::runtime_error("Unknown tool ID: " + toolId);
}

} // namespace view
