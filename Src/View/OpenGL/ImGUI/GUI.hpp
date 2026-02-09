#pragma once

#include <Controller/IController.hpp>
#include <View/CameraController.hpp>
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/Commands/ImGUI/CommandHistoryPanel.hpp>
#include <View/ImGUI/ViewPresetsPanel.hpp>
#include <View/Navigation/NavigationEventHandler.hpp>
#include <View/Navigation/NavigationManager.hpp>
#include <View/ObjectManagement/ImGUI/OutlinerPanel.hpp>
#include <View/ObjectManagement/ImGUI/PropertyInspectorPanel.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/Precision/CoordinateInputWidget.hpp>
#include <View/Precision/GridSettingsPanel.hpp>
#include <View/Precision/SnapSettingsPanel.hpp>
#include <View/Tools/ImGUI/CommandManager.hpp>
#include <View/Tools/ImGUI/ToolOptionsPanel.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glfwpp/glfwpp.h>
#include <imgui.h>
#include <imgui_internal.h> // for docking
#include <memory>
#include <optional>
#include <string>

class GUI {
private:
  std::shared_ptr<controller::IController> sp_controller_;
  ImGuiDockNodeFlags dockFlags_{ImGuiDockNodeFlags_None};
  ImGuiID dockId_{}; // id of the parent window to which all other panels will
                     // be attached
  ImGuiID dockIdTools_{};
  ImGuiID dockIdLog_{};
  ImGuiID dockIdMouse_{};
  ImGuiID dockIdOutliner_{};       // Phase 3: Outliner panel dock ID
  ImGuiID dockIdProperties_{};     // Phase 3: Property inspector dock ID
  ImGuiID dockIdCommandHistory_{}; // Phase 4: Command history panel dock ID
  ImGuiID dockIdViewPresets_{};    // Phase 5: View presets panel dock ID
  ImGuiID dockIdGridSettings_{};        // Phase 6: Grid settings panel dock ID
  ImGuiID dockIdSnapSettings_{};        // Phase 6: Snap settings panel dock ID
  ImGuiID dockIdCoordinateInput_{};      // Phase 6: Coordinate input widget dock ID
  ImVec2 mouseOverlayPosition_{};
  ImVec2 mousePositionAbsolute_{};
  ImTextureID textureId_{};
  ImGuiWindowFlags canvasFlags_{ImGuiWindowFlags_NoDecoration |
                                ImGuiWindowFlags_NoNav |
                                ImGuiWindowFlags_NoInputs};
  bool isCanvasHovered_{};         // current state of canvas
  ImVec2 mousePositionRelative_{}; // mouse coords in canvas space
  float momentWheel_{};            // momentum speed of scroll

  bool circleSegmentsOverride_{false};
  static const int circleSegmentsOverride_v_default = 15;
  int circleSegmentsOverride_v_{circleSegmentsOverride_v_default};

  bool curveSegmentsOverride_{false};
  static const int curveSegmentsOverride_v_default = 15;
  int curveSegmentsOverride_v_{curveSegmentsOverride_v_default};

  /// Camera controller for managing camera state during sketch mode
  std::unique_ptr<view::CameraController> cameraController_;

  /// UI-FSM adapter for bridging state changes to UI updates
  std::unique_ptr<view::UIFSMAdapter> uiFSMAdapter_;

  /// Phase 2: Tool options panel for tool selection and configuration
  std::unique_ptr<view::ImGUI::ToolOptionsPanel> toolOptionsPanel_;

  /// Phase 2: Command manager for undo/redo support
  std::unique_ptr<view::ImGUI::CommandManager> commandManager_;

  /// Phase 3: Selection manager for object selection operations
  std::unique_ptr<view::SelectionManager> selectionManager_;

  /// Phase 3: Outliner panel for scene hierarchy view
  std::unique_ptr<view::OutlinerPanel> outlinerPanel_;

  /// Phase 3: Property inspector panel for editing object properties
  std::unique_ptr<view::PropertyInspectorPanel> propertyInspectorPanel_;

  /// Phase 4: Extended command manager for typed commands (stateless, delegates
  /// to UIFSMAdapter)
  std::unique_ptr<view::ExtendedCommandManager> extendedCommandManager_;

  /// Phase 4: Command history panel for undo/redo visualization
  std::unique_ptr<view::CommandHistoryPanel> commandHistoryPanel_;

  /// Phase 5: Navigation manager for zoom, pan, orbit operations (stateless
  /// coordinator)
  std::unique_ptr<view::NavigationManager> navigationManager_;

  /// Phase 5: Navigation event handler for mouse and keyboard events
  std::unique_ptr<view::NavigationEventHandler> navigationEventHandler_;

  /// Phase 5: View presets panel for view preset selection
  std::unique_ptr<view::ViewPresetsPanel> viewPresetsPanel_;

  /// Phase 6: Grid settings panel for grid configuration
  std::unique_ptr<view::GridSettingsPanel> gridSettingsPanel_;

  /// Phase 6: Snap settings panel for snap configuration
  std::unique_ptr<view::SnapSettingsPanel> snapSettingsPanel_;

  /// Phase 6: Coordinate input widget for precise coordinate entry
  std::unique_ptr<view::CoordinateInputWidget> coordinateInputWidget_;

  /// Current status bar text
  std::string statusText_{"3D Mode"};

  void ShowMainMenuBar();
  void ShowDockSpace();
  void ShowLog();
  void ShowSidePanel();
  ImVec2 ShowCanvas(ImTextureID renderTexture);
  void ShowStatusBar();
  void ShowSketchPlaneOverlay();
  void ShowOutlinerPanel();
  void ShowPropertyInspectorPanel();
  void ShowCommandHistoryPanel(); // Phase 4: Command history panel
  void ShowViewPresetsPanel();    // Phase 5: View presets panel
  void ShowGridSettingsPanel();      // Phase 6: Grid settings panel
  void ShowSnapSettingsPanel();      // Phase 6: Snap settings panel
  void ShowCoordinateInputWidget();  // Phase 6: Coordinate input widget
  // void ShowListPanel()
  void ShowSimpleOverlay(); // let it float after the mouse and show its
                            // coordinates if within canvas
public:
  GUI(std::shared_ptr<controller::IController> sp_controller);
  std::tuple<ImVec2, float, std::optional<ImVec2>>
  DrawGUI(ImTextureID renderTexture);
};