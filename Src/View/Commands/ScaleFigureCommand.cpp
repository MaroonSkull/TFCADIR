#include "ScaleFigureCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

ScaleFigureCommand::ScaleFigureCommand(UIFSMAdapter &fsmAdapter,
                                       uint32_t figureId, glm::vec3 scaleFactor)
    : fsmAdapter_(fsmAdapter), figureId_(figureId), scaleFactor_(scaleFactor),
      previousScale_(1.0f, 1.0f, 1.0f), executed_(false) {}

void ScaleFigureCommand::execute() {
  if (executed_) {
    spdlog::warn("ScaleFigureCommand: Already executed");
    return;
  }

  try {
    // NOTE: Figure scaling is not yet implemented
    // This requires adding scale properties to IFigure interface
    // For now, this command is disabled.
    spdlog::error(
        "ScaleFigureCommand: Figure scaling not yet implemented. "
        "Requires scale properties to be added to IFigure interface.");
    return;
  } catch (const std::exception &e) {
    spdlog::error("ScaleFigureCommand: Failed to scale figure: {}", e.what());
    return;
  }
}

void ScaleFigureCommand::undo() {
  if (!executed_) {
    spdlog::warn("ScaleFigureCommand: Not executed, cannot undo");
    return;
  }

  try {
    // Restore previous scale
    fsmAdapter_.updateFigureProperty(figureId_, "scale.x", previousScale_.x);
    fsmAdapter_.updateFigureProperty(figureId_, "scale.y", previousScale_.y);
    fsmAdapter_.updateFigureProperty(figureId_, "scale.z", previousScale_.z);

    executed_ = false;
    spdlog::info("ScaleFigureCommand: Restored figure {} to previous scale",
                 figureId_);
    return;
  } catch (const std::exception &e) {
    spdlog::error("ScaleFigureCommand: Failed to undo: {}", e.what());
    return;
  }
}

std::string ScaleFigureCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Scale Figure #" << figureId_ << " by (" << scaleFactor_.x << ", "
      << scaleFactor_.y << ", " << scaleFactor_.z << ")";
  return oss.str();
}

std::string ScaleFigureCommand::getType() const { return "ScaleFigure"; }

std::vector<uint32_t> ScaleFigureCommand::getAffectedFigures() const {
  return {figureId_};
}

std::string ScaleFigureCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "ScaleFigure";
  j["figureId"] = figureId_;
  j["scaleFactor"] = vec3ToJson(scaleFactor_);
  j["previousScale"] = vec3ToJson(previousScale_);
  j["executed"] = executed_;

  return j.dump();
}

nlohmann::json ScaleFigureCommand::vec3ToJson(const glm::vec3 &vec) const {
  nlohmann::json j;
  j["x"] = vec.x;
  j["y"] = vec.y;
  j["z"] = vec.z;
  return j;
}

} // namespace view
