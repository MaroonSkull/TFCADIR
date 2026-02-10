#include "DuplicateFigureCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

DuplicateFigureCommand::DuplicateFigureCommand(model::FlatFigures &model,
                                               uint32_t sourceFigureId,
                                               glm::vec3 offset)
    : model_(model), sourceFigureId_(sourceFigureId), newFigureId_(0),
      offset_(offset), executed_(false) {}

bool DuplicateFigureCommand::execute() {
  if (executed_) {
    spdlog::warn("DuplicateFigureCommand: Already executed");
    return false;
  }

  auto sourceFigure = model_.getFigure(sourceFigureId_);
  if (!sourceFigure) {
    spdlog::error(
        "DuplicateFigureCommand: Source figure with index {} not found",
        sourceFigureId_);
    return false;
  }

  try {
    // NOTE: Figure duplication is not yet implemented
    // This requires adding a clone() method to the IFigure interface
    // which is a separate task. For now, this command is disabled.
    spdlog::error(
        "DuplicateFigureCommand: Figure duplication not yet implemented. "
        "Requires IFigure::clone() method to be added.");
    return false;
  } catch (const std::exception &e) {
    spdlog::error("DuplicateFigureCommand: Failed to duplicate figure: {}",
                  e.what());
    return false;
  }
}

bool DuplicateFigureCommand::undo() {
  if (!executed_) {
    spdlog::warn("DuplicateFigureCommand: Not executed, cannot undo");
    return false;
  }

  try {
    // Remove the duplicated figure
    bool removed = model_.removeFigure(newFigureId_);
    if (removed) {
      executed_ = false;
      spdlog::info("DuplicateFigureCommand: Removed duplicated figure {}",
                   newFigureId_);
      return true;
    } else {
      spdlog::error(
          "DuplicateFigureCommand: Failed to remove duplicated figure {}",
          newFigureId_);
      return false;
    }
  } catch (const std::exception &e) {
    spdlog::error("DuplicateFigureCommand: Failed to undo: {}", e.what());
    return false;
  }
}

std::string DuplicateFigureCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Duplicate Figure #" << sourceFigureId_ << " with offset ("
      << offset_.x << ", " << offset_.y << ", " << offset_.z << ")";
  return oss.str();
}

std::string DuplicateFigureCommand::getType() const {
  return "DuplicateFigure";
}

std::vector<uint32_t> DuplicateFigureCommand::getAffectedFigures() const {
  if (executed_ && newFigureId_ > 0) {
    return {sourceFigureId_, newFigureId_};
  }
  return {sourceFigureId_};
}

std::string DuplicateFigureCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "DuplicateFigure";
  j["sourceFigureId"] = sourceFigureId_;
  j["newFigureId"] = newFigureId_;
  j["offset"] = vec3ToJson(offset_);
  j["executed"] = executed_;

  return j.dump();
}

nlohmann::json DuplicateFigureCommand::vec3ToJson(const glm::vec3 &vec) const {
  nlohmann::json j;
  j["x"] = vec.x;
  j["y"] = vec.y;
  j["z"] = vec.z;
  return j;
}

} // namespace view
