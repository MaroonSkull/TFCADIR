#pragma once

#include <Controller/FSM.hpp>
#include <Model/IModel.hpp>
#include <Model/SketchPlane.hpp>
#include <View/CameraController.hpp>
#include <View/Commands/ICommand.hpp>
#include <View/Navigation/NavigationTypes.hpp>
#include <any>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace view {

// ==========================================================================
// Phase 6: Precision & Snapping - Data Structures
// ==========================================================================

/**
 * @brief Grid settings for visualization and snapping
 *
 * Contains configuration for grid display, spacing, colors, and axes.
 * These settings are FSM state stored in UIFSMAdapter.
 */
struct GridSettings {
  /// Whether the grid is visible
  bool visible = true;

  /// Whether to show minor grid lines
  bool showMinorLines = true;

  /// Whether to show X, Y, Z axes
  bool showAxes = true;

  /// Whether to show the origin point
  bool showOrigin = true;

  /// Spacing between major grid lines
  float majorSpacing = 10.0f;

  /// Number of minor divisions between major lines
  int minorDivisions = 10;

  /// Color of major grid lines (RGBA)
  glm::vec4 color = glm::vec4(0.27f, 0.27f, 0.27f, 1.0f);

  /// Color of minor grid lines (RGBA)
  glm::vec4 minorColor = glm::vec4(0.20f, 0.20f, 0.20f, 1.0f);

  /// Grid opacity (0.0 to 1.0)
  float opacity = 1.0f;

  /**
   * @brief Equality operator for GridSettings
   * @param other The other GridSettings to compare
   * @return true if all settings are equal
   */
  bool operator==(const GridSettings &other) const {
    return visible == other.visible && showMinorLines == other.showMinorLines &&
           showAxes == other.showAxes && showOrigin == other.showOrigin &&
           majorSpacing == other.majorSpacing &&
           minorDivisions == other.minorDivisions && color == other.color &&
           minorColor == other.minorColor && opacity == other.opacity;
  }
};

/**
 * @brief Snap settings for precision drawing
 *
 * Contains configuration for snap modes, tolerance, and visual indicators.
 * These settings are FSM state stored in UIFSMAdapter.
 */
struct SnapSettings {
  /// Whether grid snap is enabled
  bool gridEnabled = true;

  /// Whether endpoint snap is enabled
  bool endpointEnabled = true;

  /// Whether midpoint snap is enabled
  bool midpointEnabled = true;

  /// Whether center snap is enabled
  bool centerEnabled = true;

  /// Whether intersection snap is enabled
  bool intersectionEnabled = true;

  /// Whether nearest point snap is enabled
  bool nearestEnabled = false;

  /// Whether tangent snap is enabled
  bool tangentEnabled = false;

  /// Whether perpendicular snap is enabled
  bool perpendicularEnabled = false;

  /// Snap tolerance in pixels
  float tolerancePixels = 10.0f;

  /// Whether to show snap indicators
  bool showIndicators = true;

  /// Color of snap indicators (RGBA)
  glm::vec4 indicatorColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

  /**
   * @brief Equality operator for SnapSettings
   * @param other The other SnapSettings to compare
   * @return true if all settings are equal
   */
  bool operator==(const SnapSettings &other) const {
    return gridEnabled == other.gridEnabled &&
           endpointEnabled == other.endpointEnabled &&
           midpointEnabled == other.midpointEnabled &&
           centerEnabled == other.centerEnabled &&
           intersectionEnabled == other.intersectionEnabled &&
           nearestEnabled == other.nearestEnabled &&
           tangentEnabled == other.tangentEnabled &&
           perpendicularEnabled == other.perpendicularEnabled &&
           tolerancePixels == other.tolerancePixels &&
           showIndicators == other.showIndicators &&
           indicatorColor == other.indicatorColor;
  }
};

// ==========================================================================
// Phase 6: FSM Event Definitions
// These events are defined in fsm::events namespace in FSM.hpp
// and are used for cache invalidation and notifications
// ==========================================================================

} // namespace view

namespace view {

/**
 * @brief Adapter class that bridges FSM state changes with UI updates
 *
 * UIFSMAdapter connects the finite state machine to the user interface,
 * triggering UI updates when the FSM state changes. It handles plane
 * selection UI display and mode status updates.
 */
class UIFSMAdapter {
public:
  /**
   * @brief Callback function type for showing plane selection UI
   * @param onPlaneSelected Function to call when user selects a plane
   */
  using ShowPlaneSelectionCallback =
      std::function<void(std::function<void(int)>)>;

  /**
   * @brief Callback function type for updating status bar text
   * @param text The status text to display
   */
  using UpdateStatusCallback = std::function<void(const std::string &)>;

  /**
   * @brief Construct a UIFSMAdapter
   * @param fsm Reference to the finite state machine
   * @param cameraController Reference to the camera controller
   * @param logger Logger for diagnostic output
   */
  UIFSMAdapter(fsm::Machine &fsm, CameraController &cameraController,
               std::shared_ptr<spdlog::logger> logger);

  /**
   * @brief Destructor
   */
  ~UIFSMAdapter();

  /**
   * @brief Set callback for showing plane selection UI
   * @param callback Function to invoke when plane selection UI should be shown
   */
  void setShowPlaneSelectionCallback(ShowPlaneSelectionCallback callback);

  /**
   * @brief Set callback for updating status bar
   * @param callback Function to invoke when status text should be updated
   */
  void setUpdateStatusCallback(UpdateStatusCallback callback);

  /**
   * @brief Handle FSM state change
   * @param newState The new FSM state
   *
   * Called automatically when the FSM state changes to update the UI
   * accordingly. Triggers plane selection UI or status updates based on the new
   * state.
   */
  void onStateChanged(fsm::State newState);

  /**
   * @brief Enter sketch mode from UI
   *
   * Sends the OnEnterSketchMode event to the FSM to transition to
   * PlaneSelection state.
   */
  void enterSketchMode();

  /**
   * @brief Select a sketch plane
   * @param planeIndex Index of the selected plane (0=Right, 1=Top, 2=Front)
   *
   * Sends the OnPlaneSelected event to the FSM with the selected plane index.
   */
  void selectPlane(int planeIndex);

  /**
   * @brief Exit sketch mode from UI
   *
   * Sends the OnExitSketchMode event to the FSM to return to Idle state.
   */
  void exitSketchMode();

  /**
   * @brief Get the currently selected sketch plane
   * @return Reference to the current sketch plane, or nullptr if none selected
   */
  model::SketchPlane *getCurrentSketchPlane();

  /**
   * @brief Check if currently in sketch mode
   * @return true if in PlaneSelection or SketchEdit state, false otherwise
   */
  bool isInSketchMode() const;

  // ==========================================================================
  // Phase 2: Tool Management Methods
  // These methods provide access to tool state stored in the FSM
  // ==========================================================================

  /**
   * @brief Get the currently active tool ID
   * @return Current tool ID or empty string if no tool active
   *
   * Queries the FSM state data for the activeTool field.
   * Returns the tool identifier (e.g., "Line3D", "Circle3D") or empty string
   * if no tool is currently active.
   */
  std::string getActiveTool() const;

  /**
   * @brief Get options for the active tool
   * @return Map of option names to values
   *
   * Queries the FSM state data for the toolOptions field.
   * Returns a map of option names to their current values for the
   * currently active tool. Options are tool-specific configuration
   * parameters (e.g., radius for Circle3D, sides for Polygon3D).
   */
  std::map<std::string, std::any> getToolOptions() const;

  /**
   * @brief Get collected points for current drawing operation
   * @return Vector of collected point coordinates
   *
   * Queries the FSM state data for the collectedPoints field.
   * Returns the list of 3D points collected so far in the current
   * drawing operation. The number of points depends on the tool:
   * - Line3D: 2 points (start, end)
   * - Circle3D: 2 points (center, edge)
   * - Arc3D: 3 points (start, control, end)
   * - Rectangle3D: 2 points (first corner, opposite corner)
   * - Polygon3D: N points (vertices in order)
   */
  std::vector<glm::vec3> getCollectedPoints() const;

  /**
   * @brief Activate a tool
   * @param toolId Tool identifier to activate
   *
   * Sends the appropriate tool activation event to the FSM:
   * - "Line3D" → OnActivateLine3D event
   * - "Circle3D" → OnActivateCircle3D event
   * - "Arc3D" → OnActivateArc3D event
   * - "Rectangle3D" → OnActivateRectangle3D event
   * - "Polygon3D" → OnActivatePolygon3D event
   * - "NGon3D" → OnActivateNGon3D event
   * - "LineInSketch" → OnActivateLineInSketch event
   * - "CircleInSketch" → OnActivateCircleInSketch event
   *
   * The tool remains active until deactivated or a drawing operation completes.
   */
  void activateTool(const std::string &toolId);

  /**
   * @brief Deactivate the current tool
   *
   * Sends the OnDeactivateTool event to the FSM, clearing the activeTool
   * field in state data. This cancels any pending drawing operation.
   */
  void deactivateTool();

  // ==========================================================================
  // Phase 3: Object Management Methods
  // These methods provide selection state management for figures
  // ==========================================================================

  /**
   * @brief Get the list of selected figure IDs
   * @return Vector of selected figure IDs
   */
  std::vector<uint32_t> getSelectedFigureIds() const;

  /**
   * @brief Get the primary selection index
   * @return Index of primary selection in the selected figures list, or -1
   */
  int getPrimarySelectionIndex() const;

  /**
   * @brief Select a single figure (replaces current selection)
   * @param figureId The figure to select
   */
  void selectFigure(uint32_t figureId);

  /**
   * @brief Toggle selection state of a figure
   * @param figureId The figure to toggle
   */
  void toggleFigureSelection(uint32_t figureId);

  /**
   * @brief Add a figure to the current selection
   * @param figureId The figure to add
   */
  void addToSelection(uint32_t figureId);

  /**
   * @brief Remove a figure from the current selection
   * @param figureId The figure to remove
   */
  void removeFromSelection(uint32_t figureId);

  /**
   * @brief Clear all selections
   */
  void clearSelection();

  /**
   * @brief Set the primary selection by index
   * @param index The index in the selected figures list to set as primary
   */
  void setPrimarySelection(int index);

  /**
   * @brief Get the primary selection figure ID
   * @return Primary selection ID or 0 if no selection
   */
  uint32_t getPrimarySelectionId() const;

  /**
   * @brief Update a property of a specific figure
   * @param figureId The figure to update
   * @param propertyPath The property path (e.g., "center.x", "radius")
   * @param value The new value
   * @note This method implements the property path format from architecture
   * v1.3
   */
  void updateFigureProperty(uint32_t figureId, const std::string &propertyPath,
                            const std::any &value);

  // ==========================================================================
  // Phase 4: Command History Management Methods
  // UIFSMAdapter is the single source of truth for command history storage
  // ==========================================================================

  /**
   * @brief Execute a command and add it to history
   * @param command Unique pointer to the command to execute
   *
   * Executes the command and adds it to the command history if successful.
   * Any commands after the current position are removed (redo chain cleared).
   */
  void executeCommand(std::unique_ptr<ICommand> command);

  /**
   * @brief Undo the last command
   * @return true if successful, false otherwise
   *
   * Executes undo on the command at the current index and decrements
   * the current index. Returns false if undo is not available.
   */
  bool undoCommand();

  /**
   * @brief Redo the next command
   * @return true if successful, false otherwise
   *
   * Increments the current index and executes redo on the command at
   * the new position. Returns false if redo is not available.
   */
  bool redoCommand();

  /**
   * @brief Clear the command history
   *
   * Removes all commands from the history and resets the current index to 0.
   */
  void clearCommandHistory();

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
   * @brief Get the total number of commands in history
   * @return Size of command history
   */
  size_t getHistorySize() const;

  /**
   * @brief Get the current command index
   * @return Current index in command history
   */
  size_t getCurrentCommandIndex() const;

  /**
   * @brief Get command at specific index
   * @param index The index of the command to retrieve
   * @return Pointer to the command, or nullptr if index is invalid
   *
   * Returns a raw pointer to the command at the specified index.
   * The command remains owned by UIFSMAdapter.
   */
  const ICommand *getCommandAt(size_t index) const;

  // ==========================================================================
  // Phase 5: Navigation State Management Methods
  // UIFSMAdapter is the single source of truth for navigation domain state
  // ==========================================================================

  /**
   * @brief Callback type for navigation state change notifications
   */
  using NavigationCallback = std::function<void()>;

  /**
   * @brief Set the orbit center for 3D navigation
   * @param center The orbit center type
   */
  void setOrbitCenter(OrbitCenter center);

  /**
   * @brief Get the current orbit center
   * @return The current orbit center type
   */
  OrbitCenter getOrbitCenter() const;

  /**
   * @brief Set a custom orbit center point
   * @param center The custom orbit center coordinates
   */
  void setCustomOrbitCenter(const glm::vec3 &center);

  /**
   * @brief Get the custom orbit center point
   * @return The custom orbit center coordinates
   */
  glm::vec3 getCustomOrbitCenter() const;

  /**
   * @brief Set the current view preset
   * @param preset The view preset to set
   */
  void setCurrentViewPreset(ViewPreset preset);

  /**
   * @brief Get the current view preset
   * @return The current view preset
   */
  ViewPreset getCurrentViewPreset() const;

  /**
   * @brief Set whether a view transition is in progress
   * @param transitioning true if a transition is in progress
   */
  void setIsTransitioning(bool transitioning);

  /**
   * @brief Check if a view transition is in progress
   * @return true if a transition is in progress
   */
  bool isTransitioning() const;

  /**
   * @brief Set callback for view preset change notifications
   * @param callback Function to invoke when view preset changes
   */
  void setViewPresetChangedCallback(NavigationCallback callback);

  /**
   * @brief Set callback for orbit center change notifications
   * @param callback Function to invoke when orbit center changes
   */
  void setOrbitCenterChangedCallback(NavigationCallback callback);

  // ==========================================================================
  // Phase 6: Precision & Snapping Methods
  // These methods provide access to grid and snap settings
  // ==========================================================================

  /**
   * @brief Callback type for grid settings change notifications
   */
  using GridSettingsCallback = std::function<void()>;

  /**
   * @brief Callback type for snap settings change notifications
   */
  using SnapSettingsCallback = std::function<void()>;

  /**
   * @brief Callback type for figure change notifications
   * Used for cache invalidation in SnapManager
   */
  using FigureChangedCallback = std::function<void()>;

  /**
   * @brief Callback type for camera change notifications
   * Used for cache invalidation in SnapManager
   */
  using CameraChangedCallback = std::function<void()>;

  /**
   * @brief Callback type for grid geometry dirty notifications
   * @details Called when grid settings change that require grid geometry
   * regeneration
   */
  using GridGeometryDirtyCallback = std::function<void()>;

  /**
   * @brief Get the current grid settings
   * @return Current grid settings
   */
  GridSettings getGridSettings() const;

  /**
   * @brief Set grid settings
   * @param settings The new grid settings
   *
   * Triggers OnGridSettingsChanged FSM event and notifies listeners.
   */
  void setGridSettings(const GridSettings &settings);

  /**
   * @brief Get the current snap settings
   * @return Current snap settings
   */
  SnapSettings getSnapSettings() const;

  /**
   * @brief Set snap settings
   * @param settings The new snap settings
   *
   * Triggers OnSnapSettingsChanged FSM event and notifies listeners.
   */
  void setSnapSettings(const SnapSettings &settings);

  /**
   * @brief Set callback for grid settings change notifications
   * @param callback Function to invoke when grid settings change
   */
  void setGridSettingsChangedCallback(GridSettingsCallback callback);

  /**
   * @brief Set callback for snap settings change notifications
   * @param callback Function to invoke when snap settings change
   */
  void setSnapSettingsChangedCallback(SnapSettingsCallback callback);

  /**
   * @brief Set callback for figure change notifications
   * @param callback Function to invoke when figures are added/removed/modified
   *
   * Used by SnapManager for cache invalidation.
   */
  void setFigureChangedCallback(FigureChangedCallback callback);

  /**
   * @brief Set callback for camera change notifications
   * @param callback Function to invoke when camera zooms/pans/orbits
   *
   * Used by SnapManager for cache invalidation.
   */
  void setCameraChangedCallback(CameraChangedCallback callback);

  // ==========================================================================
  // Grid State Query Methods (for GridManager)
  // These methods provide access to individual grid settings properties
  // ==========================================================================

  /**
   * @brief Check if grid is enabled
   * @return true if grid is visible
   */
  bool isGridEnabled() const;

  /**
   * @brief Get the major grid spacing
   * @return Spacing between major grid lines
   */
  float getGridMajorSpacing() const;

  /**
   * @brief Get the minor grid spacing
   * @return Spacing between minor grid lines
   */
  float getGridMinorSpacing() const;

  /**
   * @brief Get the major grid color
   * @return Color of major grid lines (RGBA)
   */
  glm::vec4 getGridMajorColor() const;

  /**
   * @brief Get the minor grid color
   * @return Color of minor grid lines (RGBA)
   */
  glm::vec4 getGridMinorColor() const;

  /**
   * @brief Get the grid opacity
   * @return Grid opacity (0.0 to 1.0)
   */
  float getGridOpacity() const;

  /**
   * @brief Check if axes are shown
   * @return true if X, Y, Z axes are visible
   */
  bool getGridShowAxes() const;

  /**
   * @brief Check if origin is shown
   * @return true if origin point is visible
   */
  bool getGridShowOrigin() const;

  /**
   * @brief Check if minor lines are shown
   * @return true if minor grid lines are visible
   */
  bool getGridShowMinorLines() const;

  /**
   * @brief Get the number of minor divisions
   * @return Number of minor divisions between major lines
   */
  int getGridMinorDivisions() const;

  /**
   * @brief Sets callback for grid geometry dirty notifications
   * @param callback Function to call when grid geometry becomes dirty
   * @details GridManager registers this callback to know when to regenerate
   * grid geometry
   */
  void setGridGeometryDirtyCallback(GridGeometryDirtyCallback callback);

  // ==========================================================================
  // Snap State Query Methods (for SnapManager)
  // These methods provide access to individual snap settings properties
  // ==========================================================================

  /**
   * @brief Check if grid snap is enabled
   * @return true if grid snap is enabled
   */
  bool isSnapGridEnabled() const;

  /**
   * @brief Check if endpoint snap is enabled
   * @return true if endpoint snap is enabled
   */
  bool isSnapEndpointEnabled() const;

  /**
   * @brief Check if midpoint snap is enabled
   * @return true if midpoint snap is enabled
   */
  bool isSnapMidpointEnabled() const;

  /**
   * @brief Check if center snap is enabled
   * @return true if center snap is enabled
   */
  bool isSnapCenterEnabled() const;

  /**
   * @brief Check if intersection snap is enabled
   * @return true if intersection snap is enabled
   */
  bool isSnapIntersectionEnabled() const;

  /**
   * @brief Check if nearest point snap is enabled
   * @return true if nearest point snap is enabled
   */
  bool isSnapNearestEnabled() const;

  /**
   * @brief Check if tangent snap is enabled
   * @return true if tangent snap is enabled
   */
  bool isSnapTangentEnabled() const;

  /**
   * @brief Check if perpendicular snap is enabled
   * @return true if perpendicular snap is enabled
   */
  bool isSnapPerpendicularEnabled() const;

  /**
   * @brief Get snap tolerance
   * @return Snap tolerance in pixels
   */
  float getSnapTolerance() const;

  /**
   * @brief Check if snap indicators should be shown
   * @return true if snap indicators should be shown
   */
  bool getSnapShowIndicators() const;

  /**
   * @brief Get snap indicator color
   * @return Snap indicator color (RGBA)
   */
  glm::vec4 getSnapIndicatorColor() const;

  // ==========================================================================
  // Figure Grouping Methods (STUB - Not fully implemented)
  // These methods are stubs to allow compilation of GroupFiguresCommand
  // and UngroupFiguresCommand. Full implementation is pending.
  // ==========================================================================

  /**
   * @brief Group multiple figures together
   * @param figureIds The IDs of the figures to group
   * @return The ID of the created group, or 0 if grouping failed
   * @note This is a stub method that returns 0. Full implementation pending.
   */
  uint32_t groupFigures(const std::vector<uint32_t> &figureIds);

  /**
   * @brief Ungroup a group of figures
   * @param groupId The ID of the group to ungroup
   * @return Vector of released figure IDs, or empty if ungrouping failed
   * @note This is a stub method that returns empty vector. Full implementation
   * pending.
   */
  std::vector<uint32_t> ungroupFigures(uint32_t groupId);

  // ==========================================================================
  // Phase 3: Notification Callbacks
  // These callbacks notify panels of state changes
  // ==========================================================================

  /**
   * @brief Callback type for selection change notifications
   */
  using SelectionChangedCallback =
      std::function<void(const std::vector<uint32_t> &)>;

  /**
   * @brief Callback type for property change notifications
   */
  using PropertyChangedCallback =
      std::function<void(uint32_t, const std::string &)>;

  /**
   * @brief Set callback for selection change notifications
   * @param callback Function to invoke when selection changes
   */
  void setSelectionChangedCallback(SelectionChangedCallback callback);

  /**
   * @brief Set callback for property change notifications
   * @param callback Function to invoke when a property changes
   */
  void setPropertyChangedCallback(PropertyChangedCallback callback);

private:
  /// Reference to the finite state machine
  fsm::Machine &fsm_;

  /// Reference to the camera controller
  CameraController &cameraController_;

  /// Logger for diagnostic output
  std::shared_ptr<spdlog::logger> logger_;

  /// Callback for showing plane selection UI
  ShowPlaneSelectionCallback showPlaneSelectionCallback_;

  /// Callback for updating status bar
  UpdateStatusCallback updateStatusCallback_;

  /// Current sketch plane
  std::unique_ptr<model::SketchPlane> currentSketchPlane_;

  /// Current FSM state
  fsm::State currentState_;

  // ==========================================================================
  // Phase 2: Tool State Storage
  // These member variables store tool state that cannot be stored in FSM
  // because FSMConfig's VariableValue only supports simple types (int, float,
  // string, bool), not complex types like std::map or std::vector<glm::vec3>.
  // ==========================================================================

  /// Currently active tool ID (e.g., "Line3D", "Circle3D")
  std::string activeTool_;

  /// Options for the active tool (e.g., radius, sides, creation method)
  std::map<std::string, std::any> toolOptions_;

  /// Points collected during the current drawing operation
  std::vector<glm::vec3> collectedPoints_;

  // ==========================================================================
  // Phase 3: Selection State Storage
  // These member variables store selection state that cannot be stored in FSM
  // because FSMConfig's VariableValue only supports simple types (int, float,
  // string, bool), not complex types like std::vector<uint32_t>.
  // ==========================================================================

  /// IDs of selected figures
  std::vector<uint32_t> selectedFigureIds_;

  /// Index of primary selection in selectedFigureIds_ (-1 if no selection)
  int primarySelectionIndex_;

  /// Callback for selection change notifications
  SelectionChangedCallback selectionChangedCallback_;

  /// Callback for property change notifications
  PropertyChangedCallback propertyChangedCallback_;

  // ==========================================================================
  // Phase 4: Command History Storage
  // UIFSMAdapter is the single source of truth for command history
  // ==========================================================================

  /// Vector of command objects (actual commands, not just descriptions)
  std::vector<std::unique_ptr<ICommand>> commandHistory_;

  /// Current position in command history (index of last executed command)
  size_t currentCommandIndex_{0};

  /// Maximum history size (for memory management)
  static constexpr size_t MAX_HISTORY_SIZE = 1000;

  // ==========================================================================
  // Phase 5: Navigation State Storage
  // These member variables store navigation state that cannot be stored in FSM
  // because FSMConfig's VariableValue only supports simple types (int, float,
  // string, bool), not complex types like glm::vec3 or enum classes.
  // ==========================================================================

  /// Current orbit center for 3D navigation
  OrbitCenter orbitCenter_ = OrbitCenter::Origin;

  /// Custom orbit center point (used when orbitCenter_ is Custom)
  glm::vec3 customOrbitCenter_{0.0f, 0.0f, 0.0f};

  /// Current view preset
  ViewPreset currentViewPreset_ = ViewPreset::Top2D;

  /// Whether a view transition is currently in progress
  bool isTransitioning_ = false;

  /// Callback for view preset change notifications
  NavigationCallback onViewPresetChanged_;

  /// Callback for orbit center change notifications
  NavigationCallback onOrbitCenterChanged_;

  // ==========================================================================
  // Phase 6: Grid and Snap Settings Storage
  // These member variables store grid and snap settings that cannot be stored
  // in FSM because FSMConfig's VariableValue only supports simple types.
  // ==========================================================================

  /// Grid settings for visualization and snapping
  GridSettings gridSettings_;

  /// Snap settings for precision drawing
  SnapSettings snapSettings_;

  /// Callback for grid settings change notifications
  GridSettingsCallback onGridSettingsChanged_;

  /// Callback for snap settings change notifications
  SnapSettingsCallback onSnapSettingsChanged_;

  /// Callback for figure change notifications (used by SnapManager)
  FigureChangedCallback onFigureChanged_;

  /// Callback for camera change notifications (used by SnapManager)
  CameraChangedCallback onCameraChanged_;

  /// Callback to notify when grid geometry becomes dirty
  /// Triggered when grid settings change (spacing, divisions, etc.)
  GridGeometryDirtyCallback onGridGeometryDirty_;

  /**
   * @brief Get available sketch planes
   * @return Vector of preset sketch planes
   */
  std::vector<model::SketchPlane> getAvailablePlanes() const;
};

} // namespace view
