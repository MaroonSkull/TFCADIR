#include "ExtendedCommandManager.hpp"

#include "CommandFactory.hpp"
#include "CommandHistory.hpp"
#include "Model/Model.hpp"
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace view {

ExtendedCommandManager::ExtendedCommandManager(CommandHistory &commandHistory,
                                               model::FlatFigures &model)
    : commandHistory_(commandHistory), model_(model) {
  spdlog::info(
      "[ExtendedCommandManager] Initialized (delegates to CommandHistory)");
}

// ==========================================================================
// Phase 4 Typed Command API
// ==========================================================================

void ExtendedCommandManager::executeCommand(std::unique_ptr<ICommand> command) {
  spdlog::debug(
      "[ExtendedCommandManager] executeCommand (Phase 4 typed): type={}",
      command->getType());

  // Execute the command
  if (command->execute()) {
    spdlog::info("[ExtendedCommandManager] Command executed successfully: {}",
                 command->getDescription());

    // Add to command history
    commandHistory_.addCommand(std::move(command));
    notifyCommandExecuted();
  } else {
    spdlog::error("[ExtendedCommandManager] Command execution failed: {}",
                  command->getType());
  }
}

void ExtendedCommandManager::executeMacro(std::unique_ptr<ICommand> macro) {
  spdlog::debug("[ExtendedCommandManager] executeMacro (Phase 4)");

  // Execute the macro
  if (macro->execute()) {
    spdlog::info("[ExtendedCommandManager] Macro executed successfully: {}",
                 macro->getDescription());

    // Add to command history
    commandHistory_.addCommand(std::move(macro));
    notifyCommandExecuted();
  } else {
    spdlog::error("[ExtendedCommandManager] Macro execution failed");
  }
}

bool ExtendedCommandManager::undo() {
  spdlog::debug("[ExtendedCommandManager] undo()");

  if (commandHistory_.undo()) {
    notifyCommandUndone();
    return true;
  }
  return false;
}

bool ExtendedCommandManager::redo() {
  spdlog::debug("[ExtendedCommandManager] redo()");

  if (commandHistory_.redo()) {
    notifyCommandRedone();
    return true;
  }
  return false;
}

// ==========================================================================
// Query Methods for UI (Delegate to CommandHistory)
// ==========================================================================

std::vector<CommandInfo> ExtendedCommandManager::getCommandHistory() const {
  spdlog::debug("[ExtendedCommandManager] getCommandHistory()");

  std::vector<CommandInfo> result;
  size_t historySize = commandHistory_.getHistorySize();

  for (size_t i = 0; i < historySize; ++i) {
    const ICommand *cmd = commandHistory_.getCommandAt(i);
    if (cmd) {
      CommandInfo info = commandToInfo(cmd, i);
      result.push_back(info);
    }
  }

  return result;
}

CommandInfo ExtendedCommandManager::getCurrentCommand() const {
  spdlog::debug("[ExtendedCommandManager] getCurrentCommand()");

  size_t currentIndex = commandHistory_.getCurrentIndex();
  if (currentIndex == 0) {
    return CommandInfo{};
  }

  const ICommand *cmd = commandHistory_.getCommandAt(currentIndex - 1);
  if (cmd) {
    return commandToInfo(cmd, currentIndex - 1);
  }

  return CommandInfo{};
}

CommandInfo ExtendedCommandManager::getUndoCommand() const {
  spdlog::debug("[ExtendedCommandManager] getUndoCommand()");

  if (!commandHistory_.canUndo()) {
    return CommandInfo{};
  }

  size_t currentIndex = commandHistory_.getCurrentIndex();
  const ICommand *cmd = commandHistory_.getCommandAt(currentIndex - 1);
  if (cmd) {
    return commandToInfo(cmd, currentIndex - 1);
  }

  return CommandInfo{};
}

CommandInfo ExtendedCommandManager::getRedoCommand() const {
  spdlog::debug("[ExtendedCommandManager] getRedoCommand()");

  if (!commandHistory_.canRedo()) {
    return CommandInfo{};
  }

  size_t currentIndex = commandHistory_.getCurrentIndex();
  const ICommand *cmd = commandHistory_.getCommandAt(currentIndex);
  if (cmd) {
    return commandToInfo(cmd, currentIndex);
  }

  return CommandInfo{};
}

size_t ExtendedCommandManager::getHistorySize() const {
  spdlog::debug("[ExtendedCommandManager] getHistorySize()");
  return commandHistory_.getHistorySize();
}

size_t ExtendedCommandManager::getCurrentIndex() const {
  spdlog::debug("[ExtendedCommandManager] getCurrentIndex()");
  return commandHistory_.getCurrentIndex();
}

// ==========================================================================
// Phase 2: Undo/Redo Query Methods (Backward Compatibility)
// These methods delegate to CommandHistory for state access
// ==========================================================================

bool ExtendedCommandManager::canUndo() const {
  /// Delegate to CommandHistory to check if undo is available
  /// Following the stateless coordinator pattern (no local state)
  return commandHistory_.canUndo();
}

bool ExtendedCommandManager::canRedo() const {
  /// Delegate to CommandHistory to check if redo is available
  /// Following the stateless coordinator pattern (no local state)
  return commandHistory_.canRedo();
}

std::string ExtendedCommandManager::getUndoDescription() const {
  /// Delegate to CommandHistory to get the undo command description
  /// Following the stateless coordinator pattern (no local state)
  return commandHistory_.getUndoDescription();
}

std::string ExtendedCommandManager::getRedoDescription() const {
  /// Delegate to CommandHistory to get the redo command description
  /// Following the stateless coordinator pattern (no local state)
  return commandHistory_.getRedoDescription();
}

// ==========================================================================
// Persistence Methods
// ==========================================================================

void ExtendedCommandManager::saveHistory(const std::string &filepath) {
  spdlog::info("[ExtendedCommandManager] saveHistory: {}", filepath);

  try {
    nlohmann::json root;
    root["version"] = "1.0";
    root["currentIndex"] = commandHistory_.getCurrentIndex();

    nlohmann::json commandsArray = nlohmann::json::array();
    size_t historySize = commandHistory_.getHistorySize();

    for (size_t i = 0; i < historySize; ++i) {
      const ICommand *cmd = commandHistory_.getCommandAt(i);
      if (cmd) {
        try {
          nlohmann::json cmdJson = nlohmann::json::parse(cmd->serialize());
          commandsArray.push_back(cmdJson);
        } catch (const nlohmann::json::parse_error &e) {
          spdlog::warn(
              "[ExtendedCommandManager] Failed to parse command JSON: {}",
              e.what());
        }
      }
    }

    root["commands"] = commandsArray;

    std::ofstream file(filepath);
    if (!file.is_open()) {
      spdlog::error(
          "[ExtendedCommandManager] Failed to open file for writing: {}",
          filepath);
      return;
    }

    file << root.dump(2);
    file.close();

    spdlog::info("[ExtendedCommandManager] Saved {} commands to {}",
                 historySize, filepath);
  } catch (const std::exception &e) {
    spdlog::error("[ExtendedCommandManager] Failed to save history: {}",
                  e.what());
  }
}

void ExtendedCommandManager::loadHistory(
    const std::shared_ptr<CommandFactory> &factory,
    const std::string &filepath) {
  spdlog::info("[ExtendedCommandManager] loadHistory: {}", filepath);

  if (!factory) {
    spdlog::error("[ExtendedCommandManager] CommandFactory is null");
    return;
  }

  try {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      spdlog::error(
          "[ExtendedCommandManager] Failed to open file for reading: {}",
          filepath);
      return;
    }

    nlohmann::json root;
    file >> root;
    file.close();

    // Clear existing history
    commandHistory_.clear();

    // Load commands
    if (root.contains("commands") && root["commands"].is_array()) {
      for (const auto &cmdJson : root["commands"]) {
        std::string jsonStr = cmdJson.dump();
        auto command = factory->deserialize(jsonStr);
        if (command) {
          // Add to history without executing (just store)
          commandHistory_.addCommand(std::move(command));
        } else {
          spdlog::warn(
              "[ExtendedCommandManager] Failed to deserialize command");
        }
      }
    }

    // Restore current index
    if (root.contains("currentIndex")) {
      size_t targetIndex = root["currentIndex"].get<size_t>();
      // The addCommand calls have already set currentIndex to the end
      // We need to undo back to the target index
      while (commandHistory_.getCurrentIndex() > targetIndex &&
             commandHistory_.canUndo()) {
        commandHistory_.undo();
      }
    }

    spdlog::info("[ExtendedCommandManager] Loaded {} commands from {}",
                 commandHistory_.getHistorySize(), filepath);
  } catch (const std::exception &e) {
    spdlog::error("[ExtendedCommandManager] Failed to load history: {}",
                  e.what());
  }
}

// ==========================================================================
// Private Notification Methods
// ==========================================================================

void ExtendedCommandManager::notifyCommandExecuted() {
  spdlog::debug("[ExtendedCommandManager] notifyCommandExecuted");
  // TODO: Implement notification callbacks when UI integration is ready
}

void ExtendedCommandManager::notifyCommandUndone() {
  spdlog::debug("[ExtendedCommandManager] notifyCommandUndone");
  // TODO: Implement notification callbacks when UI integration is ready
}

void ExtendedCommandManager::notifyCommandRedone() {
  spdlog::debug("[ExtendedCommandManager] notifyCommandRedone");
  // TODO: Implement notification callbacks when UI integration is ready
}

void ExtendedCommandManager::notifyHistoryCleared() {
  spdlog::debug("[ExtendedCommandManager] notifyHistoryCleared");
  // TODO: Implement notification callbacks when UI integration is ready
}

// ==========================================================================
// Private Helper Methods
// ==========================================================================

CommandInfo ExtendedCommandManager::commandToInfo(const ICommand *command,
                                                  size_t index) const {
  CommandInfo info;
  info.index = index;
  info.type = command->getType();
  info.description = command->getDescription();
  info.affectedFigures = command->getAffectedFigures();

  // Generate timestamp (placeholder - could be stored in command)
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  info.timestamp = std::ctime(&time);
  // Remove trailing newline
  if (!info.timestamp.empty() && info.timestamp.back() == '\n') {
    info.timestamp.pop_back();
  }

  // Check if this is a macro command
  // Note: This is a simple check - could be improved with a dynamic_cast
  info.isMacro = (info.type == "MacroCommand");

  return info;
}

} // namespace view
