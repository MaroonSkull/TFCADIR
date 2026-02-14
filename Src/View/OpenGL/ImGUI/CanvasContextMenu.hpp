#pragma once

#include "ContextMenu.hpp"
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/Navigation/NavigationManager.hpp>
#include <View/Precision/GridSettingsPanel.hpp>
#include <View/Precision/SnapSettingsPanel.hpp>
#include <functional>
#include <memory>

namespace view {

// Forward declarations
class UIFSMAdapter;

/**
 * @brief Context menu for the canvas area
 *
 * Provides context menu actions for the canvas including:
 * - Undo/Redo operations
 * - Cut/Copy/Paste operations
 * - Repeat last command
 * - Pan and Zoom operations
 * - Snap and Grid settings
 *
 * This menu appears when right-clicking on empty canvas space.
 */
class CanvasContextMenu : public ContextMenu {
private:
  /// Reference to the ExtendedCommandManager for undo/redo operations
  ExtendedCommandManager &commandManager_;

  /// Reference to the NavigationManager for pan/zoom operations
  NavigationManager &navigationManager_;

  /// Pointer to UIFSMAdapter for state access
  UIFSMAdapter *uiFSMAdapter_;

  /// Callback to open snap settings dialog
  std::function<void()> onOpenSnapSettings_;

  /// Callback to open grid settings dialog
  std::function<void()> onOpenGridSettings_;

  /// Callback for pan operation
  std::function<void()> onPan_;

  /// Callback for zoom to fit operation
  std::function<void()> onZoomToFit_;

  /// Callback for zoom to selection operation
  std::function<void()> onZoomToSelection_;

  /// Callback for cut operation
  std::function<void()> onCut_;

  /// Callback for copy operation
  std::function<void()> onCopy_;

  /// Callback for paste operation
  std::function<void()> onPaste_;

  /**
   * @brief Build the menu items based on current state
   */
  void buildMenuItems();

public:
  /**
   * @brief Construct a CanvasContextMenu
   * @param commandManager Reference to the command manager for undo/redo
   * @param navigationManager Reference to the navigation manager for pan/zoom
   * @param uiFSMAdapter Pointer to the UIFSMAdapter for state access
   */
  CanvasContextMenu(ExtendedCommandManager &commandManager,
                    NavigationManager &navigationManager,
                    UIFSMAdapter *uiFSMAdapter);

  /**
   * @brief Destructor
   */
  ~CanvasContextMenu() override = default;

  /**
   * @brief Render the canvas context menu
   *
   * Renders the ImGUI popup menu with all canvas actions.
   */
  void render() override;

  /**
   * @brief Set callback for opening snap settings dialog
   * @param callback Function to call when snap settings should be opened
   */
  void setOnOpenSnapSettings(std::function<void()> callback);

  /**
   * @brief Set callback for opening grid settings dialog
   * @param callback Function to call when grid settings should be opened
   */
  void setOnOpenGridSettings(std::function<void()> callback);

  /**
   * @brief Set callback for pan operation
   * @param callback Function to call when pan is requested
   */
  void setOnPan(std::function<void()> callback);

  /**
   * @brief Set callback for zoom to fit operation
   * @param callback Function to call when zoom to fit is requested
   */
  void setOnZoomToFit(std::function<void()> callback);

  /**
   * @brief Set callback for zoom to selection operation
   * @param callback Function to call when zoom to selection is requested
   */
  void setOnZoomToSelection(std::function<void()> callback);

  /**
   * @brief Set callback for cut operation
   * @param callback Function to call when cut is requested
   */
  void setOnCut(std::function<void()> callback);

  /**
   * @brief Set callback for copy operation
   * @param callback Function to call when copy is requested
   */
  void setOnCopy(std::function<void()> callback);

  /**
   * @brief Set callback for paste operation
   * @param callback Function to call when paste is requested
   */
  void setOnPaste(std::function<void()> callback);
};

} // namespace view
