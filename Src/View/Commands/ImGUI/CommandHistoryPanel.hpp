#pragma once

#include "../ExtendedCommandManager.hpp"
#include <chrono>
#include <string>
#include <vector>

namespace view {

/**
 * @brief ImGUI panel for displaying and managing command history
 *
 * CommandHistoryPanel provides a visual representation of the command history,
 * showing both the undo stack (executed commands) and redo stack (available
 * commands to redo). The panel allows users to navigate the command history,
 * perform undo/redo operations, and view command metadata.
 *
 * **Features:**
 * - Two-pane list view (undo stack and redo stack)
 * - Command info display (type, description, timestamp)
 * - Current position highlighting
 * - Click-to-jump navigation
 * - Undo/redo buttons with keyboard shortcuts
 * - Real-time updates on command execution
 * - Performance caching (100ms update rate limit, dirty flag)
 *
 * **Cache Invalidation Protocol:**
 * 1. ExtendedCommandManager calls `markDirty()` after every command operation
 * 2. Panel checks dirty flag every frame
 * 3. Cache rebuilds if dirty AND 100ms elapsed since last update
 * 4. Dirty flag reset after cache rebuild
 */
class CommandHistoryPanel {
public:
  /**
   * @brief Construct a CommandHistoryPanel
   * @param commandManager Reference to the ExtendedCommandManager for queries
   */
  explicit CommandHistoryPanel(ExtendedCommandManager &commandManager);

  /**
   * @brief Destructor
   */
  ~CommandHistoryPanel() = default;

  /**
   * @brief Render the command history panel
   *
   * Main render method that displays the complete command history UI including
   * toolbar, redo stack, current position indicator, and undo stack.
   */
  void render() const;

  /**
   * @brief Mark cache as dirty
   *
   * Called by ExtendedCommandManager after command state changes to trigger
   * cache rebuild on next render.
   */
  void markDirty() { historyCache_.dirty = true; }

private:
  /**
   * @brief Render the toolbar with undo/redo/clear buttons
   */
  void renderToolbar() const;

  /**
   * @brief Render the redo stack section
   */
  void renderRedoStack() const;

  /**
   * @brief Render the current position indicator
   */
  void renderCurrentPosition() const;

  /**
   * @brief Render the undo stack section
   */
  void renderUndoStack() const;

  /**
   * @brief Render a single command item in the list
   * @param info Command information to display
   * @param isCurrent Whether this is the current command
   * @param index Index of the command in history
   */
  void renderCommandItem(const CommandInfo &info, bool isCurrent,
                         size_t index) const;

  /**
   * @brief Update the cached command history
   *
   * Queries ExtendedCommandManager for the current command history and
   * populates the cache for rendering.
   */
  void updateHistoryCache() const;

  /**
   * @brief Render the cached command history
   *
   * Renders the command list using cached data.
   */
  void renderCachedHistory() const;

  /// Reference to the command manager for queries
  ExtendedCommandManager &commandManager_;

  // UI-only state (not in FSM) - mutable for const render() method
  mutable std::string filterText_;           ///< Text filter for command search
  mutable bool showRedoStackExpanded_{true}; ///< Whether redo stack is expanded
  mutable bool showUndoStackExpanded_{true}; ///< Whether undo stack is expanded

  /**
   * @brief Performance cache for command history
   */
  mutable struct HistoryCache {
    std::vector<CommandInfo> cachedCommands;  ///< Cached command list
    std::vector<CommandInfo> cachedUndoStack; ///< Cached undo stack
    std::vector<CommandInfo> cachedRedoStack; ///< Cached redo stack
    size_t cachedCurrentIndex{0};             ///< Cached current index
    std::chrono::steady_clock::time_point
        lastUpdate;   ///< Last cache update time
    bool dirty{true}; ///< Cache dirty flag
  } historyCache_;
};

} // namespace view
