#include "MoveFigureCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

MoveFigureCommand::MoveFigureCommand(model::FlatFigures &model,
                                     uint32_t figureId, glm::vec3 delta)
    : model_(model), figureId_(figureId), delta_(delta),
      previousPosition_(0.0f, 0.0f, 0.0f), executed_(false) {}

bool MoveFigureCommand::execute() {
  if (executed_) {
    spdlog::warn("MoveFigureCommand: Already executed");
    return false;
  }

  auto figure = model_.getFigure(figureId_);
  if (!figure) {
    spdlog::error("MoveFigureCommand: Figure with ID {} not found", figureId_);
    return false;
  }

  try {
    // Store current position for undo
    previousPosition_ = figure->getPosition();

    // Apply the delta
    glm::vec3 newPosition = previousPosition_ + delta_;
    model_.updateFigurePosition(figureId_, newPosition);

    executed_ = true;
    spdlog::info("MoveFigureCommand: Moved figure {} by ({}, {}, {})",
                 figureId_, delta_.x, delta_.y, delta_.z);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("MoveFigureCommand: Failed to move figure: {}", e.what());
    return false;
  }
}

bool MoveFigureCommand::undo() {
  if (!executed_) {
    spdlog::warn("MoveFigureCommand: Not executed, cannot undo");
    return false;
  }

  auto figure = model_.getFigure(figureId_);
  if (!figure) {
    spdlog::error("MoveFigureCommand: Figure with ID {} not found", figureId_);
    return false;
  }

  try {
    // Restore previous position
    model_.updateFigurePosition(figureId_, previousPosition_);

    executed_ = false;
    spdlog::info("MoveFigureCommand: Restored figure {} to previous position",
                 figureId_);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("MoveFigureCommand: Failed to undo: {}", e.what());
    return false;
  }
}

std::string MoveFigureCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Move Figure #" << figureId_ << " by (" << delta_.x << ", " << delta_.y
      << ", " << delta_.z << ")";
  return oss.str();
}

std::string MoveFigureCommand::getType() const { return "MoveFigure"; }

std::vector<uint32_t> MoveFigureCommand::getAffectedFigures() const {
  return {figureId_};
}

std::string MoveFigureCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "MoveFigure";
  j["figureId"] = figureId_;
  j["delta"] = vec3ToJson(delta_);
  j["previousPosition"] = vec3ToJson(previousPosition_);
  j["executed"] = executed_;

  return j.dump();
}

nlohmann::json MoveFigureCommand::vec3ToJson(const glm::vec3 &vec) const {
  nlohmann::json j;
  j["x"] = vec.x;
  j["y"] = vec.y;
  j["z"] = vec.z;
  return j;
}

} // namespace view
