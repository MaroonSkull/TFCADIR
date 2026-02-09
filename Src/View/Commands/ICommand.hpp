#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace view {

/**
 * @brief Interface for all command objects in the command system
 *
 * ICommand defines the contract for all reversible operations in TFCADIR.
 * Each command encapsulates a single user action (create figure, move, rotate,
 * etc.) and provides both execution and undo functionality.
 *
 * All commands support JSON serialization for persistence and session recovery.
 */
class ICommand {
public:
  /**
   * @brief Virtual destructor for proper cleanup of derived classes
   */
  virtual ~ICommand() = default;

  /**
   * @brief Execute the command
   * @return true if successful, false otherwise
   *
   * Performs the primary action of the command. If execution fails,
   * the command should not modify any state and should return false.
   */
  virtual bool execute() = 0;

  /**
   * @brief Undo the command
   * @return true if successful, false otherwise
   *
   * Reverses the effects of execute(), restoring the application
   * to the state before the command was executed.
   */
  virtual bool undo() = 0;

  /**
   * @brief Get command description for UI display
   * @return Human-readable command description
   *
   * Returns a brief description suitable for display in the
   * command history panel (e.g., "Create Triangle", "Move Circle").
   */
  virtual std::string getDescription() const = 0;

  /**
   * @brief Serialize command to JSON for persistence
   * @return JSON string representation of command
   *
   * Serializes the complete command state to JSON format
   * for saving to disk and later deserialization.
   */
  virtual std::string serialize() const = 0;

  /**
   * @brief Get command type identifier
   * @return Type string for deserialization factory
   *
   * Returns the command type name used by CommandFactory
   * to deserialize commands from JSON (e.g., "CreateFigure", "MoveFigure").
   */
  virtual std::string getType() const = 0;

  /**
   * @brief Get figure IDs affected by this command
   * @return Vector of figure IDs
   *
   * Returns a list of all figure IDs that this command
   * modifies or creates. Used for tracking and validation.
   */
  virtual std::vector<uint32_t> getAffectedFigures() const = 0;
};

} // namespace view
