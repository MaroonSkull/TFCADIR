#include "CommandStack.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace view::Commands {

CommandStack::CommandStack(std::shared_ptr<spdlog::logger> logger)
    : undoStack_(), redoStack_(), logger_(std::move(logger)) {
  if (logger_) {
    logger_->info("CommandStack initialized");
  }
}

void CommandStack::execute(std::unique_ptr<ICommand> command) {
  if (!command) {
    throw std::runtime_error("Cannot execute null command");
  }

  try {
    // Execute the command
    command->execute();

    if (logger_) {
      logger_->debug("Executed command: {}", command->getDescription());
    }

    // Add to undo stack and clear redo stack
    undoStack_.push_back(std::move(command));
    redoStack_.clear();

  } catch (const std::exception &e) {
    if (logger_) {
      logger_->error("Failed to execute command: {}", e.what());
    }
    throw;
  }
}

void CommandStack::undo() {
  if (undoStack_.empty()) {
    throw std::runtime_error("Cannot undo: undo stack is empty");
  }

  try {
    // Pop most recent command from undo stack
    auto command = std::move(undoStack_.back());
    undoStack_.pop_back();

    // Undo the command
    command->undo();

    if (logger_) {
      logger_->debug("Undone command: {}", command->getDescription());
    }

    // Add to redo stack
    redoStack_.push_back(std::move(command));

  } catch (const std::exception &e) {
    if (logger_) {
      logger_->error("Failed to undo command: {}", e.what());
    }
    throw;
  }
}

void CommandStack::redo() {
  if (redoStack_.empty()) {
    throw std::runtime_error("Cannot redo: redo stack is empty");
  }

  try {
    // Pop most recent command from redo stack
    auto command = std::move(redoStack_.back());
    redoStack_.pop_back();

    // Re-execute the command
    command->execute();

    if (logger_) {
      logger_->debug("Redone command: {}", command->getDescription());
    }

    // Add back to undo stack
    undoStack_.push_back(std::move(command));

  } catch (const std::exception &e) {
    if (logger_) {
      logger_->error("Failed to redo command: {}", e.what());
    }
    throw;
  }
}

bool CommandStack::canUndo() const { return !undoStack_.empty(); }

bool CommandStack::canRedo() const { return !redoStack_.empty(); }

std::string CommandStack::getUndoDescription() const {
  if (canUndo()) {
    return undoStack_.back()->getDescription();
  }
  return "";
}

std::string CommandStack::getRedoDescription() const {
  if (canRedo()) {
    return redoStack_.back()->getDescription();
  }
  return "";
}

void CommandStack::clear() {
  size_t undoCount = undoStack_.size();
  size_t redoCount = redoStack_.size();
  undoStack_.clear();
  redoStack_.clear();

  if (logger_) {
    logger_->info("Cleared command history: {} undo commands, {} redo commands",
                  undoCount, redoCount);
  }
}

size_t CommandStack::getUndoStackSize() const { return undoStack_.size(); }

size_t CommandStack::getRedoStackSize() const { return redoStack_.size(); }

} // namespace view::Commands
