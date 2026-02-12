#include "CommandManager.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace view::Commands {

CommandManager::CommandManager(std::shared_ptr<spdlog::logger> logger)
    : commandStack_(logger), logger_(std::move(logger)) {
  if (logger_) {
    logger_->info("CommandManager initialized");
  }
}

void CommandManager::executeCommand(std::unique_ptr<ICommand> command) {
  if (!command) {
    throw std::runtime_error("Cannot execute null command");
  }

  try {
    // Execute the command via CommandStack
    commandStack_.execute(std::move(command));

    if (logger_) {
      logger_->debug("Command executed successfully");
    }

  } catch (const std::exception &e) {
    if (logger_) {
      logger_->error("Failed to execute command: {}", e.what());
    }
    throw;
  }
}

void CommandManager::undo() {
  if (!canUndo()) {
    throw std::runtime_error("Cannot undo: no commands available");
  }

  try {
    commandStack_.undo();

    if (logger_) {
      logger_->debug("Undo completed successfully");
    }

  } catch (const std::exception &e) {
    if (logger_) {
      logger_->error("Failed to undo command: {}", e.what());
    }
    throw;
  }
}

void CommandManager::redo() {
  if (!canRedo()) {
    throw std::runtime_error("Cannot redo: no commands available");
  }

  try {
    commandStack_.redo();

    if (logger_) {
      logger_->debug("Redo completed successfully");
    }

  } catch (const std::exception &e) {
    if (logger_) {
      logger_->error("Failed to redo command: {}", e.what());
    }
    throw;
  }
}

bool CommandManager::canUndo() const { return commandStack_.canUndo(); }

bool CommandManager::canRedo() const { return commandStack_.canRedo(); }

std::string CommandManager::getUndoDescription() const {
  return commandStack_.getUndoDescription();
}

std::string CommandManager::getRedoDescription() const {
  return commandStack_.getRedoDescription();
}

void CommandManager::clear() {
  commandStack_.clear();

  if (logger_) {
    logger_->info("Cleared command history");
  }
}

size_t CommandManager::getUndoCount() const {
  return commandStack_.getUndoStackSize();
}

size_t CommandManager::getRedoCount() const {
  return commandStack_.getRedoStackSize();
}

} // namespace view::Commands
