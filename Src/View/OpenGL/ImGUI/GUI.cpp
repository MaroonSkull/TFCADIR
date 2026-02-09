#include "imgui.h"
#include <Controller/OpenGL/ImGUI.hpp>
#include <GUI.hpp>
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/Commands/ImGUI/CommandHistoryPanel.hpp>
#include <View/ImGUI/ViewPresetsPanel.hpp>
#include <View/Navigation/NavigationEventHandler.hpp>
#include <View/Navigation/NavigationManager.hpp>
#include <View/ObjectManagement/ImGUI/OutlinerPanel.hpp>
#include <View/ObjectManagement/ImGUI/PropertyInspectorPanel.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/Precision/GridSettingsPanel.hpp>
#include <View/Precision/SnapSettingsPanel.hpp>
#include <View/Tools/ImGUI/CommandManager.hpp>
#include <View/Tools/ImGUI/ToolOptionsPanel.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
using namespace ImGui;

/**
 * @brief Shows the main menu bar with sketch mode controls
 */
void GUI::ShowMainMenuBar() {
  if (!BeginMainMenuBar())
    throw std::runtime_error("Failed to create main menu bar!");

  if (BeginMenu("Menu")) {
    if (BeginMenu("Examples")) {
      if (MenuItem("1")) {
      }
      if (MenuItem("2")) {
      }
      if (MenuItem("3")) {
      }
      EndMenu();
    }
    EndMenu();
  }
  if (BeginMenu("Edit")) {
    if (MenuItem("Undo", "CTRL+Z")) {
    }
    if (MenuItem("Redo", "CTRL+Y", false, false)) {
    } // Disabled item
    EndMenu();
  }
  if (BeginMenu("Sketch")) {
    if (MenuItem("Enter Sketch Mode")) {
      spdlog::info("Entering sketch mode...");
      if (sp_controller_) {
        sp_controller_->fsm_.process_event(fsm::events::OnEnterSketchMode{});
      }
    }
    Separator();
    // Check if in SketchEdit state before enabling
    bool inSketchEdit =
        sp_controller_ &&
        sp_controller_->fsm_.get_current_state() == fsm::State::SketchEdit;
    if (MenuItem("Exit Sketch Mode", nullptr, false, inSketchEdit)) {
      spdlog::info("Exiting sketch mode...");
      if (sp_controller_) {
        sp_controller_->fsm_.process_event(fsm::events::OnExitSketchMode{});
      }
    }
    EndMenu();
  }
  if (BeginMenu("View")) {
    if (MenuItem("Grid")) {
    }
    if (MenuItem("Sized", NULL, false, false)) {
    } // Disabled item
    Separator();

    if (Checkbox("##circlesegmentoverride", &circleSegmentsOverride_))
      circleSegmentsOverride_v_ = circleSegmentsOverride_v_default;
    SameLine(0.0f, GetStyle().ItemInnerSpacing.x);
    circleSegmentsOverride_ |= SliderInt("Circle segments override",
                                         &circleSegmentsOverride_v_, 3, 40);
    if (Checkbox("##curvessegmentoverride", &curveSegmentsOverride_))
      curveSegmentsOverride_v_ = curveSegmentsOverride_v_default;
    SameLine(0.0f, GetStyle().ItemInnerSpacing.x);
    curveSegmentsOverride_ |=
        SliderInt("Curves segments override", &curveSegmentsOverride_v_, 3, 40);
    EndMenu();
  }
  EndMainMenuBar();
}

void GUI::ShowDockSpace() {
  dockId_ = DockSpaceOverViewport(0, GetMainViewport()); // move inside if
  static bool isInit{};
  if (!isInit) {
    isInit = true;
    DockBuilderRemoveNode(dockId_);
    dockId_ = DockBuilderAddNode(dockId_, dockFlags_);
    DockBuilderSetNodeSize(dockId_, GetMainViewport()->Size);

    dockIdLog_ =
        DockBuilderSplitNode(dockId_, ImGuiDir_Down, 0.15f, nullptr, &dockId_);
    dockIdTools_ =
        DockBuilderSplitNode(dockId_, ImGuiDir_Right, 0.25f, nullptr, &dockId_);

    // Phase 3: Split the remaining center node to create space for outliner
    ImGuiID centerId = dockId_;
    dockIdOutliner_ = DockBuilderSplitNode(centerId, ImGuiDir_Down, 0.25f,
                                           nullptr, &centerId);
    dockIdProperties_ = DockBuilderSplitNode(centerId, ImGuiDir_Right, 0.30f,
                                             nullptr, &centerId);

    // Phase 4: Split tools dock to create space for command history
    dockIdCommandHistory_ = DockBuilderSplitNode(dockIdTools_, ImGuiDir_Down,
                                                 0.3f, nullptr, &dockIdTools_);

    // Phase 5: Split tools dock to create space for view presets
    dockIdViewPresets_ = DockBuilderSplitNode(dockIdTools_, ImGuiDir_Down,
                                              0.25f, nullptr, &dockIdTools_);

    // Phase 6: Split view presets dock to create space for grid settings
    dockIdGridSettings_ = DockBuilderSplitNode(
        dockIdViewPresets_, ImGuiDir_Down, 0.50f, nullptr, &dockIdViewPresets_);

    // Phase 6: Split grid settings dock to create space for snap settings
    dockIdSnapSettings_ =
        DockBuilderSplitNode(dockIdGridSettings_, ImGuiDir_Down, 0.50f, nullptr,
                             &dockIdGridSettings_);

    DockBuilderDockWindow("Canvas", centerId);
    DockBuilderDockWindow("Tools", dockIdTools_);
    DockBuilderDockWindow("Log", dockIdLog_);
    DockBuilderDockWindow("Mouse coords", dockIdMouse_);
    DockBuilderDockWindow("Outliner", dockIdOutliner_);
    DockBuilderDockWindow("Properties", dockIdProperties_);
    DockBuilderDockWindow("Command History", dockIdCommandHistory_);
    DockBuilderDockWindow("View Presets", dockIdViewPresets_);
    DockBuilderDockWindow("Grid Settings", dockIdGridSettings_);
    DockBuilderDockWindow("Snap Settings", dockIdSnapSettings_);

    DockBuilderFinish(dockId_);
  }
}

void GUI::ShowLog() {
  if (Begin("Log")) {
    Text("Just logs here");
  }
  End();
}

void GUI::ShowSidePanel() {
  if (Begin("Tools")) {
    // Render the Phase 2 ToolOptionsPanel
    if (toolOptionsPanel_) {
      ImVec2 panelSize = GetContentRegionAvail();
      toolOptionsPanel_->render(panelSize);
    } else {
      // Fallback: original simple tool buttons if ToolOptionsPanel not
      // available
      Text("Side panel with tools");
      // todo: add buttons for addLine, addTriangleByCorners and addCircle
      if (Button("Add Line")) {
        // Code to handle adding a line
        spdlog::info("Line added!");
        sp_controller_->addLine();
      }
      SameLine();
      if (Button("Add Triangle by Corners")) {
        // Code to handle adding a triangle by corners
        spdlog::info("Triangle added by corners!");
      }
      SameLine();
      if (Button("Add Circle")) {
        // Code to handle adding a circle
        spdlog::info("Circle added!");
      }
    }
  }
  End();
}

ImVec2 GUI::ShowCanvas(ImTextureID renderTexture) {
  ImVec2 canvasSize{};
  ImVec2 screenPositionAbsolute{};
  if (Begin("Canvas", nullptr, canvasFlags_)) {
    // Using a Child allow to fill all the space of the window.
    // It also alows customization
    BeginChild("##Canvas", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoMove);
    // Allow capturing mouse wheel scroll event
    SetItemKeyOwner(ImGuiKey_MouseWheelY);
    momentWheel_ = GetIO().MouseWheel;
    // Get the size of the child (i.e. the whole draw size of the windows).
    canvasSize = GetWindowSize();

    // Because I use the texture from OpenGL, I need to invert the V from the
    // UV.
    Image(renderTexture, canvasSize, ImVec2(0, 1), ImVec2(1, 0));
    isCanvasHovered_ = IsItemHovered();
    mousePositionAbsolute_ = GetMousePos();
    screenPositionAbsolute = GetItemRectMin();
    mousePositionRelative_ =
        ImVec2(mousePositionAbsolute_.x - screenPositionAbsolute.x,
               mousePositionAbsolute_.y - screenPositionAbsolute.y);

    if (isCanvasHovered_) {
      // calculate cursor coordinates
    }

    EndChild();
  }
  End();
  return canvasSize;
}

void GUI::ShowSimpleOverlay() {
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_None | ImGuiWindowFlags_NoDecoration |
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
      ImGuiWindowFlags_NoNav;
  SetNextWindowBgAlpha(0.35f); // Transparent background
  ImVec2 overlayPos = mousePositionAbsolute_;
  overlayPos.x += 16;
  overlayPos.y += 24;
  SetNextWindowPos(overlayPos, ImGuiCond_Always);
  SetNextWindowViewport(GetMainViewport()->ID);

  if (Begin("Mouse coords", nullptr, window_flags)) {
    if (IsMousePosValid())
      Text("%.1f,%.1f", mousePositionRelative_.x, mousePositionRelative_.y);
    else
      Text("<invalid>");
  }
  End();
}

/**
 * @brief Shows status bar displaying current application mode
 */
void GUI::ShowStatusBar() {
  // Update status text based on current FSM state
  if (sp_controller_) {
    auto currentState = sp_controller_->fsm_.get_current_state();
    if (currentState == fsm::State::PlaneSelection) {
      statusText_ = "Plane Selection Mode";
    } else if (currentState == fsm::State::SketchEdit) {
      statusText_ = "Sketch Mode - 2D Drawing";
    } else {
      statusText_ = "3D Mode";
    }
  }

  // Create a status bar at the bottom of the viewport
  float statusBarHeight = GetFrameHeight() + GetStyle().ItemSpacing.y;
  SetNextWindowPos(ImVec2(0, GetMainViewport()->Size.y - statusBarHeight));
  SetNextWindowSize(ImVec2(GetMainViewport()->Size.x, statusBarHeight));
  SetNextWindowViewport(GetMainViewport()->ID);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoFocusOnAppearing;

  PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));
  PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  if (Begin("StatusBar", nullptr, window_flags)) {
    TextUnformatted(statusText_.c_str());
  }
  End();
  PopStyleVar(2);
}

/**
 * @brief Renders the Outliner panel for scene hierarchy
 */
void GUI::ShowOutlinerPanel() {
  if (outlinerPanel_) {
    outlinerPanel_->render();
  }
}

/**
 * @brief Renders the Property Inspector panel for object properties
 */
void GUI::ShowPropertyInspectorPanel() {
  if (propertyInspectorPanel_) {
    propertyInspectorPanel_->render();
  }
}

/**
 * @brief Renders the Command History panel for undo/redo visualization
 */
void GUI::ShowCommandHistoryPanel() {
  if (commandHistoryPanel_) {
    commandHistoryPanel_->render();
  }
}

/**
 * @brief Renders the View Presets panel for view preset selection
 */
void GUI::ShowViewPresetsPanel() {
  if (viewPresetsPanel_) {
    viewPresetsPanel_->render();
  }
}

/**
 * @brief Render the grid settings panel
 *
 * Displays the grid settings window for configuring grid visualization.
 */
void GUI::ShowGridSettingsPanel() {
  if (gridSettingsPanel_) {
    gridSettingsPanel_->render();
  }
}

/**
 * @brief Render the snap settings panel
 *
 * Displays the snap settings window for configuring snap modes and settings.
 */
void GUI::ShowSnapSettingsPanel() {
  if (snapSettingsPanel_) {
    snapSettingsPanel_->render();
  }
}

/**
 * @brief Shows sketch plane visualization overlay when in sketch mode
 */
void GUI::ShowSketchPlaneOverlay() {
  if (!sp_controller_)
    return;

  auto currentState = sp_controller_->fsm_.get_current_state();
  if (currentState != fsm::State::SketchEdit)
    return;

  // Display sketch plane information overlay in top-left corner of canvas
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize |
      ImGuiWindowFlags_NoFocusOnAppearing;

  SetNextWindowPos(ImVec2(10, 50), ImGuiCond_Always);
  PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
  PushStyleColor(ImGuiCol_WindowBg,
                 ImVec4(0.0f, 0.0f, 0.0f, 0.5f)); // Semi-transparent black

  if (Begin("SketchPlaneInfo", nullptr, window_flags)) {
    TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Sketch Plane Active");
    Separator();
    Text("Grid View: Orthographic");
    Text("2D Drawing Enabled");
    Text("Use drawing tools to sketch");
  }
  End();
  PopStyleColor();
  PopStyleVar();
}

GUI::GUI(std::shared_ptr<controller::IController> sp_controller)
    : sp_controller_(sp_controller),
      cameraController_(std::make_unique<view::CameraController>()),
      uiFSMAdapter_(std::make_unique<view::UIFSMAdapter>(
          sp_controller->fsm_, *cameraController_,
          spdlog::get("logger") ? spdlog::get("logger")
                                : spdlog::default_logger())),
      toolOptionsPanel_(
          std::make_unique<view::ImGUI::ToolOptionsPanel>(*uiFSMAdapter_)),
      commandManager_(std::make_unique<view::ImGUI::CommandManager>()) {
  // Get the model from the controller using dynamic_cast
  auto *openglController =
      dynamic_cast<controller::OpenglImguiController *>(sp_controller_.get());
  if (!openglController) {
    throw std::runtime_error(
        "Failed to cast controller to OpenglImguiController");
  }
  auto model = openglController->getModel();

  // Initialize Phase 3 components
  selectionManager_ =
      std::make_unique<view::SelectionManager>(*uiFSMAdapter_, *model);
  outlinerPanel_ = std::make_unique<view::OutlinerPanel>(
      *uiFSMAdapter_, *selectionManager_, *model);
  propertyInspectorPanel_ = std::make_unique<view::PropertyInspectorPanel>(
      *uiFSMAdapter_, *selectionManager_, *model);

  // Initialize Phase 4: Command system
  /// Create extended command manager with UIFSMAdapter (stateless, delegates to
  /// UIFSMAdapter)
  extendedCommandManager_ =
      std::make_unique<view::ExtendedCommandManager>(*uiFSMAdapter_, *model);
  /// Create command history panel and link to command manager for cache
  /// invalidation
  commandHistoryPanel_ =
      std::make_unique<view::CommandHistoryPanel>(*extendedCommandManager_);
  extendedCommandManager_->setCommandHistoryPanel(commandHistoryPanel_.get());

  // Initialize Phase 5: Navigation and Views system
  /// Create navigation manager (stateless coordinator, delegates to
  /// UIFSMAdapter)
  navigationManager_ = std::make_unique<view::NavigationManager>(
      *uiFSMAdapter_, *cameraController_,
      selectionManager_.get() /* optional for orbit center */);
  /// Create navigation event handler for mouse/keyboard events
  navigationEventHandler_ =
      std::make_unique<view::NavigationEventHandler>(*navigationManager_);
  /// Create view presets panel for view preset selection
  viewPresetsPanel_ =
      std::make_unique<view::ViewPresetsPanel>(*navigationManager_);

  /// Phase 6: Create grid settings panel for grid configuration
  gridSettingsPanel_ =
      std::make_unique<view::GridSettingsPanel>(uiFSMAdapter_.get());

  /// Phase 6: Create snap settings panel for snap configuration
  snapSettingsPanel_ =
      std::make_unique<view::SnapSettingsPanel>(uiFSMAdapter_.get());

  // Set up UIFSMAdapter callbacks
  uiFSMAdapter_->setUpdateStatusCallback(
      [this](const std::string &status) { statusText_ = status; });

  uiFSMAdapter_->setShowPlaneSelectionCallback(
      [this](std::function<void(int)> callback) {
        // Show plane selection dialog when in PlaneSelection state
        if (Begin("Select Sketch Plane", nullptr,
                  ImGuiWindowFlags_AlwaysAutoResize)) {
          Text("Select a plane for 2D sketching:");
          if (Button("YZ Plane (Right)")) {
            callback(0); // Index 0 = YZ/Right
            CloseCurrentPopup();
          }
          SameLine();
          if (Button("XZ Plane (Top)")) {
            callback(1); // Index 1 = XZ/Top
            CloseCurrentPopup();
          }
          SameLine();
          if (Button("XY Plane (Front)")) {
            callback(2); // Index 2 = XY/Front
            CloseCurrentPopup();
          }
          End();
        }
      });
}

std::tuple<ImVec2, float, std::optional<ImVec2>>
GUI::DrawGUI(ImTextureID renderTexture) {
  NewFrame();

  // Handle keyboard shortcuts for undo/redo
  ImGuiIO &io = GetIO();
  if (io.KeyCtrl && commandManager_) {
    if (IsKeyPressed(ImGuiKey_Z) && commandManager_->canUndo()) {
      commandManager_->undo();
      spdlog::info("Undo performed");
    }
    if (IsKeyPressed(ImGuiKey_Y) && commandManager_->canRedo()) {
      commandManager_->redo();
      spdlog::info("Redo performed");
    }
  }

  ShowMainMenuBar();
  ShowDockSpace();
  ShowLog();
  ShowSidePanel();
  auto canvasSize = ShowCanvas(renderTexture);
  if (isCanvasHovered_)
    ShowSimpleOverlay();
  ShowStatusBar();
  ShowSketchPlaneOverlay();

  // Phase 3: Render object management panels
  ShowOutlinerPanel();
  ShowPropertyInspectorPanel();

  // Phase 4: Render command history panel
  ShowCommandHistoryPanel();

  // Phase 5: Render view presets panel
  ShowViewPresetsPanel();

  // Phase 6: Render grid settings panel
  ShowGridSettingsPanel();

  // Phase 6: Render snap settings panel
  ShowSnapSettingsPanel();

  // SshowDemoWindow();
  Render();

  return (isCanvasHovered_)
             ? std::tuple<ImVec2, float, std::optional<ImVec2>>(
                   canvasSize, momentWheel_, mousePositionRelative_)
             : std::tuple<ImVec2, float, std::optional<ImVec2>>(
                   canvasSize, momentWheel_, std::nullopt);
}
