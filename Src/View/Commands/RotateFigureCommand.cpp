#include "RotateFigureCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

RotateFigureCommand::RotateFigureCommand(UIFSMAdapter &fsmAdapter,
                                         uint32_t figureId, float angleDegrees)
    : fsmAdapter_(fsmAdapter), figureId_(figureId), angleDegrees_(angleDegrees),
      previousAngle_(0.0f), executed_(false) {}

void RotateFigureCommand::execute() {
  if (executed_) {
    spdlog::warn("RotateFigureCommand: Already executed");
    return;
  }

  try {
    // NOTE: Figure rotation is not yet implemented
    // This requires adding rotation properties to IFigure interface
    // and implementing rotation matrix support in figures
    // For now, this command is disabled.
    spdlog::error(
        "RotateFigureCommand: Figure rotation not yet implemented. "
        "Requires rotation properties to be added to IFigure interface.");
    return;
  } catch (const std::exception &e) {
    spdlog::error("RotateFigureCommand: Failed to rotate figure: {}", e.what());
    return;
  }
}

void RotateFigureCommand::undo() {
  if (!executed_) {
    spdlog::warn("RotateFigureCommand: Not executed, cannot undo");
    return;
  }

  try {
    // Restore previous angle
    fsmAdapter_.updateFigureProperty(figureId_, "rotation.angle",
                                     previousAngle_);

    executed_ = false;
    spdlog::info("RotateFigureCommand: Restored figure {} to previous angle",
                 figureId_);
    return;
  } catch (const std::exception &e) {
    spdlog::error("RotateFigureCommand: Failed to undo: {}", e.what());
    return;
  }
}

std::string RotateFigureCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Rotate Figure #" << figureId_ << " by " << angleDegrees_
      << " degrees";
  return oss.str();
}

std::string RotateFigureCommand::getType() const { return "RotateFigure"; }

std::vector<uint32_t> RotateFigureCommand::getAffectedFigures() const {
  return {figureId_};
}

std::string RotateFigureCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "RotateFigure";
  j["figureId"] = figureId_;
  j["angleDegrees"] = angleDegrees_;
  j["previousAngle"] = previousAngle_;
  j["executed"] = executed_;

  return j.dump();
}

} // namespace view
