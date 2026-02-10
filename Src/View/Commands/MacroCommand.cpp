#include "MacroCommand.hpp"
#include <algorithm>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <unordered_set>

namespace view {

MacroCommand::MacroCommand(std::vector<std::unique_ptr<ICommand>> commands)
    : commands_(std::move(commands)), executedCount_(0), executed_(false) {}

bool MacroCommand::execute() {
  if (executed_) {
    spdlog::warn("MacroCommand: Already executed");
    return false;
  }

  if (commands_.empty()) {
    spdlog::warn("MacroCommand: No sub-commands to execute");
    return false;
  }

  executedCount_ = 0;

  // Execute all sub-commands in order
  for (size_t i = 0; i < commands_.size(); ++i) {
    if (!commands_[i]->execute()) {
      spdlog::error("MacroCommand: Sub-command {} ({}) failed, rolling back", i,
                    commands_[i]->getType());

      // Rollback: undo all previously executed commands in reverse order
      for (size_t j = executedCount_; j > 0; --j) {
        size_t index = j - 1;
        if (!commands_[index]->undo()) {
          spdlog::error("MacroCommand: Failed to rollback sub-command {} ({})",
                        index, commands_[index]->getType());
        }
      }

      executedCount_ = 0;
      return false;
    }

    executedCount_++;
  }

  executed_ = true;
  spdlog::info("MacroCommand: Executed {} sub-commands successfully",
               executedCount_);
  return true;
}

bool MacroCommand::undo() {
  if (!executed_) {
    spdlog::warn("MacroCommand: Not executed, cannot undo");
    return false;
  }

  // Undo all executed commands in reverse order
  size_t undoneCount = 0;
  for (size_t i = executedCount_; i > 0; --i) {
    size_t index = i - 1;
    if (!commands_[index]->undo()) {
      spdlog::error("MacroCommand: Failed to undo sub-command {} ({})", index,
                    commands_[index]->getType());
      return false;
    }
    undoneCount++;
  }

  executed_ = false;
  executedCount_ = 0;
  spdlog::info("MacroCommand: Undone {} sub-commands successfully",
               undoneCount);
  return true;
}

std::string MacroCommand::getDescription() const {
  std::ostringstream oss;

  if (commands_.empty()) {
    oss << "Macro Command (empty)";
  } else if (commands_.size() == 1) {
    oss << "Macro Command (" << commands_[0]->getDescription() << ")";
  } else {
    oss << "Macro Command (" << commands_.size() << " operations)";
  }

  return oss.str();
}

std::string MacroCommand::getType() const { return "Macro"; }

std::vector<uint32_t> MacroCommand::getAffectedFigures() const {
  std::unordered_set<uint32_t> uniqueFigureIds;

  for (const auto &command : commands_) {
    auto figureIds = command->getAffectedFigures();
    uniqueFigureIds.insert(figureIds.begin(), figureIds.end());
  }

  return std::vector<uint32_t>(uniqueFigureIds.begin(), uniqueFigureIds.end());
}

std::string MacroCommand::serialize() const {
  nlohmann::json j;

  j["type"] = "Macro";
  j["commandCount"] = commands_.size();
  j["executedCount"] = executedCount_;
  j["executed"] = executed_;

  // Serialize sub-commands
  nlohmann::json subCommands = nlohmann::json::array();
  for (const auto &command : commands_) {
    try {
      nlohmann::json subCommandJson =
          nlohmann::json::parse(command->serialize());
      subCommands.push_back(subCommandJson);
    } catch (const std::exception &e) {
      spdlog::error("MacroCommand: Failed to serialize sub-command: {}",
                    e.what());
    }
  }
  j["subCommands"] = subCommands;

  return j.dump();
}

} // namespace view
