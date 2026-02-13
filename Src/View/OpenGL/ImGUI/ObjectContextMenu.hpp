#pragma once

#include "ContextMenu.hpp"
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace view {

// Forward declarations
class UIFSMAdapter;

/**
 * @brief Context menu for selected objects
 *
 * Provides context menu actions for selected objects including:
 * - Selection operations
 * - Duplicate and Delete operations
 * - Transform operations (Move, Rotate, Scale)
 * - Properties and Layers dialogs
 * - Z-order operations (Bring to Front, Send to Back)
 * - Create Copy and Mirror operations
 *
 * This menu appears when right-clicking on selected objects.
 */
class ObjectContextMenu : public ContextMenu {
private:
  /// Reference to the ExtendedCommandManager for command execution
  ExtendedCommandManager &commandManager_;

  /// Pointer to SelectionManager for selection state
  SelectionManager *selectionManager_;

  /// Pointer to UIFSMAdapter for state access
  UIFSMAdapter *uiFSMAdapter_;

  /// Currently selected object IDs
  std::vector<uint32_t> selectedObjectIds_;

  /// Callback for duplicate operation
  std::function<void(const std::vector<uint32_t> &)> onDuplicate_;

  /// Callback for delete operation
  std::function<void(const std::vector<uint32_t> &)> onDelete_;

  /// Callback for move operation
  std::function<void(const std::vector<uint32_t> &)> onMove_;

  /// Callback for rotate operation
  std::function<void(const std::vector<uint32_t> &)> onRotate_;

  /// Callback for scale operation
  std::function<void(const std::vector<uint32_t> &)> onScale_;

  /// Callback for properties dialog
  std::function<void()> onOpenProperties_;

  /// Callback for layers dialog
  std::function<void()> onOpenLayers_;

  /// Callback for bring to front operation
  std::function<void(const std::vector<uint32_t> &)> onBringToFront_;

  /// Callback for send to back operation
  std::function<void(const std::vector<uint32_t> &)> onSendToBack_;

  /// Callback for create copy operation
  std::function<void(const std::vector<uint32_t> &)> onCreateCopy_;

  /// Callback for mirror operation
  std::function<void(const std::vector<uint32_t> &)> onMirror_;

  /**
   * @brief Build the menu items based on current state
   */
  void buildMenuItems();

public:
  /**
   * @brief Construct an ObjectContextMenu
   * @param commandManager Reference to the command manager for operations
   * @param selectionManager Pointer to the selection manager
   * @param uiFSMAdapter Pointer to the UIFSMAdapter for state access
   */
  ObjectContextMenu(ExtendedCommandManager &commandManager,
                    SelectionManager *selectionManager,
                    UIFSMAdapter *uiFSMAdapter);

  /**
   * @brief Destructor
   */
  ~ObjectContextMenu() override = default;

  /**
   * @brief Set the selected objects for the context menu
   * @param objectIds IDs of the selected objects
   */
  void setSelectedObjects(const std::vector<uint32_t> &objectIds);

  /**
   * @brief Get the selected objects
   * @return IDs of the selected objects
   */
  const std::vector<uint32_t> &getSelectedObjects() const;

  /**
   * @brief Render the object context menu
   *
   * Renders the ImGUI popup menu with all object actions.
   */
  void render() override;

  /**
   * @brief Set callback for duplicate operation
   * @param callback Function to call when duplicate is requested
   */
  void
  setOnDuplicate(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for delete operation
   * @param callback Function to call when delete is requested
   */
  void setOnDelete(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for move operation
   * @param callback Function to call when move is requested
   */
  void setOnMove(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for rotate operation
   * @param callback Function to call when rotate is requested
   */
  void setOnRotate(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for scale operation
   * @param callback Function to call when scale is requested
   */
  void setOnScale(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for opening properties dialog
   * @param callback Function to call when properties should be opened
   */
  void setOnOpenProperties(std::function<void()> callback);

  /**
   * @brief Set callback for opening layers dialog
   * @param callback Function to call when layers should be opened
   */
  void setOnOpenLayers(std::function<void()> callback);

  /**
   * @brief Set callback for bring to front operation
   * @param callback Function to call when bring to front is requested
   */
  void setOnBringToFront(
      std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for send to back operation
   * @param callback Function to call when send to back is requested
   */
  void
  setOnSendToBack(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for create copy operation
   * @param callback Function to call when create copy is requested
   */
  void
  setOnCreateCopy(std::function<void(const std::vector<uint32_t> &)> callback);

  /**
   * @brief Set callback for mirror operation
   * @param callback Function to call when mirror is requested
   */
  void setOnMirror(std::function<void(const std::vector<uint32_t> &)> callback);
};

} // namespace view
