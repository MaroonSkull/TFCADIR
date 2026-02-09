#include "ExtendedCommandManager.hpp"

#include "../UIFSMAdapter.hpp"
#include "Model/FlatFigure.hpp"
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace view {

ExtendedCommandManager::ExtendedCommandManager(UIFSMAdapter &fsmAdapter,
                                               model::FlatFigures &model)
    : fsmAdapter_(fsmAdapter), model_(model) {
  spdlog::info("[ExtendedCommandManager] Initialized (stateless coordinator)");
}

// ==========================================================================
// Phase 4 Typed Command API
// ==========================================================================

void ExtendedCommandManager::executeCommand(std::unique_ptr<ICommand> command) {
  spdlog::debug(
      "[ExtendedCommandManager] executeCommand (Phase 4 typed): type={}",
      command->getType());

  // Execute the command directly without history storage
  // Command history management requires UIFSMAdapter extension with
  // command state storage and callbacks
  if (command->execute()) {
    spdlog::info("[ExtendedCommandManager] Command executed successfully: {}",
                 command->getDescription());
    notifyCommandExecuted();
  } else {
    spdlog::error("[ExtendedCommandManager] Command execution failed: {}",
                  command->getType());
  }
}

void ExtendedCommandManager::executeMacro(std::unique_ptr<ICommand> macro) {
  spdlog::debug("[ExtendedCommandManager] executeMacro (Phase 4)");

  // Execute the macro directly without history storage
  // Command history management requires UIFSMAdapter extension with
  // command state storage and callbacks
  if (macro->execute()) {
    spdlog::info("[ExtendedCommandManager] Macro executed successfully: {}",
                 macro->getDescription());
    notifyCommandExecuted();
  } else {
    spdlog::error("[ExtendedCommandManager] Macro execution failed");
  }
}

// ==========================================================================
// Query Methods for UI (Delegate to UIFSMAdapter)
// ==========================================================================

std::vector<CommandInfo> ExtendedCommandManager::getCommandHistory() const {
  spdlog::debug("[ExtendedCommandManager] getCommandHistory()");

  // Return empty vector until UIFSMAdapter is extended with
  // Phase 4 command state management
  return std::vector<CommandInfo>{};
}

CommandInfo ExtendedCommandManager::getCurrentCommand() const {
  spdlog::debug("[ExtendedCommandManager] getCurrentCommand()");

  // Return empty CommandInfo until UIFSMAdapter is extended with
  // Phase 4 command state management
  return CommandInfo{};
}

CommandInfo ExtendedCommandManager::getUndoCommand() const {
  spdlog::debug("[ExtendedCommandManager] getUndoCommand()");

  // Return empty CommandInfo until UIFSMAdapter is extended with
  // Phase 4 command state management
  return CommandInfo{};
}

CommandInfo ExtendedCommandManager::getRedoCommand() const {
  spdlog::debug("[ExtendedCommandManager] getRedoCommand()");

  // Return empty CommandInfo until UIFSMAdapter is extended with
  // Phase 4 command state management
  return CommandInfo{};
}

size_t ExtendedCommandManager::getHistorySize() const {
  spdlog::debug("[ExtendedCommandManager] getHistorySize()");

  // Return 0 until UIFSMAdapter is extended with
  // Phase 4 command state management
  return 0;
}

size_t ExtendedCommandManager::getCurrentIndex() const {
  spdlog::debug("[ExtendedCommandManager] getCurrentIndex()");

  // Return 0 until UIFSMAdapter is extended with
  // Phase 4 command state management
  return 0;
}

// ==========================================================================
// Phase 2: Undo/Redo Query Methods (Backward Compatibility)
// These methods delegate to UIFSMAdapter for state access
// ==========================================================================

bool ExtendedCommandManager::canUndo() const {
  /// Delegate to UIFSMAdapter to check if undo is available
  /// Following the stateless coordinator pattern (no local state)
  return fsmAdapter_.canUndo();
}

bool ExtendedCommandManager::canRedo() const {
  /// Delegate to UIFSMAdapter to check if redo is available
  /// Following the stateless coordinator pattern (no local state)
  return fsmAdapter_.canRedo();
}

std::string ExtendedCommandManager::getUndoDescription() const {
  /// Delegate to UIFSMAdapter to get the undo command description
  /// Following the stateless coordinator pattern (no local state)
  return fsmAdapter_.getUndoDescription();
}

std::string ExtendedCommandManager::getRedoDescription() const {
  /// Delegate to UIFSMAdapter to get the redo command description
  /// Following the stateless coordinator pattern (no local state)
  return fsmAdapter_.getRedoDescription();
}

// ==========================================================================
// Persistence Methods
// ==========================================================================

void ExtendedCommandManager::saveHistory(const std::string &filepath) {
  spdlog::info("[ExtendedCommandManager] saveHistory: {}", filepath);

  // Log warning until UIFSMAdapter is extended with command history storage
  spdlog::warn(
      "[ExtendedCommandManager] saveHistory requires UIFSMAdapter extension "
      "with command history storage");
}

void ExtendedCommandManager::loadHistory(
    const std::shared_ptr<CommandFactory> &factory,
    const std::string &filepath) {
  spdlog::info("[ExtendedCommandManager] loadHistory: {}", filepath);

  // Log warning until UIFSMAdapter is extended with command history storage
  spdlog::warn(
      "[ExtendedCommandManager] loadHistory requires UIFSMAdapter extension "
      "with command history storage");
}

// ==========================================================================
// Private Notification Methods
// ==========================================================================

void ExtendedCommandManager::notifyCommandExecuted() {
  spdlog::debug("[ExtendedCommandManager] notifyCommandExecuted");

  // Notification requires UIFSMAdapter extension with Phase 4
  // command notification callbacks
  //
  // Example implementation after UIFSMAdapter extension:
  // if (fsmAdapter_.onCommandExecuted_) {
  //     fsmAdapter_.onCommandExecuted_(getCurrentCommand());
  // }
}

void ExtendedCommandManager::notifyCommandUndone() {
  spdlog::debug("[ExtendedCommandManager] notifyCommandUndone");

  // Notification requires UIFSMAdapter extension with Phase 4
  // command notification callbacks
}

void ExtendedCommandManager::notifyCommandRedone() {
  spdlog::debug("[ExtendedCommandManager] notifyCommandRedone");

  // Notification requires UIFSMAdapter extension with Phase 4
  // command notification callbacks
}

void ExtendedCommandManager::notifyHistoryCleared() {
  spdlog::debug("[ExtendedCommandManager] notifyHistoryCleared");

  // Notification requires UIFSMAdapter extension with Phase 4
  // command notification callbacks
}

} // namespace view
