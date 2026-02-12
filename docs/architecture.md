# Architecture Documentation

## Table of Contents

- [System Overview](#system-overview)
- [Component Architecture](#component-architecture)
  - [Model Layer](#model-layer)
  - [Controller Layer](#controller-layer)
  - [View Layer](#view-layer)
- [Integration](#integration)
  - [MVC Communication](#mvc-communication)
  - [Data Flow](#data-flow)
  - [Event Propagation](#event-propagation)
- [Component Diagrams](#component-diagrams)
- [Design Decisions](#design-decisions)
- [Technology Stack](#technology-stack)
- [Development Guidelines](#development-guidelines)

---

## System Overview

### Project Architecture

TFCADIR is a **2D/3D CAD application** built using the **Model-View-Controller (MVC)** architectural pattern with a **Finite State Machine (FSM)** for state management. The application provides tools for creating and manipulating geometric figures (lines, circles, triangles, squares, polygons) with support for both 2D drafting and 3D modeling workflows.

### Technology Stack

| Component | Technology | Purpose |
|------------|-----------|---------|
| **Language** | C++20 | Modern C++ with RAII, smart pointers, move semantics |
| **Build System** | CMake 3.20+ | Cross-platform build configuration |
| **Windowing** | GLFWpp (C++ wrapper for GLFW) | Window and input management |
| **UI Framework** | ImGUI | Immediate mode GUI rendering |
| **Graphics** | OpenGL 4.5+ (via GLAD) | Hardware-accelerated rendering |
| **Math** | GLM | OpenGL Mathematics library (vec3, mat4) |
| **State Machine** | FSMConfig (dependency) | YAML-configurable FSM engine |
| **Configuration** | yaml-cpp | YAML parsing for FSM configuration |
| **Logging** | spdlog | Fast C++ logging library |
| **Testing** | GoogleTest (framework) | Unit testing framework |

### Design Philosophy

The architecture follows these core principles:

1. **Separation of Concerns**: MVC pattern isolates business logic (Model), user interaction (View), and application control (Controller)
2. **Interface-Based Design**: Abstract interfaces ([`IModel`](../Src/Model/IModel.hpp), [`IController`](../Src/Controller/IController.hpp), [`IView`](../Src/View/IView.hpp)) enable loose coupling and testability
3. **State-Driven Architecture**: FSMConfig manages application states and transitions declaratively
4. **Command Pattern**: Undo/redo functionality through command objects
5. **Observer Pattern**: Callbacks notify components of state changes

---

## Component Architecture

### Model Layer

The **Model layer** encapsulates business logic and data management for geometric figures and application state.

#### Core Interfaces

##### [`IFigure`](../Src/Model/IModel.hpp:18-59)

Abstract interface for all geometric entities:

```cpp
class IFigure {
public:
  virtual uint32_t getId() const = 0;
  virtual const std::string& getName() const = 0;
  virtual void setName(const std::string& name) = 0;
  virtual glm::vec3 getPosition() const = 0;
  virtual void setPosition(const glm::vec3& position) = 0;
  virtual std::pair<glm::vec3, glm::vec3> getBounds() const = 0;
};
```

**Responsibilities**:
- Unique identification of figures
- Position management in 3D space
- Bounding box calculation for hit testing
- Name management for UI display

##### [`ICamera`](../Src/Model/IModel.hpp:67-107)

Abstract camera interface for view transformations:

```cpp
class ICamera {
public:
  virtual void setPosition(const glm::vec3& position) = 0;
  virtual glm::vec3 getPosition() const = 0;
  virtual void setTarget(const glm::vec3& target) = 0;
  virtual glm::vec3 getTarget() const = 0;
  virtual void setUp(const glm::vec3& up) = 0;
  virtual glm::vec3 getUp() const = 0;
};
```

**Responsibilities**:
- Camera position and orientation management
- Target/look-at point management
- Support for both 2D (z=0) and 3D cameras

##### [`IModel`](../Src/Model/IModel.hpp:117-218)

Main model interface combining figures and camera:

```cpp
class IModel {
public:
  // Figure management
  virtual void addFigure(const glm::vec3& position) = 0;
  virtual bool removeFigure(uint32_t id) = 0;
  virtual std::shared_ptr<IFigure> getFigure(uint32_t id) = 0;
  virtual std::shared_ptr<IFigure> findFigureByCoords(
      const glm::vec3& coords, float delta) = 0;
  virtual bool updateFigurePosition(uint32_t id,
      const glm::vec3& newPosition) = 0;
  virtual size_t getFigureCount() const = 0;
  virtual void clearFigures() = 0;

  // Camera management
  virtual ICamera& getCamera() = 0;
  virtual const ICamera& getCamera() const = 0;

  // Transformation matrices
  virtual glm::mat4 getModelMatrix() const = 0;
  virtual void setModelMatrix(const glm::mat4& matrix) = 0;
  virtual glm::mat4 getViewMatrix() const = 0;
  virtual glm::mat4 getProjectionMatrix() const = 0;
  virtual void setProjectionMatrix(const glm::mat4& matrix) = 0;
};
```

**Responsibilities**:
- Figure lifecycle management (CRUD operations)
- Spatial queries (hit testing, bounds)
- Camera and transformation matrix management
- Interface for both 2D and 3D rendering

#### Concrete Implementations

##### [`FlatFigures`](../Src/Model/FlatFigure.cpp)

Concrete model implementation for 2D geometric figures:

**Features**:
- Stores collection of [`FlatFigure`](../Src/Model/FlatFigure.hpp) objects
- Manages camera instance
- Maintains transformation matrices (model, view, projection)
- Thread-safe figure operations
- Automatic ID generation for new figures

##### [`SketchPlane`](../Src/Model/SketchPlane.hpp)

Represents a 2D drafting plane in 3D space:

**Features**:
- Supports XY, XZ, and YZ planes
- Provides plane-specific coordinate systems
- Used in sketch mode for 2D drawing on specific planes

---

### Controller Layer

The **Controller layer** manages application logic, state transitions, and coordinates between Model and View.

#### Core Components

##### [`IController`](../Src/Controller/IController.hpp:23-48)

Abstract controller interface:

```cpp
class IController {
public:
  fsm::Machine fsm_;

  // Input state management
  virtual void updateLeftMouseButtonState(state::Button) = 0;
  virtual void updateWheelMouseButtonState(state::Button) = 0;
  virtual void updateRightMouseButtonState(state::Button) = 0;
  virtual void updateWorkspaceHoverState(state::Workspace) = 0;
  virtual void updateScreenspaceMousePosition(glm::vec2) = 0;
  virtual void updateScroll(float) = 0;

  // Figure creation commands
  virtual void addTriangleByCenter() = 0;
  virtual void addTriangleByCorners() = 0;
  virtual void addSquareByCenter() = 0;
  virtual void addSquareByCorners() = 0;
  virtual void addNgonByCenter() = 0;
  virtual void addCircleByCenter() = 0;
  virtual void addLine() = 0;
  virtual void removeFigure() = 0;
};
```

**Responsibilities**:
- Input event handling and state tracking
- Mouse button state management
- Workspace hover detection
- Figure creation command dispatching

##### [`Machine`](../Src/Controller/FSM.hpp:149-315)

Finite State Machine wrapper using FSMConfig library:

```cpp
class Machine {
private:
  std::unique_ptr<fsmconfig::StateMachine> fsm_;
  std::unique_ptr<controller::FigureCreator> currentCreator_;
  std::vector<glm::vec2> collectedPoints_;
  model::FlatFigures* model_;

public:
  template <typename Event> void process_event(const Event& event);
  std::string get_current_state_name() const;
  State get_current_state() const;
  void setModel(model::FlatFigures* model);
  fsmconfig::StateMachine* get_fsm();
};
```

**Responsibilities**:
- State transition management via FSMConfig
- Event-driven architecture
- Figure creation coordination
- Drawing operation state tracking

##### FSM States

The application FSM defines these states:

| State | Description |
|--------|-------------|
| `Idle` | Default state, waiting for user input |
| `DrawingProcessing` | Active figure drawing in progress |
| `MoveFirstPoint` | Moving first point of current figure |
| `PlaneSelection` | Selecting sketch plane (XY/XZ/YZ) |
| `SketchEdit` | Editing figures in 2D sketch mode |

##### FSM Events

Key events defined in [`fsm::events`](../Src/Controller/FSM.hpp:29-131) namespace:

**Drawing Events**:
- `OnMouseMove` - Mouse position change
- `OnAddTriangleByCenter` / `OnAddTriangleByCorners` - Triangle creation
- `OnAddSquareByCenter` / `OnAddSquareByCorners` - Square creation
- `OnAddNgonByCenter` - Polygon creation
- `OnAddCircleByCenter` - Circle creation
- `OnAddLine` - Line creation
- `OnFigureComplete` - Finish current figure
- `OnFigureCancel` - Cancel current figure

**Sketch Mode Events**:
- `OnEnterSketchMode` / `OnExitSketchMode` - Sketch mode toggle
- `OnPlaneSelected` - Plane selection (0=XY, 1=XZ, 2=YZ)
- Sketch-specific drawing events (prefixed with "InSketch")

**Cache Invalidation Events** (Phase 6):
- `OnFigureAdded` / `OnFigureRemoved` / `OnFigureModified` - Figure changes
- `OnSelectionChanged` - Selection state changes
- `OnCameraZoomed` / `OnCameraPanned` / `OnCameraOrbited` - Camera changes
- `OnGridSettingsChanged` - Grid configuration changes
- `OnSnapSettingsChanged` - Snap configuration changes

**Polish Events** (Phase 7):
- `OnShortcutRegistered` / `OnShortcutActivated` - Keyboard shortcuts
- `OnContextMenuRequested` - Context menu actions
- `OnThemeChanged` - Theme changes
- `OnTooltipChanged` - Tooltip updates
- `OnHelpRequested` - Help system access
- `OnPerformanceUpdate` - Performance metrics

##### [`FigureCreator`](../Src/Controller/FigureCreator.hpp)

Strategy pattern for figure creation:

**Responsibilities**:
- Abstracts figure creation process
- Collects points during drawing
- Validates figure geometry
- Creates finalized figure objects

---

### View Layer

The **View layer** handles user interface rendering, input processing, and command execution.

#### Core Interface

##### [`IView`](../Src/View/IView.hpp:8-12)

Abstract view interface:

```cpp
class IView {
public:
  virtual bool shouldClose() const = 0;
  virtual void draw() = 0;
};
```

**Responsibilities**:
- Window lifecycle management
- Rendering loop control
- Platform abstraction (GLFW/ImGUI)

#### UI Components

##### [`UIFSMAdapter`](../Src/View/UIFSMAdapter.hpp:760-2013)

Central adapter bridging FSM and UI:

**Responsibilities**:
- FSM state change notifications to UI
- Status bar updates
- Plane selection UI management
- Tool state queries and management
- Selection state management
- Command history (undo/redo)
- Navigation state (view presets, orbit center)
- Grid, snap, and measurement settings
- Theme, shortcut, tooltip, and help settings
- Performance monitoring

**Key Data Structures** (stored in FSM state):

| Structure | Purpose |
|-----------|---------|
| [`GridSettings`](../Src/View/UIFSMAdapter.hpp:48-88) | Grid visualization and snapping configuration |
| [`SnapSettings`](../Src/View/UIFSMAdapter.hpp:96-148) | Precision drawing snap modes and tolerance |
| [`MeasurementSettings`](../Src/View/UIFSMAdapter.hpp:156-184) | Measurement display configuration |
| [`CoordinateInputSettings`](../Src/View/UIFSMAdapter.hpp:203-225) | Precise coordinate entry behavior |
| [`ColorScheme`](../Src/View/UIFSMAdapter.hpp:249-423) | Complete ImGUI and CAD color palette |
| [`ThemeSettings`](../Src/View/UIFSMAdapter.hpp:431-450) | Theme preset and custom colors |
| [`Shortcut`](../Src/View/UIFSMAdapter.hpp:502-575) | Keyboard shortcut definition |
| [`ShortcutSettings`](../Src/View/UIFSMAdapter.hpp:583-595) | All keyboard shortcuts |
| [`TooltipConfig`](../Src/View/UIFSMAdapter.hpp:619-647) / [`TooltipSettings`](../Src/View/UIFSMAdapter.hpp:655-677) | Tooltip behavior |
| [`HelpTopic`](../Src/View/UIFSMAdapter.hpp:689-714) / [`HelpSettings`](../Src/View/UIFSMAdapter.hpp:722-741) | Help system content |

##### Command System

Located in [`View/Commands/`](../Src/View/Commands/):

**Interface**: [`ICommand`](../Src/View/Commands/ICommand.hpp)

```cpp
class ICommand {
public:
  virtual void execute() = 0;
  virtual void undo() = 0;
  virtual void redo() = 0;
  virtual std::string getDescription() const = 0;
};
```

**Concrete Commands**:
- [`CreateFigureCommand`](../Src/View/Commands/CreateFigureCommand.hpp) - Creates new figures
- [`MoveFigureCommand`](../Src/View/Commands/MoveFigureCommand.hpp) - Translates figures
- [`RotateFigureCommand`](../Src/View/Commands/RotateFigureCommand.hpp) - Rotates figures
- [`ScaleFigureCommand`](../Src/View/Commands/ScaleFigureCommand.hpp) - Scales figures
- [`ColorChangeCommand`](../Src/View/Commands/ColorChangeCommand.hpp) - Changes figure colors
- [`DeleteFiguresCommand`](../Src/View/Commands/DeleteFiguresCommand.hpp) - Removes figures
- [`DuplicateFigureCommand`](../Src/View/Commands/DuplicateFigureCommand.hpp) - Copies figures
- [`GroupFiguresCommand`](../Src/View/Commands/GroupFiguresCommand.hpp) / [`UngroupFiguresCommand`](../Src/View/Commands/UngroupFiguresCommand.hpp) - Grouping operations
- [`PropertyChangeCommand`](../Src/View/Commands/PropertyChangeCommand.hpp) - Property modifications
- [`MacroCommand`](../Src/View/Commands/MacroCommand.hpp) - Composite commands

**Manager**: [`ExtendedCommandManager`](../Src/View/Commands/ExtendedCommandManager.hpp)

**Responsibilities**:
- Command execution and history management
- Undo/redo stack management
- Macro command recording

##### Navigation

Located in [`View/Navigation/`](../Src/View/Navigation/):

**Components**:
- [`NavigationManager`](../Src/View/Navigation/NavigationManager.hpp) - View preset management
- [`NavigationEventHandler`](../Src/View/Navigation/NavigationEventHandler.hpp) - Input event handling
- [`CameraController`](../Src/View/CameraController.hpp) - Camera operations

**View Presets** (from [`NavigationTypes.hpp`](../Src/View/Navigation/NavigationTypes.hpp)):
- `Top2D` / `Bottom2D` - 2D orthogonal views
- `Front2D` / `Back2D` - 2D orthogonal views
- `Right2D` / `Left2D` - 2D orthogonal views
- `Iso3D` - Isometric 3D view
- `Perspective3D` - Perspective 3D view

**Orbit Centers**:
- `Origin` - Orbit around world origin
- `Selection` - Orbit around selected object
- `Custom` - User-defined orbit point

##### Object Management

Located in [`View/ObjectManagement/`](../Src/View/ObjectManagement/):

**Components**:
- [`SelectionManager`](../Src/View/ObjectManagement/SelectionManager.hpp) - Multi-selection handling
- [`CanvasHitTester`](../Src/View/ObjectManagement/CanvasHitTester.hpp) - Spatial hit testing

**Responsibilities**:
- Figure selection (single, multi-select, toggle)
- Primary selection management
- Selection change notifications
- Mouse-to-world coordinate conversion for hit testing

##### Precision Tools

Located in [`View/Precision/`](../Src/View/Precision/):

**Components**:
- [`GridManager`](../Src/View/Precision/GridManager.hpp) - Grid rendering and settings
- [`SnapManager`](../Src/View/Precision/SnapManager.hpp) - Snap calculation and indicators
- [`MeasurementManager`](../Src/View/Precision/MeasurementManager.hpp) - Measurement display
- [`CoordinateInputManager`](../Src/View/Precision/CoordinateInputManager.hpp) - Precise coordinate entry
- [`ExpressionEvaluator`](../Src/View/Precision/ExpressionEvaluator.hpp) - Math expression parsing

**Snap Modes**:
- Grid snap - Align to grid intersections
- Endpoint snap - Snap to figure endpoints
- Midpoint snap - Snap to edge midpoints
- Center snap - Snap to circle/arc centers
- Intersection snap - Snap to geometric intersections
- Nearest point snap - Snap to closest point
- Tangent snap - Snap along tangents
- Perpendicular snap - Snap along perpendiculars

##### Polish Components

Located in [`View/Polish/`](../Src/View/Polish/):

**Components**:
- [`ThemeManager`](../Src/View/Polish/ThemeManager.hpp) - Theme switching and color schemes
- [`ShortcutManager`](../Src/View/Polish/ShortcutManager.hpp) - Keyboard shortcut management
- [`TooltipManager`](../Src/View/Polish/TooltipManager.hpp) - Tooltip display
- [`HelpBrowser`](../Src/View/Polish/HelpBrowser.hpp) - In-application help
- [`ContextMenuManager`](../Src/View/Polish/ContextMenuManager.hpp) - Context menu handling
- [`SettingsDialog`](../Src/View/Polish/SettingsDialog.hpp) - Settings UI
- [`PerformanceMonitor`](../Src/View/Polish/PerformanceMonitor.hpp) - FPS and performance tracking

##### ImGUI Panels

Located in [`View/ImGUI/`](../Src/View/ImGUI/):

**Panels**:
- [`OutlinerPanel`](../Src/View/ObjectManagement/ImGUI/OutlinerPanel.hpp) - Scene hierarchy view
- [`PropertyInspectorPanel`](../Src/View/ObjectManagement/ImGUI/PropertyInspectorPanel.hpp) - Figure properties editor
- [`CommandHistoryPanel`](../Src/View/Commands/ImGUI/CommandHistoryPanel.hpp) - Undo/redo history display
- [`ToolOptionsPanel`](../Src/View/Tools/ImGUI/ToolOptionsPanel.hpp) - Active tool configuration
- [`ViewPresetsPanel`](../Src/View/ImGUI/ViewPresetsPanel.hpp) - View preset selection
- [`CommandManager`](../Src/View/Tools/ImGUI/CommandManager.hpp) - Tool activation UI
- [`KeyboardShortcutManager`](../Src/View/Tools/ImGUI/KeyboardShortcutManager.hpp) - Shortcut reference display
- [`GridSettingsPanel`](../Src/View/Precision/GridSettingsPanel.hpp) - Grid configuration UI
- [`SnapSettingsPanel`](../Src/View/Precision/SnapSettingsPanel.hpp) - Snap configuration UI
- [`CoordinateInputWidget`](../Src/View/Precision/CoordinateInputWidget.hpp) - Precise coordinate entry
- [`PerformanceMonitor`](../Src/View/Polish/PerformanceMonitor.hpp) - Real-time performance display

#### OpenGL Rendering

Located in [`View/OpenGL/`](../Src/View/OpenGL/):

**Components**:
- [`GLProgram`](../Src/View/OpenGL/Details/GLProgram/GLProgram.hpp) - Shader program management
- [`Shader`](../Src/View/OpenGL/Details/Shader/Shader.hpp) - Shader compilation and loading
- [`GUI`](../Src/View/OpenGL/ImGUI/GUI.cpp) - Main ImGUI rendering loop

**Rendering Pipeline**:
1. Clear buffers (color, depth, stencil)
2. Set viewport and scissor
3. Bind shader program
4. Set uniforms (model, view, projection matrices)
5. Bind vertex/array buffers
6. Draw grid (if enabled)
7. Draw figures (iterate through model)
8. Draw preview/ghost geometry (during drawing)
9. Draw selection highlights
10. Draw snap indicators
11. Draw measurements
12. Render ImGUI UI overlay

---

## Integration

### MVC Communication

The three layers communicate through well-defined interfaces:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Application Flow                          │
└─────────────────────────────────────────────────────────────────────────┘

    ┌─────────┐
    │  Model   │
    │           │
    └─────┬────┐
          │         │
          ▼         ▼
    ┌───────────────┐  ┌──────────────┐
    │  Controller   │◄─►│  FSM (State)  │
    │              │  └──────────────┘
    └───────┬─────┐
              │         │
              ▼         ▼
        ┌─────────────┐  ┌──────────────┐
        │    View     │◄─►│  ImGUI/UI    │
        │             │  └──────────────┘
        └─────────────┘

    ┌─────────────────────────────────────────────────────────────┐
    │           GLFW Input (Mouse, Keyboard)              │
    └─────────────────────────────────────────────────────────────┘
```

**Communication Patterns**:

1. **Controller → Model**: Direct method calls on [`IModel`](../Src/Model/IModel.hpp)
   - `addFigure()` - Create new figures
   - `removeFigure()` - Delete figures
   - `updateFigurePosition()` - Move figures
   - `getFigure()` / `findFigureByCoords()` - Query figures

2. **Controller → View**: State change notifications via [`UIFSMAdapter`](../Src/View/UIFSMAdapter.hpp)
   - FSM state changes → UI updates
   - Status bar text updates
   - Selection state notifications
   - Tool activation changes

3. **View → Controller**: Input events via [`IController`](../Src/Controller/IController.hpp)
   - Mouse button state changes
   - Mouse position updates
   - Scroll events
   - Workspace hover state
   - Tool activation commands

4. **View → Model**: Command pattern via [`ICommand`](../Src/View/Commands/ICommand.hpp)
   - Execute commands that modify model
   - Undo/redo through command manager
   - Property updates via property change commands

### Data Flow

**Figure Creation Flow**:

```
User Input (ImGUI)
       │
       ▼
Input Event (IController)
       │
       ▼
FSM State Transition
       │
       ├──────────────────────────────────────┐
       │                                  │
       ▼                                  ▼
   FigureCreator collects points        Finalize → Create Figure
       │                                  │
       ▼                                  ▼
   Store in Model                      Add to Command History
```

**Selection Flow**:

```
Mouse Click (ImGUI)
       │
       ▼
CanvasHitTester (ray casting)
       │
       ▼
Find Figure by ID
       │
       ▼
SelectionManager.updateSelection()
       │
       ├──────────────────┐
       │                  │
       ▼                  ▼
Notify FSM Event    Update UI Panels
```

**Command Execution Flow**:

```
User Action (ImGUI Button/Shortcut)
       │
       ▼
Create Command Object
       │
       ▼
UIFSMAdapter.executeCommand()
       │
       ├──────────────────────────────┐
       │                             │
       ▼                             ▼
Command.execute()              Store in History
       │                             │
       ▼                             ▼
Modify Model                Trigger UI Update
```

### Event Propagation

Events flow through the FSM-based event system:

**FSM Event Flow**:

```
┌──────────────────────────────────────────────────────────────────┐
│                    FSMConfig Event System                     │
└──────────────────────────────────────────────────────────────────┘

    ┌────────────────┐
    │  Event Source │
    │              │
    └──────┬───────┐
           │            │
           ▼            ▼
    ┌─────────────┐  ┌──────────────────┐
    │  FSMConfig   │  │ Event Registry  │
    │              │  │  (Callbacks)     │
    └──────┬──────┘  └──────────────────┘
           │
           ▼
    ┌──────────────────────────────────────────────────┐
    │           Registered Callbacks               │
    │                                          │
    ├──────────────────────────────────────────────┤
    │         │         │         │         │
    ▼         ▼         ▼         ▼         ▼
State Entry  State Exit  Guard   Action  Guard   Transition  UI Update  Model Update
```

**Event Categories**:

1. **Input Events**: Mouse movement, button state, keyboard input
2. **Drawing Events**: Tool activation, figure creation, completion/cancellation
3. **State Events**: Sketch mode entry/exit, plane selection
4. **Cache Events**: Figure added/removed/modified, selection changed, camera changed
5. **Settings Events**: Grid, snap, measurement, theme, shortcut, tooltip changes

---

## Component Diagrams

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           TFCADIR Application                                   │
└─────────────────────────────────────────────────────────────────────────────────┘

    ┌────────────────────────────────────────────────────────────────────────┐
    │                        Model Layer                                     │
    │                                                                        │
    │  ┌──────────────┐  ┌─────────────┐  ┌──────────────┐                   │
    │  │ FlatFigures  │  │SketchPlane  │  │ ICamera      │                   │
    │  │              │  │             │  │              │                   │
    │  └──────────────┘  └─────────────┘  └──────────────┘                   │
    └────────────────────────────────────────────────────────────────────────┘
                                                             │
                                                             ▼
    ┌──────────────────────────────────────────────────────────────────────────┐
    │                                Controller Layer                          │
    │                                                                          │
    │  ┌──────────────┐  ┌──────────┐  ┌──────────────────┐  ┌──────────────┐  │
    │  │ IController  │  │Machine   │  │FigureCreator     │  │FSMConfig     │  │
    │  │              │  │(FSM)     │  │                  │  │              │  │
    │  └──────────────┘  └──────────┘  └──────────────────┘  └──────────────┘  │
    └──────────────────────────────────────────────────────────────────────────┘
                                                             │
                                                             ▼
    ┌──────────────────────────────────────────────────────────────────────────┐
    │                        View Layer                                        │
    │                                                                          │
    │  ┌─────────────────────────────────────────────────────────────────────────┐  │
    │  │                    ImGUI Components & OpenGL Rendering           │  │
    │  │  ┌────────────┐  ┌──────────────┐  ┌──────────────┐  │  │
    │  │  │UI Panels   │  │Commands      │  │Navigation    │  │  │OpenGL       │  │
    │  │  │            │  │              │  │              │  │              │  │
    │  │  └────────────┘  └──────────────┘  └──────────────┘  └──────────────┘  │
    └─────────────────────────────────────────────────────────────────────────────────┘
                                                             │
                                                             ▼
                                                   ┌──────────────────┐
                                                   │  GLFW / ImGUI  │
                                                   └──────────────────┘
```

### MVC Data Flow

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                      MVC Communication Pattern                          │
└──────────────────────────────────────────────────────────────────────────────────┘

    User Input
         │
         ▼
    ┌──────────────────────────────────────────────────────────────────────┐
    │                    Controller (FSM)                            │
    │  ┌────────────────────────────────────────────────────────────────┐  │
    │  │                                                 │  │
    │  │  process_event()                                   │  │
    │  │                                                 │  │
    │  └────────────────────────────────────────────────────────────┬─┘  │
    │                                                     │         │
    │                                                     ▼         ▼
    │  ┌──────────────────────┐              ┌──────────────────────┐
    │  │  State Transition     │              │  Callback Execution  │
    │  │                      │              │                      │  │
    │  └──────────────────────┘              └──────────────────────┘  │
    │                                                                │
    └────────────────────────────────────────────────────────────────────────┼────┐
                                                               │         │
                                          ┌─────────────────────────────────────────────────────┴────┐
                                          │                                       │
                                          ▼                                       ▼
    ┌──────────────────────────┐              ┌──────────────────────────┐
    │  Model Operations       │              │  View Updates          │
    │  │                      │              │                      │  │
    │  │ addFigure()            │              │  UI Panel Updates      │
    │  │ removeFigure()          │              │  Status Bar Changes   │
    │  │ updatePosition()         │              │  Selection Highlights   │
    │  └──────────────────────┘              └──────────────────────┘  │
    └──────────────────────────────────────────────────────────────────────────┘
                                                               │
                                                               ▼
                                                    Render to Screen
```

### FSM State Machine

```
┌──────────────────────────────────────────────────────────────────────────────────┐
│                    FSMConfig State Machine                          │
└──────────────────────────────────────────────────────────────────────────────────┘

    ┌──────────────────────────────────────────────────────────────────────────────┐
    │                      States                                     │
    │  ┌──────────┐  ┌──────────────────┐  ┌──────────────────┐  │
    │  │   Idle    │  │DrawingProcessing  │  │PlaneSelection   │  │
    │  │           │  │                 │  │SketchEdit      │  │
    │  └──────────┘  └──────────────────┘  └──────────────────┘  │
    │                                                                │
    └────────────────────────────────────────────────────────────────────────┼────┐
                                                               │         │
                                          ┌─────────────────────────────────────────────┴────┐
                                          │                                     │
                                          ▼                                     ▼
    ┌──────────────────────────────────────────────────────────────────────────┐
    │  Events (Transitions)                                         │
    │  ┌────────────────────────────────────────────────────────────────────┐  │
    │  │                                                         │  │
    │  │  OnMouseMove → OnAddTriangle → OnFigureComplete → ...  │  │
    │  │                                                         │  │
    │  └────────────────────────────────────────────────────────────────────┘  │
    └──────────────────────────────────────────────────────────────────────────┘
                                                               │
                                                               ▼
                                                    Callbacks / Actions
```

---

## Design Decisions

### ADR-001: MVC Architecture with FSM

**Status**: Accepted

**Context**: TFCADIR requires clear separation between business logic, user interface, and application control while supporting complex state-based workflows.

**Decision**: Adopt Model-View-Controller pattern with FSMConfig for state management.

**Consequences**:
- **Pros**: Loose coupling, testability, clear state management, declarative configuration
- **Cons**: Additional complexity from FSM learning curve, YAML configuration maintenance

### ADR-002: Command Pattern for Undo/Redo

**Status**: Accepted

**Context**: CAD applications require robust undo/redo functionality for all operations.

**Decision**: Implement Command pattern with [`ICommand`](../Src/View/Commands/ICommand.hpp) interface and [`ExtendedCommandManager`](../Src/View/Commands/ExtendedCommandManager.hpp).

**Consequences**:
- **Pros**: Reversible operations, macro command support, clear operation history
- **Cons**: Command object overhead, memory management for history

### ADR-003: Interface-Based Design

**Status**: Accepted

**Context**: Need for testability and flexibility in component implementations.

**Decision**: Define abstract interfaces ([`IModel`](../Src/Model/IModel.hpp), [`IController`](../Src/Controller/IController.hpp), [`IView`](../Src/View/IView.hpp)) for all major components.

**Consequences**:
- **Pros**: Mockability for testing, loose coupling, implementation flexibility
- **Cons**: Interface maintenance overhead, virtual function call costs

### ADR-004: FSMConfig Integration

**Status**: Accepted

**Context**: Complex state management requirements with multiple states and transitions.

**Decision**: Integrate FSMConfig library as dependency for declarative state machine configuration.

**Consequences**:
- **Pros**: YAML-based configuration, event-driven architecture, callback system
- **Cons**: External dependency, VariableValue type limitations

### ADR-005: ImGUI for Immediate Mode GUI

**Status**: Accepted

**Context**: Need for immediate mode GUI with CAD-style controls and docking.

**Decision**: Use ImGUI for all UI rendering with custom CAD-style panels.

**Consequences**:
- **Pros**: Immediate mode rendering, built-in controls, docking system, theming
- **Cons**: Limited styling options, immediate mode constraints

---

## Technology Stack

### External Dependencies

| Dependency | Version | Purpose |
|------------|-------|---------|
| **FSMConfig** | (local submodule) | Finite state machine with YAML configuration |
| **GLFWpp** | (local submodule) | C++ window and input wrapper |
| **GLAD** | (local submodule) | OpenGL loader |
| **GLM** | (local submodule) | OpenGL mathematics library |
| **yaml-cpp** | system | YAML configuration parsing |
| **spdlog** | system | Fast logging framework |

### Build System

**CMake 3.20+** configuration:
- Modular CMakeLists.txt structure
- Target-based compilation
- Header-only interface separation
- Subdirectory organization (Model, Controller, View)

**Compiler Requirements**:
- C++20 standard
- RAII and smart pointers
- Move semantics support

---

## Development Guidelines

### Coding Standards

**Documentation Style**: Doxygen Java-style (`/** @... */`)

**Naming Conventions**:
- Classes: PascalCase (`FigureCreator`, `SelectionManager`)
- Methods: camelCase (`addFigure()`, `getCamera()`)
- Members: trailing underscore (`selectedFigureIds_`, `currentTool_`)
- Files: kebab-case or PascalCase for headers

**Memory Management**:
- Use `std::unique_ptr` for exclusive ownership
- Use `std::shared_ptr` for shared ownership
- Avoid raw pointers and manual `new`/`delete`
- Prefer move semantics over copies

### Build System

**Compilation**:
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build (parallel)
cmake --build build --parallel $(nproc)

# Alternative: Ninja
ninja -C build -j $(nproc)
```

**Testing**:
```bash
# Run tests
ctest --output-on-failure

# Run specific test
./TFCADIR_tests --gtest_filter=SelectionManagerTest
```

### Linting

**Clang-Tidy**: Comprehensive configuration in [`.clang-tidy`](../.clang-tidy) with ~250+ enabled checks

**Clang-Format**: Configuration in [`.clang-format`](../.clang-format) for consistent code style

**Pre-commit Hooks**: Optional hooks for automated quality checks

### Git Workflow

**Branch Naming**:
- `feature/` - New features
- `bugfix/` - Bug fixes
- `refactor/` - Code refactoring
- `docs/` - Documentation updates

**Commit Message Format**:
```
<type>: <short description>

<detailed description (optional)>

<task references (optional)>
```

**Types**: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`

---

## References

- [API Reference](../api_reference.md) - Detailed API documentation
- [User Manual](user_manual.md) - End-user documentation
- [Keyboard Shortcuts Card](keyboard_shortcuts_card.md) - Quick reference
- [Integration Test Results](integration_test_results.md) - Phase 8.3 testing outcomes
- [Setup Documentation](setup.md) - Build and setup instructions

---

*Document Version: 1.0*
*Last Updated: 2026-02-11*
*Phase: 8.7 - Architecture Documentation*
