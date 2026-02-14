#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace view::Commands {

/**
 * @brief Interface for command objects in the Command pattern
 * @details Provides undo/redo capability for user actions
 *
 * This interface defines the contract for all executable commands
 * in the TFCADIR application. Commands encapsulate user actions
 * and provide undo functionality through the Command pattern.
 *
 * @note Implementations must be exception-safe in execute() and undo()
 */
class ICommand {
public:
  /**
   * @brief Virtual destructor for proper cleanup
   */
  virtual ~ICommand() = default;

  /**
   * @brief Execute the command
   * @throws std::runtime_error if execution fails
   *
   * This method performs the primary action of the command.
   * Implementations should handle all error conditions and
   * ensure the application state is consistent after execution.
   */
  virtual void execute() = 0;

  /**
   * @brief Undo the command
   * @throws std::runtime_error if undo operation fails
   *
   * This method reverses the action performed by execute().
   * Implementations should restore the application to the exact
   * state before execute() was called.
   */
  virtual void undo() = 0;

  /**
   * @brief Get human-readable description of the command
   * @return Description string suitable for UI display
   *
   * This description is shown in command history panels,
   * undo/redo menus, and tooltips. Should be concise but informative.
   */
  virtual std::string getDescription() const = 0;

  /**
   * @brief Get the type identifier of the command
   * @return Type string for command identification
   *
   * This method returns a string identifier for the command type,
   * used for serialization and command history display.
   */
  virtual std::string getType() const = 0;

  /**
   * @brief Serialize the command to JSON format
   * @return JSON string representation of the command
   *
   * This method serializes the command's state to a JSON string
   * for persistence and history saving.
   */
  virtual std::string serialize() const = 0;

  /**
   * @brief Get the list of figure IDs affected by this command
   * @return Vector of figure IDs
   *
   * This method returns the list of figure IDs that were affected
   * by the command execution, used for UI display and tracking.
   */
  virtual std::vector<uint32_t> getAffectedFigures() const = 0;
};

} // namespace view::Commands
