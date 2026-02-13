#include "MacroCommand.hpp"
#include <algorithm>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace view {

MacroCommand::MacroCommand(
    std::vector<std::unique_ptr<Commands::ICommand>> commands)
    : commands_(std::move(commands)), executedCount_(0), executed_(false) {}

void MacroCommand::execute() {
  if (executed_) {
    throw std::runtime_error("MacroCommand: Already executed");
  }

  if (commands_.empty()) {
    spdlog::warn("MacroCommand: No sub-commands to execute");
    return;
  }

  executedCount_ = 0;

  // Execute all sub-commands in order
  for (size_t i = 0; i < commands_.size(); ++i) {
    try {
      commands_[i]->execute();
    } catch (const std::exception &e) {
      spdlog::error(
          "MacroCommand: Sub-command {} ({}) failed, rolling back: {}", i,
          commands_[i]->getType(), e.what());

      // Rollback: undo all previously executed commands in reverse order
      for (size_t j = executedCount_; j > 0; --j) {
        size_t index = j - 1;
        try {
          commands_[index]->undo();
        } catch (const std::exception &rollbackError) {
          spdlog::error(
              "MacroCommand: Failed to rollback sub-command {} ({}): {}", index,
              commands_[index]->getType(), rollbackError.what());
        }
      }

      executedCount_ = 0;
      throw std::runtime_error("MacroCommand: Sub-command execution failed");
    }

    executedCount_++;
  }

  executed_ = true;
  spdlog::info("MacroCommand: Executed {} sub-commands successfully",
               executedCount_);
}

void MacroCommand::undo() {
  if (!executed_) {
    throw std::runtime_error("MacroCommand: Not executed, cannot undo");
  }

  // Undo all executed commands in reverse order
  size_t undoneCount = 0;
  for (size_t i = executedCount_; i > 0; --i) {
    size_t index = i - 1;
    try {
      commands_[index]->undo();
    } catch (const std::exception &e) {
      spdlog::error("MacroCommand: Failed to undo sub-command {} ({}): {}",
                    index, commands_[index]->getType(), e.what());
      throw std::runtime_error("MacroCommand: Undo operation failed");
    }
    undoneCount++;
  }

  executed_ = false;
  executedCount_ = 0;
  spdlog::info("MacroCommand: Undone {} sub-commands successfully",
               undoneCount);
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
