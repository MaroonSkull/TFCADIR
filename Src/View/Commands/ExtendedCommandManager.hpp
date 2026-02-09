#pragma once

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
 * ExtendedCommandManager is a stateless coordinator that follows the Phase 3
 * SelectionManager pattern. It does NOT store command history locally - all
 * command state is queried from UIFSMAdapter (the single source of truth).
 *
 * This class provides Phase 4 typed commands (ICommand interface).
 *
 * **Design Principles:**
 * - Stateless coordinator (no local command history storage)
 * - Query-based (delegates to UIFSMAdapter for command state)
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
   * @param fsmAdapter Reference to the UIFSMAdapter for command state queries
   * @param model Reference to the model for figure operations
   */
  ExtendedCommandManager(UIFSMAdapter &fsmAdapter, model::FlatFigures &model);

  /**
   * @brief Destructor
   */
  ~ExtendedCommandManager() = default;

  // ==========================================================================
  // Phase 4 Typed Command API
  // ==========================================================================

  /**
   * @brief Execute a typed command
   * @param command Unique pointer to the command to execute
   *
   * Executes the command and adds it to the command history if successful.
   * The command must implement the ICommand interface.
   *
   * @note Command history management requires UIFSMAdapter extension with
   *       command state storage. This method executes the command directly
   *       without history storage until that extension is complete.
   */
  void executeCommand(std::unique_ptr<ICommand> command);

  /**
   * @brief Execute a macro command (composite command)
   * @param macro Unique pointer to the macro command to execute
   *
   * Executes all sub-commands in the macro atomically.
   * If any sub-command fails, all executed commands are rolled back.
   *
   * @note Command history management requires UIFSMAdapter extension with
   *       command state storage. This method executes the macro directly
   *       without history storage until that extension is complete.
   */
  void executeMacro(std::unique_ptr<ICommand> macro);

  // ==========================================================================
  // Query Methods for UI (Delegate to UIFSMAdapter)
  // ==========================================================================

  /**
   * @brief Get the complete command history
   * @return Vector of command info structures
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       state management. Returns empty vector until extension is complete.
   */
  std::vector<CommandInfo> getCommandHistory() const;

  /**
   * @brief Get the current command (last executed)
   * @return Command info structure
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       state management. Returns empty CommandInfo until extension is
   * complete.
   */
  CommandInfo getCurrentCommand() const;

  /**
   * @brief Get the command that would be undone
   * @return Command info structure
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       state management. Returns empty CommandInfo until extension is
   * complete.
   */
  CommandInfo getUndoCommand() const;

  /**
   * @brief Get the command that would be redone
   * @return Command info structure
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       state management. Returns empty CommandInfo until extension is
   * complete.
   */
  CommandInfo getRedoCommand() const;

  /**
   * @brief Get the total number of commands in history
   * @return Size of command history
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       state management. Returns 0 until extension is complete.
   */
  size_t getHistorySize() const;

  /**
   * @brief Get the current command index
   * @return Current index in command history
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       state management. Returns 0 until extension is complete.
   */
  size_t getCurrentIndex() const;

  // ==========================================================================
  // Phase 2: Undo/Redo Query Methods (Backward Compatibility)
  // These methods delegate to UIFSMAdapter for state access
  // ==========================================================================

  /**
   * @brief Check if undo is available
   * @return true if there is a command to undo
   *
   * Delegates to UIFSMAdapter to check if undo is available.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  bool canUndo() const;

  /**
   * @brief Check if redo is available
   * @return true if there is a command to redo
   *
   * Delegates to UIFSMAdapter to check if redo is available.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  bool canRedo() const;

  /**
   * @brief Get description of the command that would be undone
   * @return Description of the next undo command, or empty string if none
   *
   * Delegates to UIFSMAdapter to get the undo command description.
   * This method maintains backward compatibility with Phase 2 tools.
   */
  std::string getUndoDescription() const;

  /**
   * @brief Get description of the command that would be redone
   * @return Description of the next redo command, or empty string if none
   *
   * Delegates to UIFSMAdapter to get the redo command description.
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
   *
   * @note This method requires UIFSMAdapter extension with command history
   *       storage. Logs a warning until extension is complete.
   */
  void saveHistory(const std::string &filepath);

  /**
   * @brief Load command history from a file
   * @param factory Command factory for deserializing commands
   * @param filepath Path to the file containing saved history
   *
   * Deserializes commands from JSON and restores the command history.
   *
   * @note This method requires UIFSMAdapter extension with command history
   *       storage. Logs a warning until extension is complete.
   */
  void loadHistory(const std::shared_ptr<class CommandFactory> &factory,
                   const std::string &filepath);

private:
  /// Reference to the UIFSMAdapter (stateless coordinator queries this)
  UIFSMAdapter &fsmAdapter_;

  /// Reference to the model for figure operations
  model::FlatFigures &model_;

  /**
   * @brief Notify listeners that a command was executed
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       notification callbacks.
   */
  void notifyCommandExecuted();

  /**
   * @brief Notify listeners that a command was undone
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       notification callbacks.
   */
  void notifyCommandUndone();

  /**
   * @brief Notify listeners that a command was redone
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       notification callbacks.
   */
  void notifyCommandRedone();

  /**
   * @brief Notify listeners that command history was cleared
   *
   * @note This method requires UIFSMAdapter extension with Phase 4 command
   *       notification callbacks.
   */
  void notifyHistoryCleared();
};

} // namespace view
