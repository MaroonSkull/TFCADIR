#pragma once

#include "CommandHistory.hpp"
#include "ICommand.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace model {
class FlatFigures;
}

namespace view {

class UIFSMAdapter;

// Forward declarations
class CommandFactory;
class CommandHistoryPanel;

/**
 * @brief Information structure for command history display
 *
 * Contains metadata about a command for display in the command history UI.
 */
struct CommandInfo {
  size_t index{0};                       ///< Index in command history
  std::string type;                      ///< Command type identifier
  std::string description;               ///< Human-readable description
  std::string timestamp;                 ///< Command execution timestamp
  std::vector<uint32_t> affectedFigures; ///< Figures affected by command
  bool isMacro{false};                   ///< Whether this is a macro command
  size_t subCommandCount{0}; ///< Number of sub-commands (for macros)
};

/**
 * @brief Extended command manager supporting Phase 4 command API
 *
 * ExtendedCommandManager is a stateless coordinator that delegates to
 * CommandHistory for command storage and undo/redo operations. This follows
 * the single responsibility principle - CommandHistory owns the command state,
 * while ExtendedCommandManager coordinates command execution.
 *
 * This class provides Phase 4 typed commands (ICommand interface).
 *
 * **Design Principles:**
 * - Stateless coordinator (delegates to CommandHistory for state)
 * - CommandHistory owns command storage and undo/redo state
 * - Phase 4 ICommand interface
 * - Callback notifications after command operations
 */
class ExtendedCommandManager {
public:
  /**
   * @brief Command function type for legacy compatibility
   *
   * Using std::function allows any callable to be used as a command,
   * including lambdas with captures, function pointers, or functors.
   *
   * @note This type is provided for reference. Phase 4 implementation uses
   *       the ICommand interface instead.
   */
  using Command = std::function<void()>;

  /**
   * @brief Construct an ExtendedCommandManager
   * @param commandHistory Reference to the command history for state management
   * @param model Reference to the model for figure operations
   */
  ExtendedCommandManager(CommandHistory &commandHistory,
                         model::FlatFigures &model);

  /**
   * @brief Destructor
   */
  ~ExtendedCommandManager() = default;

  /**
   * @brief Set the command history panel for cache invalidation notifications
   * @param panel Pointer to the command history panel (can be null)
   *
   * This allows ExtendedCommandManager to notify the panel when command
   * state changes, triggering cache updates. The panel is optional to
   * support headless operation.
   */
  void setCommandHistoryPanel(CommandHistoryPanel *panel);

  // ==========================================================================
  // Phase 4 Typed Command API
  // ==========================================================================

  /**
   * @brief Execute a typed command
   * @param command Unique pointer to the command to execute
   *
   * Executes the command and adds it to the command history if successful.
   * The command must implement the ICommand interface.
   */
  void executeCommand(std::unique_ptr<ICommand> command);

  /**
   * @brief Execute a macro command (composite command)
   * @param macro Unique pointer to the macro command to execute
   *
   * Executes all sub-commands in the macro atomically.
   * If any sub-command fails, all executed commands are rolled back.
   */
  void executeMacro(std::unique_ptr<ICommand> macro);

  /**
   * @brief Undo the last command
   * @return true if successful, false otherwise
   *
   * Delegates to CommandHistory to undo the current command.
   */
  bool undo();

  /**
   * @brief Redo the next command
   * @return true if successful, false otherwise
   *
   * Delegates to CommandHistory to redo the next command.
   */
  bool redo();

  // ==========================================================================
  // Query Methods for UI (Delegate to CommandHistory)
  // ==========================================================================

  /**
   * @brief Get the complete command history
   * @return Vector of command info structures
   */
  std::vector<CommandInfo> getCommandHistory() const;

  /**
   * @brief Get the current command (last executed)
   * @return Command info structure
   */
  CommandInfo getCurrentCommand() const;

  /**
   * @brief Get the command that would be undone
   * @return Command info structure
   */
  CommandInfo getUndoCommand() const;

  /**
   * @brief Get the command that would be redone
   * @return Command info structure
   */
  CommandInfo getRedoCommand() const;

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

  // ==========================================================================
  // Phase 2: Undo/Redo Query Methods (Backward Compatibility)
  // These methods delegate to CommandHistory for state access
  // ==========================================================================

  /**
   * @brief Check if undo is available
   * @return true if there is a command to undo
   *
   * Delegates to CommandHistory to check if undo is available.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  bool canUndo() const;

  /**
   * @brief Check if redo is available
   * @return true if there is a command to redo
   *
   * Delegates to CommandHistory to check if redo is available.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  bool canRedo() const;

  /**
   * @brief Get description of the command that would be undone
   * @return Description of the next undo command, or empty string if none
   *
   * Delegates to CommandHistory to get the undo command description.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  std::string getUndoDescription() const;

  /**
   * @brief Get description of the command that would be redone
   * @return Description of the next redo command, or empty string if none
   *
   * Delegates to CommandHistory to get the redo command description.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  std::string getRedoDescription() const;

  // ==========================================================================
  // Persistence Methods
  // ==========================================================================

  /**
   * @brief Save command history to a file
   * @param filepath Path to the file where history should be saved
   *
   * Serializes the command history to JSON format for persistence.
   */
  void saveHistory(const std::string &filepath);

  /**
   * @brief Load command history from a file
   * @param factory Command factory for deserializing commands
   * @param filepath Path to the file containing saved history
   *
   * Deserializes commands from JSON and restores the command history.
   */
  void loadHistory(const std::shared_ptr<CommandFactory> &factory,
                   const std::string &filepath);

private:
  /// Reference to the command history for state management
  CommandHistory &commandHistory_;

  /// Reference to the model for figure operations
  model::FlatFigures &model_;

  /// Pointer to command history panel for cache invalidation (optional)
  CommandHistoryPanel *commandHistoryPanel_{nullptr};

  /**
   * @brief Notify listeners that a command was executed
   */
  void notifyCommandExecuted();

  /**
   * @brief Notify listeners that a command was undone
   */
  void notifyCommandUndone();

  /**
   * @brief Notify listeners that a command was redone
   */
  void notifyCommandRedone();

  /**
   * @brief Notify listeners that command history was cleared
   */
  void notifyHistoryCleared();

  /**
   * @brief Convert command to CommandInfo structure
   * @param command The command to convert
   * @param index The index of the command in history
   * @return CommandInfo structure with command metadata
   */
  CommandInfo commandToInfo(const ICommand *command, size_t index) const;
};

} // namespace view
