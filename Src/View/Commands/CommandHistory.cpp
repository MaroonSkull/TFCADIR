#include "CommandHistory.hpp"
#include <spdlog/spdlog.h>

namespace view {

void CommandHistory::addCommand(std::unique_ptr<ICommand> command) {
  spdlog::debug("[CommandHistory] addCommand: type={}, description={}",
                command->getType(), command->getDescription());

  // Remove all commands after current index (clear redo chain)
  if (currentIndex_ < commands_.size()) {
    commands_.erase(commands_.begin() + currentIndex_, commands_.end());
  }

  // Add the new command
  commands_.push_back(std::move(command));
  currentIndex_ = commands_.size();

  // Enforce maximum history size
  if (commands_.size() > MAX_HISTORY_SIZE) {
    size_t excess = commands_.size() - MAX_HISTORY_SIZE;
    commands_.erase(commands_.begin(), commands_.begin() + excess);
    currentIndex_ -= excess;
    spdlog::warn("[CommandHistory] History exceeded maximum size, removed {} "
                 "oldest commands",
                 excess);
  }

  spdlog::debug(
      "[CommandHistory] Command added, history size={}, current index={}",
      commands_.size(), currentIndex_);
}

bool CommandHistory::canUndo() const { return currentIndex_ > 0; }

bool CommandHistory::canRedo() const {
  return currentIndex_ < commands_.size();
}

bool CommandHistory::undo() {
  if (!canUndo()) {
    spdlog::warn("[CommandHistory] undo() called but no command to undo");
    return false;
  }

  // Get the command to undo (at current index - 1)
  size_t undoIndex = currentIndex_ - 1;
  ICommand *command = commands_[undoIndex].get();

  spdlog::debug("[CommandHistory] undo(): index={}, type={}, description={}",
                undoIndex, command->getType(), command->getDescription());

  if (command->undo()) {
    currentIndex_ = undoIndex;
    spdlog::info("[CommandHistory] Undo successful, new current index={}",
                 currentIndex_);
    return true;
  } else {
    spdlog::error("[CommandHistory] Undo failed for command at index {}",
                  undoIndex);
    return false;
  }
}

bool CommandHistory::redo() {
  if (!canRedo()) {
    spdlog::warn("[CommandHistory] redo() called but no command to redo");
    return false;
  }

  // Get the command to redo (at current index)
  ICommand *command = commands_[currentIndex_].get();

  spdlog::debug("[CommandHistory] redo(): index={}, type={}, description={}",
                currentIndex_, command->getType(), command->getDescription());

  // For redo, we execute the command again
  if (command->execute()) {
    currentIndex_++;
    spdlog::info("[CommandHistory] Redo successful, new current index={}",
                 currentIndex_);
    return true;
  } else {
    spdlog::error("[CommandHistory] Redo failed for command at index {}",
                  currentIndex_);
    return false;
  }
}

std::string CommandHistory::getUndoDescription() const {
  if (!canUndo()) {
    return "";
  }

  size_t undoIndex = currentIndex_ - 1;
  return commands_[undoIndex]->getDescription();
}

std::string CommandHistory::getRedoDescription() const {
  if (!canRedo()) {
    return "";
  }

  return commands_[currentIndex_]->getDescription();
}

void CommandHistory::clear() {
  spdlog::info("[CommandHistory] clear(): clearing command history (size={})",
               commands_.size());
  commands_.clear();
  currentIndex_ = 0;
}

size_t CommandHistory::getHistorySize() const { return commands_.size(); }

size_t CommandHistory::getCurrentIndex() const { return currentIndex_; }

ICommand *CommandHistory::getCommandAt(size_t index) {
  if (index >= commands_.size()) {
    spdlog::warn(
        "[CommandHistory] getCommandAt: index {} out of bounds (size={})",
        index, commands_.size());
    return nullptr;
  }
  return commands_[index].get();
}

const ICommand *CommandHistory::getCommandAt(size_t index) const {
  if (index >= commands_.size()) {
    spdlog::warn(
        "[CommandHistory] getCommandAt: index {} out of bounds (size={}) const",
        index, commands_.size());
    return nullptr;
  }
  return commands_[index].get();
}

} // namespace view
