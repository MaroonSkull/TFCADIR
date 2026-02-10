#include "UngroupFiguresCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

UngroupFiguresCommand::UngroupFiguresCommand(UIFSMAdapter &fsmAdapter,
                                             uint32_t groupId)
    : fsmAdapter_(fsmAdapter), groupId_(groupId), executed_(false) {}

bool UngroupFiguresCommand::execute() {
  if (executed_) {
    spdlog::warn("UngroupFiguresCommand: Already executed");
    return false;
  }

  try {
    // NOTE: Figure ungrouping is not yet implemented
    // This requires adding UIFSMAdapter methods:
    // - ungroupFigures(groupId, releasedFigureIds)
    // - groupFigures(figureIds, releasedFigureIds)
    // For now, this command is disabled.
    spdlog::error(
        "UngroupFiguresCommand: Figure ungrouping not yet implemented. "
        "Requires UIFSMAdapter extensions for grouping functionality.");
    return false;
  } catch (const std::exception &e) {
    spdlog::error("UngroupFiguresCommand: Failed to ungroup: {}", e.what());
    return false;
  }
}

bool UngroupFiguresCommand::undo() {
  if (!executed_) {
    spdlog::warn("UngroupFiguresCommand: Not executed, cannot undo");
    return false;
  }

  try {
    // Re-group the figures
    uint32_t newGroupId = fsmAdapter_.groupFigures(releasedFigureIds_);

    if (newGroupId == 0) {
      spdlog::error("UngroupFiguresCommand: Failed to re-group figures");
      return false;
    }

    executed_ = false;
    spdlog::info("UngroupFiguresCommand: Re-grouped {} figures into group {}",
                 releasedFigureIds_.size(), newGroupId);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("UngroupFiguresCommand: Failed to undo: {}", e.what());
    return false;
  }
}

std::string UngroupFiguresCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Ungroup Group #" << groupId_;
  return oss.str();
}

std::string UngroupFiguresCommand::getType() const { return "UngroupFigures"; }

std::vector<uint32_t> UngroupFiguresCommand::getAffectedFigures() const {
  return releasedFigureIds_;
}

std::string UngroupFiguresCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "UngroupFigures";
  j["groupId"] = groupId_;
  j["releasedFigureIds"] = releasedFigureIds_;
  j["executed"] = executed_;

  return j.dump();
}

} // namespace view
