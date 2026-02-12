#pragma once

#include "ICommand.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

namespace view::Commands {

/**
 * @brief Manages command history for undo/redo functionality
 * @details Implements the Command pattern's invoker component
 *
 * CommandStack maintains two stacks: one for executed commands (undo stack)
 * and one for undone commands (redo stack). This allows users to reverse
 * actions and reapply them.
 *
 * Thread Safety: This class is NOT thread-safe. External synchronization
 * is required if used from multiple threads.
 */
class CommandStack {
public:
  /**
   * @brief Construct an empty CommandStack
   * @param logger Optional logger for diagnostics
   */
  explicit CommandStack(std::shared_ptr<spdlog::logger> logger = nullptr);

  /**
   * @brief Destructor - clears both stacks
   */
  ~CommandStack() = default;

  // Copy prohibition
  CommandStack(const CommandStack &) = delete;
  CommandStack &operator=(const CommandStack &) = delete;

  // Move permission
  CommandStack(CommandStack &&) noexcept = default;
  CommandStack &operator=(CommandStack &&) noexcept = default;

  /**
   * @brief Execute a command and add it to the undo stack
   * @param command Command to execute (takes ownership)
   * @throws std::runtime_error if command execution fails
   *
   * Executes the command and, if successful, adds it to the undo stack.
   * Clears the redo stack as new commands invalidate redo history.
   *
   * @note The command must be valid and non-null
   */
  void execute(std::unique_ptr<ICommand> command);

  /**
   * @brief Undo the most recently executed command
   * @throws std::runtime_error if undo stack is empty or undo fails
   *
   * Pops the most recent command from the undo stack, calls its undo()
   * method, and pushes it onto the redo stack.
   */
  void undo();

  /**
   * @brief Redo the most recently undone command
   * @throws std::runtime_error if redo stack is empty or redo fails
   *
   * Pops the most recent command from the redo stack, calls its execute()
   * method, and pushes it back onto the undo stack.
   */
  void redo();

  /**
   * @brief Check if undo is available
   * @return true if there are commands on the undo stack
   */
  bool canUndo() const;

  /**
   * @brief Check if redo is available
   * @return true if there are commands on the redo stack
   */
  bool canRedo() const;

  /**
   * @brief Get description of command that would be undone
   * @return Description string or empty string if no undo available
   */
  std::string getUndoDescription() const;

  /**
   * @brief Get description of command that would be redone
   * @return Description string or empty string if no redo available
   */
  std::string getRedoDescription() const;

  /**
   * @brief Clear all commands from both stacks
   *
   * Removes all commands from undo and redo stacks.
   * Useful for clearing history when loading a new document.
   */
  void clear();

  /**
   * @brief Get the number of commands in the undo stack
   * @return Size of undo stack
   */
  size_t getUndoStackSize() const;

  /**
   * @brief Get the number of commands in the redo stack
   * @return Size of redo stack
   */
  size_t getRedoStackSize() const;

private:
  /// Stack of executed commands available for undo
  std::vector<std::unique_ptr<ICommand>> undoStack_;

  /// Stack of undone commands available for redo
  std::vector<std::unique_ptr<ICommand>> redoStack_;

  /// Logger for diagnostics
  std::shared_ptr<spdlog::logger> logger_;
};

} // namespace view::Commands
