#pragma once

#include <functional>
#include <stack>
#include <string>

namespace view {
namespace ImGUI {

/**
 * @brief Manages command execution with undo/redo support
 *
 * CommandManager implements the Command pattern for reversible operations
 * in the ImGUI interface. It maintains separate stacks for undo and redo,
 * allowing users to reverse and reapply commands.
 *
 * Commands are represented as std::function<void()> objects, providing
 * flexibility in what constitutes a "command" - from simple parameter
 * changes to complex drawing operations.
 */
class CommandManager {
public:
  /**
   * @brief Command function type
   *
   * Using std::function allows any callable to be used as a command,
   * including lambdas with captures, function pointers, or functors.
   */
  using Command = std::function<void()>;

  /**
   * @brief Construct a CommandManager
   */
  CommandManager() = default;

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
   * @brief Execute a reversible command and add it to the undo stack
   * @param execute The command to execute
   * @param unexecute The inverse command to undo the operation
   *
   * Executes the command immediately and stores it on the undo stack.
   * Any existing commands on the redo stack are cleared, as executing
   * a new command invalidates the redo history.
   *
   * The command is executed before being stored, so if it throws an
   * exception, it will not be added to the undo stack.
   *
   * Example:
   * @code
   * manager.executeCommand(
   *     [this, newRadius]() {
   *         toolOptions["radius"] = newRadius;
   *         updatePreview();
   *     },
   *     [this, oldRadius]() {
   *         toolOptions["radius"] = oldRadius;
   *         updatePreview();
   *     }
   * );
   * @endcode
   */
  void executeCommand(Command execute, Command unexecute);

  /**
   * @brief Execute a command without undo support
   * @param command The command to execute
   *
   * Executes the command immediately but does not store it for undo.
   * This overload is provided for operations that don't need undo support.
   * For reversible operations, use the two-parameter overload instead.
   *
   * Example:
   * @code
   * manager.executeCommand([this]() {
   *     updatePreview();
   * });
   * @endcode
   */
  void executeCommand(Command command);

  /**
   * @brief Undo the most recent command
   *
   * Pops the most recent command from the undo stack, executes its
   * inverse operation, and pushes it to the redo stack. The inverse
   * operation is expected to restore the state to what it was before
   * the original command was executed.
   *
   * @throws std::runtime_error if no commands are available to undo
   */
  void undo();

  /**
   * @brief Redo the most recently undone command
   *
   * Pops the most recent command from the redo stack and re-executes it,
   * pushing it back to the undo stack. This restores the state to what
   * it was after the original command was executed.
   *
   * @throws std::runtime_error if no commands are available to redo
   */
  void redo();

  /**
   * @brief Check if undo is available
   * @return true if there are commands on the undo stack
   */
  bool canUndo() const { return !undoStack_.empty(); }

  /**
   * @brief Check if redo is available
   * @return true if there are commands on the redo stack
   */
  bool canRedo() const { return !redoStack_.empty(); }

  /**
   * @brief Clear all command history
   *
   * Clears both undo and redo stacks. Useful when resetting the
   * application state or when operations cannot be undone.
   */
  void clear();

  /**
   * @brief Get the number of commands available for undo
   * @return The size of the undo stack
   */
  size_t getUndoCount() const { return undoStack_.size(); }

  /**
   * @brief Get the number of commands available for redo
   * @return The size of the redo stack
   */
  size_t getRedoCount() const { return redoStack_.size(); }

private:
  /**
   * @brief Represents a reversible command with its inverse
   */
  struct ReversibleCommand {
    Command execute;   ///< Command to execute/redo
    Command unexecute; ///< Command to undo

    ReversibleCommand(Command exec, Command unexec)
        : execute(std::move(exec)), unexecute(std::move(unexec)) {}
  };

  /// Stack of commands that can be undone
  std::stack<ReversibleCommand> undoStack_;

  /// Stack of commands that can be redone
  std::stack<ReversibleCommand> redoStack_;
};

} // namespace ImGUI
} // namespace view
