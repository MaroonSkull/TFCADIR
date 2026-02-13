#include "DeleteFiguresCommand.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

namespace view {

DeleteFiguresCommand::DeleteFiguresCommand(model::FlatFigures &model,
                                           uint32_t figureId)
    : model_(model), figureId_(figureId), deletedFigure_(nullptr),
      executed_(false) {}

void DeleteFiguresCommand::execute() {
  if (executed_) {
    spdlog::warn("DeleteFiguresCommand: Already executed");
    return;
  }

  try {
    // Get the figure before deleting (for undo)
    // NOTE: figureId_ is the vector index in FlatFigures
    deletedFigure_ = model_.getFigure(figureId_);

    if (!deletedFigure_) {
      spdlog::error("DeleteFiguresCommand: Figure at vector index {} not found",
                    figureId_);
      return;
    }

    // Remove the figure from the model using vector index
    bool removed = model_.removeFigure(figureId_);
    if (removed) {
      executed_ = true;
      spdlog::info("DeleteFiguresCommand: Deleted figure at vector index {} "
                   "(unique ID: {})",
                   figureId_, deletedFigure_->getId());
      return;
    } else {
      spdlog::error(
          "DeleteFiguresCommand: Failed to delete figure at vector index {}",
          figureId_);
      deletedFigure_ = nullptr;
      return;
    }
  } catch (const std::exception &e) {
    spdlog::error("DeleteFiguresCommand: Failed to delete figure: {}",
                  e.what());
    deletedFigure_ = nullptr;
    return;
  }
}

void DeleteFiguresCommand::undo() {
  if (!executed_) {
    spdlog::warn("DeleteFiguresCommand: Not executed, cannot undo");
    return;
  }

  if (!deletedFigure_) {
    spdlog::error("DeleteFiguresCommand: No deleted figure to restore");
    return;
  }

  try {
    // Add the figure back to the model
    // NOTE: This adds to the end of the vector, not at the original index
    model_.addFigurePtr(deletedFigure_);

    executed_ = false;
    spdlog::info("DeleteFiguresCommand: Restored figure (unique ID: {})",
                 deletedFigure_->getId());
    return;
  } catch (const std::exception &e) {
    spdlog::error("DeleteFiguresCommand: Failed to undo: {}", e.what());
    return;
  }
}

std::string DeleteFiguresCommand::getDescription() const {
  std::ostringstream oss;
  oss << "Delete Figure #" << figureId_;
  return oss.str();
}

std::string DeleteFiguresCommand::getType() const { return "DeleteFigure"; }

std::vector<uint32_t> DeleteFiguresCommand::getAffectedFigures() const {
  return {figureId_};
}

std::string DeleteFiguresCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "DeleteFigure";
  j["figureId"] = figureId_;
  j["executed"] = executed_;

  return j.dump();
}

} // namespace view
