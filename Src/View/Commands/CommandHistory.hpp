#pragma once

#include "ICommand.hpp"
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Service class for managing command history and undo/redo operations
 *
 * CommandHistory stores actual command objects (not just descriptions) and
 * manages the current position in the command history for undo/redo operations.
 * This class follows the single responsibility principle by focusing solely
 * on command history management.
 */
class CommandHistory {
public:
  /**
   * @brief Construct a CommandHistory
   */
  CommandHistory() = default;

  /**
   * @brief Destructor
   */
  ~CommandHistory() = default;

  /**
   * @brief Add an executed command to the history
   * @param command Unique pointer to the executed command
   *
   * Adds the command to the history and sets it as the current command.
   * Any commands after the current position are removed (redo chain cleared).
   */
  void addCommand(std::unique_ptr<ICommand> command);

  /**
   * @brief Check if undo is available
   * @return true if there is a command to undo
   *
   * Returns true if the current index is greater than 0,
   * indicating that there is at least one command that can be undone.
   */
  bool canUndo() const;

  /**
   * @brief Check if redo is available
   * @return true if there is a command to redo
   *
   * Returns true if the current index is less than the history size,
   * indicating that there is at least one command that can be redone.
   */
  bool canRedo() const;

  /**
   * @brief Undo the current command
   * @return true if successful, false otherwise
   *
   * Executes undo on the command at the current index and decrements
   * the current index. Returns false if undo is not available.
   */
  bool undo();

  /**
   * @brief Redo the next command
   * @return true if successful, false otherwise
   *
   * Increments the current index and executes redo on the command at
   * the new position. Returns false if redo is not available.
   */
  bool redo();

  /**
   * @brief Get description of the command that would be undone
   * @return Description of the next undo command, or empty string if none
   */
  std::string getUndoDescription() const;

  /**
   * @brief Get description of the command that would be redone
   * @return Description of the next redo command, or empty string if none
   */
  std::string getRedoDescription() const;

  /**
   * @brief Clear the command history
   *
   * Removes all commands from the history and resets the current index to 0.
   */
  void clear();

  /**
   * @brief Get the total number of commands in history
   * @return Size of command history
   */
  size_t getHistorySize() const;

  /**
   * @brief Get the current command index
   * @return Current index in command history
   */
  size_t getCurrentIndex() const;

  /**
   * @brief Get command at specific index
   * @param index The index of the command to retrieve
   * @return Pointer to the command, or nullptr if index is invalid
   *
   * Returns a raw pointer to the command at the specified index.
   * The command remains owned by CommandHistory.
   */
  ICommand *getCommandAt(size_t index);

  /**
   * @brief Get command at specific index (const overload)
   * @param index The index of the command to retrieve
   * @return Const pointer to the command, or nullptr if index is invalid
   */
  const ICommand *getCommandAt(size_t index) const;

private:
  /// Vector of command objects (actual commands, not just descriptions)
  std::vector<std::unique_ptr<ICommand>> commands_;

  /// Current position in command history (index of last executed command)
  size_t currentIndex_{0};

  /// Maximum history size (optional, for memory management)
  static constexpr size_t MAX_HISTORY_SIZE = 1000;
};

} // namespace view
