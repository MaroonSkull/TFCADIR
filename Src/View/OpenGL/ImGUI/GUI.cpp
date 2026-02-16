#include "imgui.h"
#include <Controller/OpenGL/ImGUI.hpp>
#include <GUI.hpp>
#include <Logging/LoggerManager.hpp>
#include <View/Commands/ExtendedCommandManager.hpp>
#include <View/Commands/ImGUI/CommandHistoryPanel.hpp>
#include <View/ImGUI/ViewPresetsPanel.hpp>
#include <View/ImportExport/ImportExportDialog.hpp>
#include <View/ImportExport/ImportExportManager.hpp>
#include <View/LayerManager/Layer.hpp>
#include <View/LayerManager/LayerManagerPanel.hpp>
#include <View/Navigation/NavigationEventHandler.hpp>
#include <View/Navigation/NavigationManager.hpp>
#include <View/ObjectManagement/ImGUI/OutlinerPanel.hpp>
#include <View/ObjectManagement/ImGUI/PropertyInspectorPanel.hpp>
#include <View/ObjectManagement/SelectionManager.hpp>
#include <View/ObjectManagement/SelectionTypes.hpp>
#include <View/OpenGL/ImGUI/CanvasContextMenu.hpp>
#include <View/OpenGL/ImGUI/ContextMenu.hpp>
#include <View/OpenGL/ImGUI/ObjectContextMenu.hpp>
#include <View/OpenGL/ImGUI/PropertiesPanel.hpp>
#include <View/OpenGL/ImGUI/ShortcutDialog.hpp>
#include <View/Polish/ShortcutManager.hpp>
#include <View/Precision/CoordinateInputWidget.hpp>
#include <View/Precision/GridSettingsPanel.hpp>
#include <View/Precision/MeasurementDisplay.hpp>
#include <View/Precision/MeasurementManager.hpp>
#include <View/Precision/SnapSettingsPanel.hpp>
#include <View/Settings/DisplaySettings.hpp>
#include <View/Settings/GridSettings.hpp>
#include <View/Settings/SettingsManager.hpp>
#include <View/Shortcuts/ShortcutManager.hpp>
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

  // Phase 14: File menu with Import/Export
  if (BeginMenu("File")) {
    if (MenuItem("Import...")) {
      if (importExportDialog_) {
        importExportDialog_->openImport();
      }
    }
    if (MenuItem("Export...")) {
      if (importExportDialog_) {
        // Export selected figures or all figures
        std::vector<std::shared_ptr<model::IFigure>> figuresToExport;
        if (selectionManager_ &&
            !selectionManager_->getSelectedFigureIds().empty()) {
          // TODO: Get selected figures from model
        }
        importExportDialog_->openExport(figuresToExport);
      }
    }
    Separator();
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
    Separator();
    if (MenuItem("Keyboard Shortcuts...", "F9")) {
      if (shortcutDialog_) {
        shortcutDialog_->open();
      }
    }
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
    // Phase 15: Grid Settings menu item
    if (MenuItem("Grid Settings...")) {
      if (gridSettingsDialog_) {
        gridSettingsDialog_->Open();
      }
    }
    // Phase 15: Display Settings menu item
    if (MenuItem("Display Settings...")) {
      if (displaySettingsDialog_) {
        displaySettingsDialog_->Open();
      }
    }
    Separator();
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

  // Phase 17: Help menu
  if (BeginMenu("Help")) {
    if (MenuItem("Documentation", "F1")) {
      if (helpDialog_) {
        helpDialog_->open();
      }
    }
    if (MenuItem("Keyboard Shortcuts")) {
      if (helpDialog_) {
        helpDialog_->open();
        helpDialog_->switchToTab(1); // Switch to Shortcuts tab
      }
    }
    if (MenuItem("Tutorials")) {
      if (helpDialog_) {
        helpDialog_->open();
        helpDialog_->switchToTab(2); // Switch to Tutorials tab
      }
    }
    Separator();
    if (MenuItem("About TFCADIR")) {
      // TODO: Show about dialog
    }
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

    // Phase 6: Split snap settings dock to create space for coordinate input
    dockIdCoordinateInput_ =
        DockBuilderSplitNode(dockIdSnapSettings_, ImGuiDir_Down, 0.50f, nullptr,
                             &dockIdSnapSettings_);

    // Phase 10: Split coordinate input dock to create space for selection
    // settings
    dockIdSelectionSettings_ =
        DockBuilderSplitNode(dockIdCoordinateInput_, ImGuiDir_Down, 0.50f,
                             nullptr, &dockIdCoordinateInput_);

    // Phase 11: Split selection settings dock to create space for annotation
    // tools
    dockIdAnnotationTools_ =
        DockBuilderSplitNode(dockIdSelectionSettings_, ImGuiDir_Down, 0.50f,
                             nullptr, &dockIdSelectionSettings_);

    // Phase 12: Split annotation tools dock to create space for measurement
    // tools
    dockIdMeasurementTools_ =
        DockBuilderSplitNode(dockIdAnnotationTools_, ImGuiDir_Down, 0.50f,
                             nullptr, &dockIdAnnotationTools_);

    // Phase 13: Split measurement tools dock to create space for layer manager
    dockIdLayerManager_ =
        DockBuilderSplitNode(dockIdMeasurementTools_, ImGuiDir_Down, 0.50f,
                             nullptr, &dockIdMeasurementTools_);

    DockBuilderDockWindow("Canvas", centerId);
    DockBuilderDockWindow("Tools", dockIdTools_);
    DockBuilderDockWindow("Tool Options", dockIdTools_);
    DockBuilderDockWindow("Log", dockIdLog_);
    DockBuilderDockWindow("Outliner", dockIdOutliner_);
    DockBuilderDockWindow("Properties", dockIdProperties_);
    DockBuilderDockWindow("Command History", dockIdCommandHistory_);
    DockBuilderDockWindow("View Presets", dockIdViewPresets_);
    DockBuilderDockWindow("Grid Settings", dockIdGridSettings_);
    DockBuilderDockWindow("Snap Settings", dockIdSnapSettings_);
    DockBuilderDockWindow("Coordinate Input", dockIdCoordinateInput_);
    DockBuilderDockWindow("Selection Settings", dockIdSelectionSettings_);
    DockBuilderDockWindow("Annotation Tools", dockIdAnnotationTools_);
    DockBuilderDockWindow("Measurement Tools", dockIdMeasurementTools_);
    DockBuilderDockWindow("Layer Manager", dockIdLayerManager_);

    DockBuilderFinish(dockId_);
  }
}

void GUI::ShowLog() {
  if (logViewer_) {
    logViewer_->render();
  }
}

void GUI::ShowSidePanel() {
  // Render "Tools" window
  if (Begin("Tools")) {
    // Fallback: original simple tool buttons if ToolOptionsPanel not available
    if (!toolOptionsPanel_) {
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

  // Render "Tool Options" separately (no parent-child relationship with "Tools"
  // window)
  if (toolOptionsPanel_) {
    ImVec2 panelSize = ImVec2(0, 0); // Let ImGui auto-size the window
    toolOptionsPanel_->render(panelSize);
  }
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

    /// Render view preset buttons in the top-left corner of the canvas
    if (viewPresetButtons_) {
      ImVec2 canvasCursorPos = GetCursorPos();
      SetCursorPos(ImVec2(10.0f, 10.0f));
      viewPresetButtons_->render();
      SetCursorPos(canvasCursorPos);
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
  // Note: viewPresetButtons_ is now rendered inside ShowCanvas() to avoid
  // creating a floating "Debug" window
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
 * @brief Render the coordinate input widget
 *
 * Displays the coordinate input window for precise coordinate entry.
 */
void GUI::ShowCoordinateInputWidget() {
  if (coordinateInputWidget_) {
    coordinateInputWidget_->render();
  }
}

/**
 * @brief Render the measurement display
 *
 * Displays the measurement overlay for real-time measurement feedback.
 */
void GUI::ShowMeasurementDisplay() {
  if (measurementDisplay_ && measurementManager_) {
    // Update measurement data from manager
    const view::MeasurementResult &result =
        measurementManager_->getLastMeasurement();
    view::MeasurementData data;
    data.isValid = result.isValid;
    data.distance = result.distance;
    data.angle = result.angle;
    data.area = result.area;
    data.perimeter = result.perimeter;
    measurementDisplay_->setMeasurementData(data);
    measurementDisplay_->render();
  }
}

/**
 * @brief Render the enhanced properties panel
 *
 * Displays the context-aware properties panel that shows different properties
 * based on selection state (empty, single, multiple).
 */
void GUI::ShowPropertiesPanel() {
  if (propertiesPanel_) {
    propertiesPanel_->render();
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
          std::make_unique<view::ImGUI::ToolOptionsPanel>(*uiFSMAdapter_)) {
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

  /// Phase 6: Create coordinate input widget for precise coordinate entry
  coordinateInputWidget_ =
      std::make_unique<view::CoordinateInputWidget>(uiFSMAdapter_.get());

  /// Phase 6: Create measurement manager for measurement calculations
  measurementManager_ =
      std::make_unique<view::MeasurementManager>(*uiFSMAdapter_);

  /// Phase 6: Create measurement display for measurement overlay
  measurementDisplay_ = std::make_unique<view::MeasurementDisplay>(
      uiFSMAdapter_.get(), measurementManager_.get());

  /// Phase 13: Create layer manager for layer management
  layerManager_ = std::make_unique<view::LayerManager>();

  /// Phase 13: Create layer manager panel for layer management UI
  layerManagerPanel_ =
      std::make_unique<view::LayerManagerPanel>(*layerManager_, *model);

  /// Phase 14: Create import/export manager for file operations
  importExportManager_ =
      std::make_unique<view::import_export::ImportExportManager>();

  /// Phase 14: Create import/export dialog for import/export UI
  importExportDialog_ =
      std::make_unique<view::import_export::ImportExportDialog>(
          *importExportManager_);

  /// Phase 15: Create grid settings dialog for grid configuration
  gridSettingsDialog_ = std::make_unique<view::settings::GridSettings>();

  /// Phase 15: Create display settings dialog for display configuration
  displaySettingsDialog_ = std::make_unique<view::settings::DisplaySettings>();

  /// Phase 15: Load settings from file and apply
  view::settings::SettingsManager::Instance().LoadFromFile(
      "config/settings.yaml");
  gridConfig_ = view::settings::SettingsManager::Instance().GetGridConfig();
  displayConfig_ =
      view::settings::SettingsManager::Instance().GetDisplayConfig();

  /// Phase 17: Create help system for context-sensitive help
  helpSystem_ = std::make_shared<view::HelpSystem>();
  helpSystem_->initialize("config/help.yaml");

  /// Phase 17: Create help dialog for displaying help content
  helpDialog_ = std::make_unique<view::HelpDialog>(helpSystem_);

  /// Initialize log viewer with ImGUI sink from LoggerManager
  auto imgui_sink =
      TFCADIR::Logging::LoggerManager::instance().get_imgui_sink();
  if (imgui_sink) {
    logViewer_ = std::make_unique<TFCADIR::Logging::LogViewer>(
        std::shared_ptr<TFCADIR::Logging::ImGUI_sink_mt>(imgui_sink,
                                                         [](auto *) {}));
  }

  /// Phase 7: Create shortcut manager for keyboard shortcut handling (stateless
  /// coordinator)
  shortcutManager_ = std::make_unique<view::ShortcutManager>(*uiFSMAdapter_);

  /// Phase 7: Register default undo/redo shortcuts
  /// Register Ctrl+Z for undo
  shortcutManager_->registerShortcut("undo",
                                     {ImGuiKey_Z, view::KeyModifier::Ctrl},
                                     "commandManager.undo", "Undo last action");
  /// Register Ctrl+Y for redo
  shortcutManager_->registerShortcut(
      "redo", {ImGuiKey_Y, view::KeyModifier::Ctrl}, "commandManager.redo",
      "Redo last undone action");

  /// Phase 9.3: Create shortcut configuration manager for customization dialog
  shortcutConfigManager_ =
      std::make_shared<view::ShortcutConfigManager>("config/shortcuts.yaml");

  /// Phase 9.3: Create shortcut customization dialog
  shortcutDialog_ =
      std::make_unique<view::ShortcutDialog>(shortcutConfigManager_);

  /// Phase 9.4: Create view preset manager for camera positioning
  viewPresetManager_ =
      std::make_shared<view::ViewPresetManager>(*navigationManager_);

  /// Phase 9.4: Create view preset buttons widget
  viewPresetButtons_ =
      std::make_unique<view::ViewPresetButtons>(viewPresetManager_);

  /// Phase 9.4: Set up preset button click callback
  viewPresetButtons_->setOnPresetClicked(
      [this](view::ViewPreset preset) { applyViewPreset(preset); });

  /// Phase 9.5: Create enhanced properties panel with context-aware display
  propertiesPanel_ = std::make_unique<view::PropertiesPanel>(
      *uiFSMAdapter_, *selectionManager_, *model);

  /// Phase 9.6: Create canvas context menu for right-click on empty canvas
  /// space
  canvasContextMenu_ = std::make_unique<view::CanvasContextMenu>(
      *extendedCommandManager_, *navigationManager_, uiFSMAdapter_.get());

  /// Phase 9.6: Create object context menu for right-click on selected objects
  objectContextMenu_ = std::make_unique<view::ObjectContextMenu>(
      *extendedCommandManager_, selectionManager_.get(), uiFSMAdapter_.get());

  /// Phase 9.6: Set up canvas context menu callbacks
  canvasContextMenu_->setOnPan([this]() {
    spdlog::info("Pan operation initiated from context menu");
    // Pan is handled by mouse drag in navigation event handler
  });

  canvasContextMenu_->setOnZoomToFit([this]() {
    if (navigationManager_) {
      navigationManager_->zoomToFit();
    }
  });

  canvasContextMenu_->setOnZoomToSelection([this]() {
    if (navigationManager_) {
      navigationManager_->zoomToSelection();
    }
  });

  canvasContextMenu_->setOnOpenSnapSettings([this]() {
    spdlog::info("Snap settings requested from context menu");
    // Snap settings panel is always visible in the dock
  });

  canvasContextMenu_->setOnOpenGridSettings([this]() {
    spdlog::info("Grid settings requested from context menu");
    // Grid settings panel is always visible in the dock
  });

  /// Phase 9.6: Set up object context menu callbacks
  objectContextMenu_->setOnDuplicate([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Duplicate {} objects from context menu", ids.size());
    // TODO: Implement duplicate through command system
  });

  objectContextMenu_->setOnDelete([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Delete {} objects from context menu", ids.size());
    // TODO: Implement delete through command system
  });

  objectContextMenu_->setOnMove([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Move {} objects from context menu", ids.size());
    // TODO: Implement move operation through tool activation
  });

  objectContextMenu_->setOnRotate([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Rotate {} objects from context menu", ids.size());
    // TODO: Implement rotate operation through tool activation
  });

  objectContextMenu_->setOnScale([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Scale {} objects from context menu", ids.size());
    // TODO: Implement scale operation through tool activation
  });

  objectContextMenu_->setOnOpenProperties([this]() {
    spdlog::info("Properties dialog requested from context menu");
    // Properties panel is always visible in the dock
  });

  objectContextMenu_->setOnOpenLayers([this]() {
    spdlog::info("Layers dialog requested from context menu");
    // TODO: Implement layers dialog
  });

  objectContextMenu_->setOnBringToFront(
      [this](const std::vector<uint32_t> &ids) {
        spdlog::info("Bring {} objects to front from context menu", ids.size());
        // TODO: Implement z-order through command system
      });

  objectContextMenu_->setOnSendToBack([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Send {} objects to back from context menu", ids.size());
    // TODO: Implement z-order through command system
  });

  objectContextMenu_->setOnCreateCopy([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Create copy of {} objects from context menu", ids.size());
    // TODO: Implement copy through command system
  });

  objectContextMenu_->setOnMirror([this](const std::vector<uint32_t> &ids) {
    spdlog::info("Mirror {} objects from context menu", ids.size());
    // TODO: Implement mirror operation
  });

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

  // Handle keyboard shortcuts through ShortcutManager
  ImGuiIO &io = GetIO();

  // Determine the modifier key combination
  view::KeyModifier modifier = view::KeyModifier::None;
  if (io.KeyCtrl && io.KeyShift && io.KeyAlt) {
    modifier = view::KeyModifier::CtrlShiftAlt;
  } else if (io.KeyCtrl && io.KeyShift) {
    modifier = view::KeyModifier::CtrlShift;
  } else if (io.KeyCtrl && io.KeyAlt) {
    modifier = view::KeyModifier::CtrlAlt;
  } else if (io.KeyShift && io.KeyAlt) {
    modifier = view::KeyModifier::ShiftAlt;
  } else if (io.KeyCtrl) {
    modifier = view::KeyModifier::Ctrl;
  } else if (io.KeyShift) {
    modifier = view::KeyModifier::Shift;
  } else if (io.KeyAlt) {
    modifier = view::KeyModifier::Alt;
  }

  // Check for relevant key presses and handle through ShortcutManager
  // This includes undo (Ctrl+Z), redo (Ctrl+Y), and other registered shortcuts
  for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key) {
    if (IsKeyPressed(static_cast<ImGuiKey>(key))) {
      shortcutManager_->handleKeyPress(key, modifier);
    }
  }

  // Phase 17: Handle F1 key for context-sensitive help
  if (IsKeyPressed(ImGuiKey_F1)) {
    if (helpDialog_) {
      // Get current context from FSM state
      std::string context;
      if (sp_controller_) {
        auto currentState = sp_controller_->fsm_.get_current_state();
        if (currentState == fsm::State::SketchEdit) {
          context = "sketch_mode";
        } else if (currentState == fsm::State::PlaneSelection) {
          context = "plane_selection";
        } else {
          context = "general";
        }
      }
      helpDialog_->openWithContext(context);
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

  // Phase 6: Render coordinate input widget
  ShowCoordinateInputWidget();

  // Phase 6: Render measurement display
  ShowMeasurementDisplay();

  // Phase 9.5: Render enhanced properties panel
  ShowPropertiesPanel();

  // Phase 9.3: Render shortcut customization dialog
  if (shortcutDialog_) {
    shortcutDialog_->render();
  }

  // Phase 9.6: Handle context menus
  // Check for right-click on canvas to show context menu
  if (isCanvasHovered_ && IsMouseClicked(ImGuiMouseButton_Right)) {
    // Check if we have selected objects
    bool hasSelection =
        selectionManager_ && !selectionManager_->getSelectedFigureIds().empty();

    if (hasSelection) {
      // Show object context menu
      objectContextMenu_->setSelectedObjects(
          selectionManager_->getSelectedFigureIds());
      objectContextMenu_->show(mousePositionAbsolute_);
    } else {
      // Show canvas context menu
      canvasContextMenu_->show(mousePositionAbsolute_);
    }
  }

  // Render context menus
  if (canvasContextMenu_ && canvasContextMenu_->isVisible()) {
    canvasContextMenu_->render();
  }

  if (objectContextMenu_ && objectContextMenu_->isVisible()) {
    objectContextMenu_->render();
  }

  // Phase 9.4: Handle view preset keyboard shortcuts
  handleViewPresetShortcuts();

  // Phase 10: Render selection settings panel
  ShowSelectionSettingsPanel();

  // Phase 10: Handle selection keyboard shortcuts
  handleSelectionShortcuts();

  // Phase 11: Render annotation tools panel
  ShowAnnotationToolsPanel();

  // Phase 12: Render measurement tools panel
  ShowMeasurementToolsPanel();

  // Phase 13: Render layer manager panel
  ShowLayerManagerPanel();

  // Phase 14: Render import/export dialog
  ShowImportExportDialog();

  // Phase 15: Render grid settings dialog
  ShowGridSettingsDialog();

  // Phase 15: Render display settings dialog
  ShowDisplaySettingsDialog();

  // Phase 17: Render help dialog
  if (helpDialog_ && helpDialog_->isOpen()) {
    helpDialog_->render();
  }

  // SshowDemoWindow();
  Render();

  return (isCanvasHovered_)
             ? std::tuple<ImVec2, float, std::optional<ImVec2>>(
                   canvasSize, momentWheel_, mousePositionRelative_)
             : std::tuple<ImVec2, float, std::optional<ImVec2>>(
                   canvasSize, momentWheel_, std::nullopt);
}

/**
 * @brief Apply a view preset through the ViewPresetManager
 * @param preset The view preset to apply
 */
void GUI::applyViewPreset(view::ViewPreset preset) {
  if (viewPresetManager_) {
    viewPresetManager_->applyPreset(preset);
    spdlog::info("Applied view preset: {}",
                 view::ViewPresetManager::getPresetName(preset));
  }
}

/**
 * @brief Handle keyboard shortcuts for view presets (NumPad keys)
 */
void GUI::handleViewPresetShortcuts() {
  ImGuiIO &io = GetIO();

  // Only handle shortcuts when canvas is hovered
  if (!isCanvasHovered_) {
    return;
  }

  // NumPad 7: Top view (or Bottom with Ctrl)
  if (IsKeyPressed(ImGuiKey_Keypad7)) {
    if (io.KeyCtrl) {
      applyViewPreset(view::ViewPreset::Bottom);
    } else {
      applyViewPreset(view::ViewPreset::Top3D);
    }
  }

  // NumPad 1: Front view (or Back with Ctrl)
  if (IsKeyPressed(ImGuiKey_Keypad1)) {
    if (io.KeyCtrl) {
      applyViewPreset(view::ViewPreset::Back);
    } else {
      applyViewPreset(view::ViewPreset::Front);
    }
  }

  // NumPad 3: Right view (or Left with Ctrl)
  if (IsKeyPressed(ImGuiKey_Keypad3)) {
    if (io.KeyCtrl) {
      applyViewPreset(view::ViewPreset::Left);
    } else {
      applyViewPreset(view::ViewPreset::Right);
    }
  }

  // NumPad 5: Isometric view
  if (IsKeyPressed(ImGuiKey_Keypad5)) {
    applyViewPreset(view::ViewPreset::Isometric);
  }
}

/**
 * @brief Render the selection settings panel for Phase 10 advanced selection
 *
 * Displays the selection mode controls, filter options, and selection memory.
 */
void GUI::ShowSelectionSettingsPanel() {
  if (Begin("Selection Settings")) {
    // Selection mode section
    if (CollapsingHeader("Selection Mode", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Selection mode radio buttons
      int modeIndex = static_cast<int>(selectionSettings_.mode);
      if (RadioButton("Point", &modeIndex,
                      static_cast<int>(view::SelectionMode::Point))) {
        selectionSettings_.mode = view::SelectionMode::Point;
        spdlog::info("Selection mode changed to Point");
      }
      SameLine();
      if (RadioButton("Box", &modeIndex,
                      static_cast<int>(view::SelectionMode::Box))) {
        selectionSettings_.mode = view::SelectionMode::Box;
        spdlog::info("Selection mode changed to Box");
      }
      SameLine();
      if (RadioButton("Lasso", &modeIndex,
                      static_cast<int>(view::SelectionMode::Lasso))) {
        selectionSettings_.mode = view::SelectionMode::Lasso;
        spdlog::info("Selection mode changed to Lasso");
      }
      SameLine();
      if (RadioButton("Polygon", &modeIndex,
                      static_cast<int>(view::SelectionMode::Polygon))) {
        selectionSettings_.mode = view::SelectionMode::Polygon;
        spdlog::info("Selection mode changed to Polygon");
      }
    }

    Separator();

    // Selection filter section
    if (CollapsingHeader("Selection Filters", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Figure type filters
      Text("Figure Types:");
      auto &filter = selectionSettings_.filter;

      CheckboxFlags(
          "Triangles", reinterpret_cast<unsigned int *>(&filter.figureTypes),
          static_cast<unsigned int>(view::FigureTypeFilter::Triangle));
      SameLine();
      CheckboxFlags("Quads",
                    reinterpret_cast<unsigned int *>(&filter.figureTypes),
                    static_cast<unsigned int>(view::FigureTypeFilter::Quad));
      SameLine();
      CheckboxFlags("Circles",
                    reinterpret_cast<unsigned int *>(&filter.figureTypes),
                    static_cast<unsigned int>(view::FigureTypeFilter::Circle));

      CheckboxFlags("N-gons",
                    reinterpret_cast<unsigned int *>(&filter.figureTypes),
                    static_cast<unsigned int>(view::FigureTypeFilter::Ngon));
      SameLine();
      CheckboxFlags(
          "Bezier3", reinterpret_cast<unsigned int *>(&filter.figureTypes),
          static_cast<unsigned int>(view::FigureTypeFilter::CurveBezier3));
      SameLine();
      CheckboxFlags(
          "Bezier4", reinterpret_cast<unsigned int *>(&filter.figureTypes),
          static_cast<unsigned int>(view::FigureTypeFilter::CurveBezier4));

      Separator();

      // Visibility and lock filters
      Checkbox("Visible Only", &filter.visibleOnly);
      SameLine();
      Checkbox("Exclude Locked", &filter.excludeLocked);

      // Select all / none buttons for figure types
      Separator();
      if (Button("Select All Types")) {
        filter.figureTypes = view::FigureTypeFilter::All;
      }
      SameLine();
      if (Button("Clear Types")) {
        filter.figureTypes = view::FigureTypeFilter::None;
      }
    }

    Separator();

    // Selection memory section
    if (CollapsingHeader("Selection Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Show saved selection sets
      Text("Saved Selections: %zu", selectionMemory_.savedSelections.size());

      // Input for naming selection sets
      static char selectionName[64] = "";
      InputText("##SelectionName", selectionName, IM_ARRAYSIZE(selectionName));
      SameLine();
      if (Button("Save")) {
        if (selectionName[0] != '\0' && selectionManager_) {
          selectionMemory_.saveSelection(
              selectionName, selectionManager_->getSelectedFigureIds());
          spdlog::info("Saved selection set '{}' with {} figures",
                       selectionName,
                       selectionManager_->getSelectedFigureIds().size());
        }
      }

      // List saved selections
      if (BeginChild("SavedSelectionsList", ImVec2(0, 100), true)) {
        for (size_t i = 0; i < selectionMemory_.savedSelections.size(); ++i) {
          const auto &entry = selectionMemory_.savedSelections[i];
          PushID(static_cast<int>(i));
          if (Button(entry.name.c_str(), ImVec2(-FLT_MIN, 0))) {
            // Restore this selection
            if (selectionManager_) {
              selectionManager_->clearSelection();
              selectionManager_->applySelectionModifier(
                  entry.figureIds, view::SelectionModifier::None);
              spdlog::info("Restored selection set '{}' with {} figures",
                           entry.name, entry.figureIds.size());
            }
          }
          SameLine();
          if (Button("X", ImVec2(24, 0))) {
            selectionMemory_.deleteSelection(entry.id);
            spdlog::info("Deleted selection set '{}'", entry.name);
          }
          PopID();
        }
      }
      EndChild();
    }

    Separator();

    // Selection actions
    if (CollapsingHeader("Actions", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (Button("Invert Selection")) {
        if (selectionManager_) {
          selectionManager_->invertSelection(selectionSettings_.filter);
          spdlog::info("Selection inverted");
        }
      }
      SameLine();
      if (Button("Select All")) {
        if (selectionManager_) {
          selectionManager_->selectAll();
          spdlog::info("All figures selected");
        }
      }
      SameLine();
      if (Button("Clear Selection")) {
        if (selectionManager_) {
          selectionManager_->clearSelection();
          spdlog::info("Selection cleared");
        }
      }
    }
  }
  End();
}

/**
 * @brief Handle keyboard shortcuts for selection modes (Phase 10)
 */
void GUI::handleSelectionShortcuts() {
  ImGuiIO &io = GetIO();

  // Only handle shortcuts when canvas is hovered
  if (!isCanvasHovered_) {
    return;
  }

  // Ctrl+A: Select all
  if (io.KeyCtrl && IsKeyPressed(ImGuiKey_A)) {
    if (selectionManager_) {
      selectionManager_->selectAll();
      spdlog::info("Select all via Ctrl+A");
    }
  }

  // Ctrl+I: Invert selection
  if (io.KeyCtrl && IsKeyPressed(ImGuiKey_I)) {
    if (selectionManager_) {
      selectionManager_->invertSelection(selectionSettings_.filter);
      spdlog::info("Invert selection via Ctrl+I");
    }
  }

  // Escape: Clear selection or cancel polygon selection
  if (IsKeyPressed(ImGuiKey_Escape)) {
    if (selectionSettings_.mode == view::SelectionMode::Polygon &&
        !currentSelectionGeometry_.points.empty()) {
      // Cancel polygon selection
      currentSelectionGeometry_.points.clear();
      spdlog::info("Polygon selection cancelled");
    } else if (selectionManager_ &&
               !selectionManager_->getSelectedFigureIds().empty()) {
      // Clear selection
      selectionManager_->clearSelection();
      spdlog::info("Selection cleared via Escape");
    }
  }

  // Number keys for quick selection mode switching
  if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt) {
    if (IsKeyPressed(ImGuiKey_1)) {
      selectionSettings_.mode = view::SelectionMode::Point;
      spdlog::info("Selection mode: Point");
    }
    if (IsKeyPressed(ImGuiKey_2)) {
      selectionSettings_.mode = view::SelectionMode::Box;
      spdlog::info("Selection mode: Box");
    }
    if (IsKeyPressed(ImGuiKey_3)) {
      selectionSettings_.mode = view::SelectionMode::Lasso;
      spdlog::info("Selection mode: Lasso");
    }
    if (IsKeyPressed(ImGuiKey_4)) {
      selectionSettings_.mode = view::SelectionMode::Polygon;
      spdlog::info("Selection mode: Polygon");
    }
  }
}

/**
 * @brief Render the annotation tools panel for Phase 11 dimension tools
 *
 * Displays the annotation tool selection buttons and dimension style options.
 */
void GUI::ShowAnnotationToolsPanel() {
  if (Begin("Annotation Tools")) {
    // Tool selection section
    if (CollapsingHeader("Dimension Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Linear Dimension button
      if (Button("Linear", ImVec2(80, 0))) {
        activateAnnotationTool(view::DimensionTool::Type::LinearDimension);
      }
      if (IsItemHovered()) {
        SetTooltip("Create linear dimensions between two points");
      }
      SameLine();

      // Angular Dimension button
      if (Button("Angular", ImVec2(80, 0))) {
        activateAnnotationTool(view::DimensionTool::Type::AngularDimension);
      }
      if (IsItemHovered()) {
        SetTooltip("Create angular dimensions between two lines");
      }
      SameLine();

      // Radial Dimension button
      if (Button("Radial", ImVec2(80, 0))) {
        activateAnnotationTool(view::DimensionTool::Type::RadialDimension);
      }
      if (IsItemHovered()) {
        SetTooltip("Create radial dimensions for circles/arcs");
      }

      // Diameter Dimension button
      if (Button("Diameter", ImVec2(80, 0))) {
        activateAnnotationTool(view::DimensionTool::Type::DiameterDimension);
      }
      if (IsItemHovered()) {
        SetTooltip("Create diameter dimensions for circles/arcs");
      }
      SameLine();

      // Leader Line button
      if (Button("Leader", ImVec2(80, 0))) {
        activateAnnotationTool(view::DimensionTool::Type::LeaderLine);
      }
      if (IsItemHovered()) {
        SetTooltip("Create leader lines with text annotations");
      }
      SameLine();

      // Text Annotation button
      if (Button("Text", ImVec2(80, 0))) {
        activateAnnotationTool(view::DimensionTool::Type::TextAnnotation);
      }
      if (IsItemHovered()) {
        SetTooltip("Add text labels to the canvas");
      }
    }

    Separator();

    // Dimension style section
    if (CollapsingHeader("Dimension Style", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Arrow style selection
      Text("Arrow Style:");
      int arrowStyle = static_cast<int>(dimensionStyle_.arrowStyle);
      if (RadioButton("Filled##Arrow", &arrowStyle,
                      static_cast<int>(view::ArrowStyle::Filled))) {
        dimensionStyle_.arrowStyle = static_cast<view::ArrowStyle>(arrowStyle);
      }
      SameLine();
      if (RadioButton("Open##Arrow", &arrowStyle,
                      static_cast<int>(view::ArrowStyle::Open))) {
        dimensionStyle_.arrowStyle = static_cast<view::ArrowStyle>(arrowStyle);
      }
      SameLine();
      if (RadioButton("Closed##Arrow", &arrowStyle,
                      static_cast<int>(view::ArrowStyle::Closed))) {
        dimensionStyle_.arrowStyle = static_cast<view::ArrowStyle>(arrowStyle);
      }

      // Arrow size
      SliderFloat("Arrow Size", &dimensionStyle_.arrowSize, 2.0f, 20.0f,
                  "%.1f");

      // Text position selection
      Text("Text Position:");
      int textPos = static_cast<int>(dimensionStyle_.textPosition);
      if (RadioButton("Above##Text", &textPos,
                      static_cast<int>(view::TextPosition::AboveLine))) {
        dimensionStyle_.textPosition = static_cast<view::TextPosition>(textPos);
      }
      SameLine();
      if (RadioButton("On Line##Text", &textPos,
                      static_cast<int>(view::TextPosition::OnLine))) {
        dimensionStyle_.textPosition = static_cast<view::TextPosition>(textPos);
      }
      SameLine();
      if (RadioButton("Below##Text", &textPos,
                      static_cast<int>(view::TextPosition::BelowLine))) {
        dimensionStyle_.textPosition = static_cast<view::TextPosition>(textPos);
      }

      // Text alignment selection
      Text("Text Alignment:");
      int textAlign = static_cast<int>(dimensionStyle_.textAlignment);
      if (RadioButton("Start##Align", &textAlign,
                      static_cast<int>(view::TextAlignment::Start))) {
        dimensionStyle_.textAlignment =
            static_cast<view::TextAlignment>(textAlign);
      }
      SameLine();
      if (RadioButton("Center##Align", &textAlign,
                      static_cast<int>(view::TextAlignment::Center))) {
        dimensionStyle_.textAlignment =
            static_cast<view::TextAlignment>(textAlign);
      }
      SameLine();
      if (RadioButton("End##Align", &textAlign,
                      static_cast<int>(view::TextAlignment::End))) {
        dimensionStyle_.textAlignment =
            static_cast<view::TextAlignment>(textAlign);
      }

      // Text size
      SliderFloat("Text Size", &dimensionStyle_.textSize, 8.0f, 32.0f, "%.0f");

      // Line weight
      SliderFloat("Line Weight", &dimensionStyle_.lineWeight, 0.5f, 5.0f,
                  "%.1f");

      // Extension line offset
      SliderFloat("Extension Offset", &dimensionStyle_.extensionLineOffset,
                  0.0f, 20.0f, "%.1f");

      // Precision
      SliderInt("Precision", &dimensionStyle_.precision, 0, 6, "%d");
    }

    Separator();

    // Active tool info section
    if (CollapsingHeader("Active Tool", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (activeAnnotationTool_) {
        Text("Tool: %s", activeAnnotationTool_->getDisplayName().c_str());
        TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Active");

        // Show tool-specific info
        Text("Points: %zu / %zu",
             activeAnnotationTool_->getCollectedPointsCount(),
             activeAnnotationTool_->getRequiredPoints());

        // Show formatted value if complete
        if (activeAnnotationTool_->isComplete()) {
          Text("Value: %s", activeAnnotationTool_->getFormattedValue().c_str());
        }

        // Cancel button
        if (Button("Cancel Tool", ImVec2(-FLT_MIN, 0))) {
          activeAnnotationTool_->cancel();
          activeAnnotationTool_.reset();
          spdlog::info("Annotation tool cancelled");
        }
      } else {
        TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No tool active");
        Text("Select a tool above to begin");
      }
    }
  }
  End();
}

/**
 * @brief Handle annotation tool activation (Phase 11)
 * @param toolType The type of annotation tool to activate
 */
void GUI::activateAnnotationTool(view::DimensionTool::Type toolType) {
  // Deactivate current tool if any
  if (activeAnnotationTool_) {
    activeAnnotationTool_->cancel();
    activeAnnotationTool_.reset();
  }

  // Create the appropriate tool using the factory
  if (uiFSMAdapter_) {
    activeAnnotationTool_ =
        view::DimensionToolFactory::createTool(toolType, *uiFSMAdapter_);

    if (activeAnnotationTool_) {
      // Apply current style
      activeAnnotationTool_->setStyle(dimensionStyle_);

      // Activate the tool
      activeAnnotationTool_->activate();

      spdlog::info("Activated {} tool",
                   view::DimensionToolFactory::getToolDisplayName(toolType));
    } else {
      spdlog::warn("Failed to create annotation tool type: {}",
                   static_cast<int>(toolType));
    }
  }
}

/**
 * @brief Render the measurement tools panel for Phase 12 measurement tools
 *
 * Displays the measurement tool selection buttons and measurement style
 * options.
 */
void GUI::ShowMeasurementToolsPanel() {
  if (Begin("Measurement Tools")) {
    // Tool selection section
    if (CollapsingHeader("Measurement Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Distance Measurement button
      if (Button("Distance", ImVec2(80, 0))) {
        activateMeasurementTool(view::MeasurementTool::Type::Distance);
      }
      if (IsItemHovered()) {
        SetTooltip("Measure distance between two points (M)");
      }
      SameLine();

      // Area Measurement button
      if (Button("Area", ImVec2(80, 0))) {
        activateMeasurementTool(view::MeasurementTool::Type::Area);
      }
      if (IsItemHovered()) {
        SetTooltip("Calculate area of polygon (A)");
      }
      SameLine();

      // Angle Measurement button
      if (Button("Angle", ImVec2(80, 0))) {
        activateMeasurementTool(view::MeasurementTool::Type::Angle);
      }
      if (IsItemHovered()) {
        SetTooltip("Measure angle between three points");
      }
      SameLine();

      // Coordinate Measurement button
      if (Button("Coordinate", ImVec2(80, 0))) {
        activateMeasurementTool(view::MeasurementTool::Type::Coordinate);
      }
      if (IsItemHovered()) {
        SetTooltip("Display point coordinates");
      }
      SameLine();

      // Length Measurement button
      if (Button("Length", ImVec2(80, 0))) {
        activateMeasurementTool(view::MeasurementTool::Type::Length);
      }
      if (IsItemHovered()) {
        SetTooltip("Measure length along path");
      }
    }

    Separator();

    // Measurement style section
    if (CollapsingHeader("Measurement Style", ImGuiTreeNodeFlags_DefaultOpen)) {
      // Precision
      SliderInt("Precision", &measurementStyle_.precision, 0, 6, "%d");

      // Unit selection
      Text("Unit:");
      int unit = static_cast<int>(measurementStyle_.unit);
      if (RadioButton("mm##Unit", &unit,
                      static_cast<int>(view::MeasurementUnit::Millimeters))) {
        measurementStyle_.unit = static_cast<view::MeasurementUnit>(unit);
      }
      SameLine();
      if (RadioButton("cm##Unit", &unit,
                      static_cast<int>(view::MeasurementUnit::Centimeters))) {
        measurementStyle_.unit = static_cast<view::MeasurementUnit>(unit);
      }
      SameLine();
      if (RadioButton("m##Unit", &unit,
                      static_cast<int>(view::MeasurementUnit::Meters))) {
        measurementStyle_.unit = static_cast<view::MeasurementUnit>(unit);
      }
      SameLine();
      if (RadioButton("in##Unit", &unit,
                      static_cast<int>(view::MeasurementUnit::Inches))) {
        measurementStyle_.unit = static_cast<view::MeasurementUnit>(unit);
      }

      // Text size
      SliderFloat("Text Size", &measurementStyle_.textSize, 8.0f, 32.0f,
                  "%.0f");

      // Line color
      Text("Line Color:");
      ColorEdit4("##LineColor", &measurementStyle_.lineColor.x,
                 ImGuiColorEditFlags_NoInputs);

      // Show preview checkbox
      Checkbox("Show Preview", &measurementStyle_.showPreview);
    }

    Separator();

    // Active tool info section
    if (CollapsingHeader("Active Tool", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (activeMeasurementTool_) {
        Text("Tool: %s", activeMeasurementTool_->getDisplayName().c_str());
        TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Active");

        // Show tool-specific info
        Text("Points: %zu", activeMeasurementTool_->getCollectedPointsCount());

        // Show formatted value if complete
        if (activeMeasurementTool_->isComplete()) {
          Text("Value: %s",
               activeMeasurementTool_->getFormattedValue().c_str());
        }

        // Cancel button
        if (Button("Cancel Tool", ImVec2(-FLT_MIN, 0))) {
          activeMeasurementTool_->cancel();
          activeMeasurementTool_.reset();
          spdlog::info("Measurement tool cancelled");
        }
      } else {
        TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No tool active");
        Text("Select a tool above to begin");
      }
    }
  }
  End();
}

/**
 * @brief Handle measurement tool activation (Phase 12)
 * @param toolType The type of measurement tool to activate
 */
void GUI::activateMeasurementTool(view::MeasurementTool::Type toolType) {
  // Deactivate current tool if any
  if (activeMeasurementTool_) {
    activeMeasurementTool_->cancel();
    activeMeasurementTool_.reset();
  }

  // Create the appropriate tool using the factory
  if (uiFSMAdapter_) {
    activeMeasurementTool_ =
        view::MeasurementToolFactory::createTool(toolType, *uiFSMAdapter_);

    if (activeMeasurementTool_) {
      // Apply current style
      activeMeasurementTool_->setStyle(measurementStyle_);

      // Activate the tool
      activeMeasurementTool_->activate();

      spdlog::info("Activated {} tool",
                   view::MeasurementToolFactory::getTypeName(toolType));
    } else {
      spdlog::warn("Failed to create measurement tool type: {}",
                   static_cast<int>(toolType));
    }
  }
}

/**
 * @brief Render the layer manager panel for Phase 13 layer management
 *
 * Displays the layer management UI for creating, editing, and deleting layers.
 */
void GUI::ShowLayerManagerPanel() {
  if (layerManagerPanel_) {
    layerManagerPanel_->render();
  }
}

/**
 * @brief Render the import/export dialog for Phase 14 file operations
 *
 * Displays the import/export dialog when open.
 */
void GUI::ShowImportExportDialog() {
  if (importExportDialog_ && importExportDialog_->isOpen()) {
    importExportDialog_->render();
  }
}

/**
 * @brief Render the grid settings dialog for Phase 15 grid configuration
 *
 * Displays the grid settings dialog when open.
 */
void GUI::ShowGridSettingsDialog() {
  if (gridSettingsDialog_ && gridSettingsDialog_->IsOpen()) {
    gridSettingsDialog_->Render(gridConfig_);

    // Check if settings changed and apply
    if (gridSettingsDialog_->HasChanged()) {
      view::settings::SettingsManager::Instance().SetGridConfig(gridConfig_);
      view::settings::SettingsManager::Instance().SaveToFile(
          "config/settings.yaml");
      spdlog::info("Grid settings updated and saved");
    }
  }
}

/**
 * @brief Render the display settings dialog for Phase 15 display configuration
 *
 * Displays the display settings dialog when open.
 */
void GUI::ShowDisplaySettingsDialog() {
  if (displaySettingsDialog_ && displaySettingsDialog_->IsOpen()) {
    displaySettingsDialog_->Render(displayConfig_);

    // Check if settings changed and apply
    if (displaySettingsDialog_->HasChanged()) {
      view::settings::SettingsManager::Instance().SetDisplayConfig(
          displayConfig_);
      view::settings::DisplaySettings::ApplyTheme(displayConfig_);
      view::settings::SettingsManager::Instance().SaveToFile(
          "config/settings.yaml");
      spdlog::info("Display settings updated and saved");
    }
  }
}
