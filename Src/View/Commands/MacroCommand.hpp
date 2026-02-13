#pragma once

#include "ICommand.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Composite command that executes multiple commands atomically
 *
 * MacroCommand implements the composite pattern for commands, allowing multiple
 * commands to be grouped and executed as a single atomic operation. If any
 * sub-command fails during execution, all previously executed commands are
 * automatically undone in reverse order (rollback behavior).
 *
 * This ensures that either all commands succeed, or none do (atomic behavior).
 */
class MacroCommand : public Commands::ICommand {
public:
  /**
   * @brief Constructs a macro command from a list of sub-commands
   * @param commands The list of commands to execute as a group
   */
  explicit MacroCommand(
      std::vector<std::unique_ptr<Commands::ICommand>> commands);

  /**
   * @brief Destructor
   */
  ~MacroCommand() override = default;

  /**
   * @brief Executes all sub-commands atomically
   *
   * If any sub-command fails, all previously executed commands are undone
   * in reverse order before throwing an exception.
   * @throws std::runtime_error if execution fails
   */
  void execute() override;

  /**
   * @brief Undoes all executed sub-commands in reverse order
   * @throws std::runtime_error if undo operation fails
   */
  void undo() override;

  /**
   * @brief Returns a description of the macro command
   * @return Human-readable description
   */
  std::string getDescription() const override;

  /**
   * @brief Serializes the macro command to JSON
   * @return JSON string representation
   */
  std::string serialize() const override;

  /**
   * @brief Returns the command type identifier
   * @return "Macro"
   */
  std::string getType() const override;

  /**
   * @brief Returns all affected figure IDs from all sub-commands
   * @return Vector of unique figure IDs
   */
  std::vector<uint32_t> getAffectedFigures() const override;

private:
  /// The list of sub-commands to execute
  std::vector<std::unique_ptr<Commands::ICommand>> commands_;

  /// The number of commands that were successfully executed
  size_t executedCount_;

  /// Tracks whether the macro command has been executed
  bool executed_;
};

} // namespace view
