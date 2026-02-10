#include "ColorChangeCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

ColorChangeCommand::ColorChangeCommand(UIFSMAdapter &fsmAdapter,
                                       uint32_t figureId, glm::vec4 newColor)
    : fsmAdapter_(fsmAdapter), figureId_(figureId), newColor_(newColor),
      previousColor_(1.0f, 1.0f, 1.0f, 1.0f), executed_(false) {}

bool ColorChangeCommand::execute() {
  if (executed_) {
    spdlog::warn("ColorChangeCommand: Already executed");
    return false;
  }

  try {
    // NOTE: Figure color change is not yet implemented
    // This requires adding color properties to IFigure interface
    // For now, this command is disabled.
    spdlog::error(
        "ColorChangeCommand: Figure color change not yet implemented. "
        "Requires color properties to be added to IFigure interface.");
    return false;
  } catch (const std::exception &e) {
    spdlog::error("ColorChangeCommand: Failed to change color: {}", e.what());
    return false;
  }
}

bool ColorChangeCommand::undo() {
  if (!executed_) {
    spdlog::warn("ColorChangeCommand: Not executed, cannot undo");
    return false;
  }

  try {
    // Restore previous color
    fsmAdapter_.updateFigureProperty(figureId_, "color.r", previousColor_.r);
    fsmAdapter_.updateFigureProperty(figureId_, "color.g", previousColor_.g);
    fsmAdapter_.updateFigureProperty(figureId_, "color.b", previousColor_.b);
    fsmAdapter_.updateFigureProperty(figureId_, "color.a", previousColor_.a);

    executed_ = false;
    spdlog::info("ColorChangeCommand: Restored figure {} to previous color",
                 figureId_);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("ColorChangeCommand: Failed to undo: {}", e.what());
    return false;
  }
}

std::string ColorChangeCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Change Color of Figure #" << figureId_ << " to (" << newColor_.r
      << ", " << newColor_.g << ", " << newColor_.b << ", " << newColor_.a
      << ")";
  return oss.str();
}

std::string ColorChangeCommand::getType() const { return "ColorChange"; }

std::vector<uint32_t> ColorChangeCommand::getAffectedFigures() const {
  return {figureId_};
}

std::string ColorChangeCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "ColorChange";
  j["figureId"] = figureId_;
  j["newColor"] = vec4ToJson(newColor_);
  j["previousColor"] = vec4ToJson(previousColor_);
  j["executed"] = executed_;

  return j.dump();
}

nlohmann::json ColorChangeCommand::vec4ToJson(const glm::vec4 &vec) const {
  nlohmann::json j;
  j["r"] = vec.r;
  j["g"] = vec.g;
  j["b"] = vec.b;
  j["a"] = vec.a;
  return j;
}

} // namespace view
