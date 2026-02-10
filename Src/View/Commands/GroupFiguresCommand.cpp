#include "GroupFiguresCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

GroupFiguresCommand::GroupFiguresCommand(UIFSMAdapter &fsmAdapter,
                                         const std::vector<uint32_t> &figureIds)
    : fsmAdapter_(fsmAdapter), figureIds_(figureIds), groupId_(0),
      executed_(false) {}

bool GroupFiguresCommand::execute() {
  if (executed_) {
    spdlog::warn("GroupFiguresCommand: Already executed");
    return false;
  }

  if (figureIds_.empty()) {
    spdlog::error("GroupFiguresCommand: No figures to group");
    return false;
  }

  try {
    // NOTE: Figure grouping is not yet implemented
    // This requires adding UIFSMAdapter methods:
    // - groupFigures(figureIds, releasedFigureIds)
    // - ungroupFigures(groupId, releasedFigureIds)
    // For now, this command is disabled.
    spdlog::error(
        "GroupFiguresCommand: Figure grouping not yet implemented. "
        "Requires UIFSMAdapter extensions for grouping functionality.");
    return false;
  } catch (const std::exception &e) {
    spdlog::error("GroupFiguresCommand: Failed to group figures: {}", e.what());
    return false;
  }
}

bool GroupFiguresCommand::undo() {
  if (!executed_) {
    spdlog::warn("GroupFiguresCommand: Not executed, cannot undo");
    return false;
  }

  try {
    // Ungroup the figures
    std::vector<uint32_t> releasedFigures =
        fsmAdapter_.ungroupFigures(groupId_);

    if (releasedFigures.empty()) {
      spdlog::error("GroupFiguresCommand: Failed to ungroup group {}",
                    groupId_);
      return false;
    }

    executed_ = false;
    spdlog::info("GroupFiguresCommand: Ungrouped group {}", groupId_);
    return true;
  } catch (const std::exception &e) {
    spdlog::error("GroupFiguresCommand: Failed to undo: {}", e.what());
    return false;
  }
}

std::string GroupFiguresCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Group " << figureIds_.size() << " Figures";
  return oss.str();
}

std::string GroupFiguresCommand::getType() const { return "GroupFigures"; }

std::vector<uint32_t> GroupFiguresCommand::getAffectedFigures() const {
  return figureIds_;
}

std::string GroupFiguresCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "GroupFigures";
  j["figureIds"] = figureIds_;
  j["groupId"] = groupId_;
  j["releasedFigureIds"] = releasedFigureIds_;
  j["executed"] = executed_;

  return j.dump();
}

} // namespace view
