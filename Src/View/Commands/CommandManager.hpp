#pragma once

#include "CommandStack.hpp"
#include "ICommand.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace view::Commands {

/**
 * @brief Manages command execution with undo/redo support
 * @details Provides high-level interface for command execution and history
 *
 * This class implements the Command pattern's Invoker component,
 * managing command stacks for undo/redo functionality.
 *
 * Design simplification (addresses MAJOR-1 from validation):
 * - Simplified interface without CommandRegistry overhead
 * - Direct CommandStack management
 * - FSM integration handled by calling code
 *
 * Thread Safety: This class is NOT thread-safe. External synchronization
 * is required if used from multiple threads.
 */
class CommandManager {
public:
  /**
   * @brief Construct a CommandManager with optional logger
   * @param logger Optional logger for diagnostics
   */
  explicit CommandManager(std::shared_ptr<spdlog::logger> logger = nullptr);

  /**
   * @brief Destructor
   */
  ~CommandManager() = default;

  // Copy prohibition
  CommandManager(const CommandManager &) = delete;
  CommandManager &operator=(const CommandManager &) = delete;

  // Move permission
  CommandManager(CommandManager &&) noexcept = default;
  CommandManager &operator=(CommandManager &&) noexcept = default;

  /**
   * @brief Execute a command and add it to undo stack
   * @param command Command to execute (takes ownership)
   * @throws std::runtime_error if command execution fails
   *
   * Executes the command immediately and stores it for potential undo.
   * Any existing redo history is cleared as executing a new command
   * invalidates previous undo/redo history.
   */
  void executeCommand(std::unique_ptr<ICommand> command);

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
   * @brief Clear all command history
   *
   * Removes all commands from both undo and redo stacks.
   * Useful for clearing history when resetting application state.
   */
  void clear();

  /**
   * @brief Get the number of commands available for undo
   * @return Size of undo stack
   */
  size_t getUndoCount() const;

  /**
   * @brief Get the number of commands available for redo
   * @return Size of redo stack
   */
  size_t getRedoCount() const;

private:
  /// CommandStack for managing undo/redo history
  CommandStack commandStack_;

  /// Logger for diagnostics
  std::shared_ptr<spdlog::logger> logger_;
};

} // namespace view::Commands
