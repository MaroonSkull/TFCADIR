/**
 * @file CommandManager.cpp
 * @brief Implementation of CommandManager for undo/redo support
 *
 * DESIGN DECISION: Internal State Storage
 * ==========================================
 * This class stores undoStack_ and redoStack_ internally rather than in the
 * FSM.
 *
 * WHY THIS IS ARCHITECTURALLY CORRECT:
 * ------------------------------------
 * 1. Undo/redo stacks are TRANSIENT command history, not domain state
 *    - Domain state (figures, layers, tools) lives in FSM
 *    - Command history is an implementation detail of the Command pattern
 *
 * 2. Similar to other transient state in the system:
 *    - CameraController stores camera position/view state internally
 *    - ImGUI stores UI widget state internally
 *    - CommandManager stores undo/redo history internally
 *
 * 3. The FSM stores domain model state:
 *    - Figures created by drawing operations
 *    - Current tool and collected points
 *    - Tool options and configuration
 *
 * 4. CommandManager stores command execution history:
 *    - Order of operations performed
 *    - Inverse operations for undo
 *    - Redo stack for undone operations
 *
 * This separation of concerns keeps domain state (FSM) distinct from
 * execution history (CommandManager), making both clearer and more
 * maintainable.
 */

#include "CommandManager.hpp"
#include <stdexcept>

namespace view {
namespace ImGUI {

void CommandManager::executeCommand(Command execute, Command unexecute) {
  /// Execute the command
  try {
    execute();
  } catch (const std::exception &e) {
    /// Command failed - rethrow without adding to undo stack
    throw;
  } catch (...) {
    /// Command failed with unknown exception - rethrow without adding to undo
    /// stack
    throw;
  }

  /// Clear redo stack when a new command is executed
  /// This is standard undo/redo behavior - new actions invalidate redo history
  while (!redoStack_.empty()) {
    redoStack_.pop();
  }

  /// Store the reversible command on undo stack
  undoStack_.push(ReversibleCommand(std::move(execute), std::move(unexecute)));
}

void CommandManager::executeCommand(Command command) {
  /// Execute the command without undo support
  try {
    command();
  } catch (const std::exception &e) {
    /// Command failed - rethrow
    throw;
  } catch (...) {
    /// Command failed with unknown exception - rethrow
    throw;
  }

  /// Clear redo stack when a new command is executed
  /// This is standard undo/redo behavior - new actions invalidate redo history
  while (!redoStack_.empty()) {
    redoStack_.pop();
  }

  /// NOTE: This overload executes commands but does not store them for undo.
  /// For reversible operations, use executeCommand(execute, unexecute) instead.
}

void CommandManager::undo() {
  if (undoStack_.empty()) {
    throw std::runtime_error("Cannot undo: no commands available");
  }

  /// Pop the most recent command from undo stack
  ReversibleCommand command = std::move(undoStack_.top());
  undoStack_.pop();

  /// Execute the inverse operation to undo
  if (command.unexecute) {
    command.unexecute();
  }

  /// Push to redo stack so the command can be redone
  redoStack_.push(std::move(command));
}

void CommandManager::redo() {
  if (redoStack_.empty()) {
    throw std::runtime_error("Cannot redo: no commands available");
  }

  /// Pop the most recent command from redo stack
  ReversibleCommand command = std::move(redoStack_.top());
  redoStack_.pop();

  /// Execute the forward operation to redo
  if (command.execute) {
    command.execute();
  }

  /// Push back to undo stack
  undoStack_.push(std::move(command));
}

void CommandManager::clear() {
  /// Clear both stacks, discarding all command history
  while (!undoStack_.empty()) {
    undoStack_.pop();
  }
  while (!redoStack_.empty()) {
    redoStack_.pop();
  }
}

} // namespace ImGUI
} // namespace view
