# TFCADIR API Reference

## Table of Contents

- [Overview](#overview)
- [Model Layer](#model-layer)
  - [IFigure Interface](#ifigure-interface)
  - [ICamera Interface](#icamera-interface)
  - [IModel Interface](#imodel-interface)
  - [FlatFigure Classes](#flatfigure-classes)
  - [Figure Types](#figure-types)
  - [SketchPlane Class](#sketchplane-class)
  - [Memento Class](#memento-class)
- [Controller Layer](#controller-layer)
  - [IController Interface](#icontroller-interface)
  - [FSM Machine Class](#fsm-machine-class)
  - [FSM Events](#fsm-events)
  - [FigureCreator Classes](#figurecreator-classes)
- [View Layer](#view-layer)
  - [IView Interface](#iview-interface)
  - [GUI Class](#gui-class)
  - [ICommand Interface](#icommand-interface)
- [Integration](#integration)
  - [MVC Architecture](#mvc-architecture)
  - [Data Flow](#data-flow)
  - [Event Propagation](#event-propagation)

---

## Overview

TFCADIR (Three-Dimensional Figure Creation and Design in R) is a CAD application built using C++20 with the Model-View-Controller (MVC) architecture pattern. The application provides tools for creating and manipulating 2D geometric figures in a 3D workspace.

### Key Dependencies

- **GLM**: OpenGL Mathematics library for vector and matrix operations
- **GLFW**: Window and input management
- **ImGUI**: Immediate mode GUI library
- **spdlog**: Fast C++ logging library
- **FSMConfig**: Finite State Machine configuration library
- **nlohmann/json**: JSON parsing and serialization

### Architecture Layers

```
┌─────────────────────────────────────────────────────┐
│                    View Layer                       │
│  (GUI, OpenGL Rendering, User Interaction)          │
└─────────────────────┬───────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────┐
│                 Controller Layer                    │
│  (FSM, FigureCreator, Event Processing)             │
└─────────────────────┬───────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────┐
│                   Model Layer                       │
│  (FlatFigure, Camera, Coordinate Systems)           │
└─────────────────────────────────────────────────────┘
```

---

## Model Layer

The Model layer contains all data structures and business logic for geometric figures, coordinate transformations, and camera management.

### IFigure Interface

**Location**: [`Src/Model/IModel.hpp`](Src/Model/IModel.hpp:18)

Abstract base interface for all figure types. Provides a common abstraction for geometric entities with position and identification capabilities.

```cpp
class IFigure {
public:
    virtual ~IFigure() = default;

    // Identification
    virtual uint32_t getId() const = 0;
    virtual const std::string& getName() const = 0;
    virtual void setName(const std::string& name) = 0;

    // Position
    virtual glm::vec3 getPosition() const = 0;
    virtual void setPosition(const glm::vec3& position) = 0;

    // Bounds
    virtual std::pair<glm::vec3, glm::vec3> getBounds() const = 0;
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `getId()` | `uint32_t` | Returns the unique identifier for this figure |
| `getName()` | `const std::string&` | Returns the figure's name |
| `setName(name)` | `void` | Sets a new name for the figure |
| `getPosition()` | `glm::vec3` | Returns the figure's position in 3D space |
| `setPosition(pos)` | `void` | Sets the figure's position |
| `getBounds()` | `std::pair<glm::vec3, glm::vec3>` | Returns min/max bounds of the figure |

---

### ICamera Interface

**Location**: [`Src/Model/IModel.hpp`](Src/Model/IModel.hpp:67)

Abstract camera interface for view transformations. Supports both 2D and 3D rendering.

```cpp
class ICamera {
public:
    virtual ~ICamera() = default;

    virtual void setPosition(const glm::vec3& position) = 0;
    virtual glm::vec3 getPosition() const = 0;
    virtual void setTarget(const glm::vec3& target) = 0;
    virtual glm::vec3 getTarget() const = 0;
    virtual void setUp(const glm::vec3& up) = 0;
    virtual glm::vec3 getUp() const = 0;
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `setPosition(pos)` | `void` | Sets the camera position in 3D space |
| `getPosition()` | `glm::vec3` | Returns the current camera position |
| `setTarget(target)` | `void` | Sets the camera's look-at point |
| `getTarget()` | `glm::vec3` | Returns the current target position |
| `setUp(up)` | `void` | Sets the camera's up vector (orientation) |
| `getUp()` | `glm::vec3` | Returns the current up vector |

---

### IModel Interface

**Location**: [`Src/Model/IModel.hpp`](Src/Model/IModel.hpp:117)

Pure abstract interface for model implementations supporting both 2D and 3D entities.

```cpp
class IModel {
public:
    virtual ~IModel() = default;

    // Figure management
    virtual void addFigure(const glm::vec3& position) = 0;
    virtual bool removeFigure(uint32_t id) = 0;
    virtual std::shared_ptr<IFigure> getFigure(uint32_t id) = 0;
    virtual std::shared_ptr<IFigure> findFigureByCoords(const glm::vec3& coords, float delta) = 0;
    virtual bool updateFigurePosition(uint32_t id, const glm::vec3& newPosition) = 0;
    virtual size_t getFigureCount() const = 0;
    virtual void clearFigures() = 0;

    // Camera access
    virtual ICamera& getCamera() = 0;
    virtual const ICamera& getCamera() const = 0;

    // Transform matrices
    virtual glm::mat4 getModelMatrix() const = 0;
    virtual void setModelMatrix(const glm::mat4& matrix) = 0;
    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual void setProjectionMatrix(const glm::mat4& matrix) = 0;
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `addFigure(position)` | `void` | Adds a new figure at the specified position |
| `removeFigure(id)` | `bool` | Removes a figure by ID; returns true if successful |
| `getFigure(id)` | `shared_ptr<IFigure>` | Retrieves a figure by ID |
| `findFigureByCoords(coords, delta)` | `shared_ptr<IFigure>` | Finds a figure near the specified coordinates |
| `updateFigurePosition(id, pos)` | `bool` | Updates a figure's position |
| `getFigureCount()` | `size_t` | Returns the total number of figures |
| `clearFigures()` | `void` | Removes all figures from the model |
| `getCamera()` | `ICamera&` | Returns the camera interface |
| `getModelMatrix()` | `glm::mat4` | Returns the model transformation matrix |
| `setModelMatrix(matrix)` | `void` | Sets the model transformation matrix |
| `getViewMatrix()` | `glm::mat4` | Returns the view transformation matrix |
| `getProjectionMatrix()` | `glm::mat4` | Returns the projection matrix |
| `setProjectionMatrix(matrix)` | `void` | Sets the projection matrix |

---

### FlatFigure Classes

**Location**: [`Src/Model/FlatFigure.hpp`](Src/Model/FlatFigure.hpp)

#### Figure Template Class

Template class that wraps geometric figure types with identification, naming, and positioning capabilities.

```cpp
template <class T>
struct Figure : public T, public IFigure {
    uint32_t id_;                    // Unique identifier
    std::string name_;               // Human-readable name
    isScribed scribed_;              // Scribed state (no/inscribed/circumscribed)

    Figure(float x = 0.0f, float y = 0.0f, float z = 0.0f,
           isScribed scribed = isScribed::no);

    // IFigure interface implementation
    uint32_t getId() const override;
    const std::string& getName() const override;
    void setName(const std::string& name) override;
    glm::vec3 getPosition() const override;
    void setPosition(const glm::vec3& position) override;
    std::pair<glm::vec3, glm::vec3> getBounds() const override;
};
```

#### FlatFigures Class

Main model implementation that stores all figures and manages coordinate transformations.

```cpp
class FlatFigures : public IModel {
public:
    glm::mat4 model_;               // Model transformation matrix
    glm::mat4 projection_;          // Projection matrix
    Camera_t camera_;               // Camera structure

    // IModel interface implementation
    void addFigure(const glm::vec3& position) override;
    bool removeFigure(uint32_t id) override;
    std::shared_ptr<IFigure> getFigure(uint32_t id) override;
    std::shared_ptr<IFigure> findFigureByCoords(const glm::vec3& coords, float delta) override;
    bool updateFigurePosition(uint32_t id, const glm::vec3& newPosition) override;
    size_t getFigureCount() const override;
    void clearFigures() override;
    ICamera& getCamera() override;
    const ICamera& getCamera() const override;
    glm::mat4 getModelMatrix() const override;
    void setModelMatrix(const glm::mat4& matrix) override;
    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;
    void setProjectionMatrix(const glm::mat4& matrix) override;

    // Additional methods
    template <class T>
    void createFigure(float x, float y, isScribed scribed = isScribed::no);
    void addFigurePtr(std::shared_ptr<IFigure> figure);
    std::unique_ptr<Memento> createMemento();
    void restoreFromMemento(const Memento& mem);

private:
    std::vector<std::shared_ptr<IFigure>> Figures_;
    glm::mat4 inverseMVP_;
    Mouse_t mouse_;
};
```

---

### Figure Types

**Location**: [`Src/Model/FlatFigure.hpp`](Src/Model/FlatFigure.hpp:20)

#### Point Structure

```cpp
struct Point {
    float x{0.f};
    float y{0.f};
    float z{0.f};
};
```

#### Triangle

```cpp
struct Triangle {
    inline static const std::string name = "Triangle";
    Point first;
    Point second;
    Point third;
};
```

#### Quad

```cpp
struct Quad {
    inline static const std::string name = "Quad";
    Point first;
    Point second;
    Point third;
    Point fourth;
};
```

#### Circle

```cpp
struct Circle {
    inline static const std::string name = "Circle";
    Point center;
    float radius;
};
```

#### Ngon (Regular Polygon)

```cpp
struct Ngon {
    inline static const std::string name = "Ngon";
    Point center;
    Point first;
    float n;          // Number of sides
    float radius;
};
```

#### CurveBezier3 (Cubic Bezier)

```cpp
struct CurveBezier3 {
    inline static const std::string name = "CurveBezier3";
    Point start;
    Point end;
    Point first;      // Control point
};
```

#### CurveBezier4 (Quartic Bezier)

```cpp
struct CurveBezier4 {
    inline static const std::string name = "CurveBezier4";
    Point start;
    Point end;
    Point first;      // First control point
    Point second;     // Second control point
};
```

#### isScribed Enum

```cpp
enum class isScribed {
    no,             // Regular figure
    inscribed,      // Inscribed in a circle
    circumscribed   // Circumscribed around a circle
};
```

---

### SketchPlane Class

**Location**: [`Src/Model/SketchPlane.hpp`](Src/Model/SketchPlane.hpp:13)

Defines a 2D sketching plane in 3D space for 2D sketching operations.

```cpp
class SketchPlane {
public:
    enum class PresetPlane {
        XY,  // Z = 0 plane (horizontal, viewed from top)
        XZ,  // Y = 0 plane (vertical, viewed from front)
        YZ   // X = 0 plane (vertical, viewed from side)
    };

    // Constructors
    explicit SketchPlane(PresetPlane preset);
    SketchPlane(const glm::vec3& normal, const glm::vec3& origin,
                const std::string& name);

    // Getters
    const glm::vec3& getNormal() const;
    const glm::vec3& getOrigin() const;
    const glm::vec3& getUp() const;
    const glm::vec3& getRight() const;
    const std::string& getName() const;
    PresetPlane getPreset() const;

    // Methods
    glm::mat4 getCameraViewMatrix() const;
    float distanceToPoint(const glm::vec3& point) const;
    glm::vec2 projectPoint(const glm::vec3& point) const;
    glm::vec3 unprojectPoint(const glm::vec2& coords) const;

private:
    glm::vec3 normal_;
    glm::vec3 origin_;
    glm::vec3 up_;
    glm::vec3 right_;
    std::string name_;
    PresetPlane preset_;
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `getNormal()` | `const glm::vec3&` | Returns the plane normal vector |
| `getOrigin()` | `const glm::vec3&` | Returns the plane origin point |
| `getUp()` | `const glm::vec3&` | Returns the plane up vector |
| `getRight()` | `const glm::vec3&` | Returns the plane right vector |
| `getName()` | `const std::string&` | Returns the human-readable name |
| `getPreset()` | `PresetPlane` | Returns the preset plane type |
| `getCameraViewMatrix()` | `glm::mat4` | Calculates orthographic camera view matrix |
| `distanceToPoint(point)` | `float` | Returns distance from a point to the plane |
| `projectPoint(point)` | `glm::vec2` | Projects a 3D point onto the plane |
| `unprojectPoint(coords)` | `glm::vec3` | Unprojects 2D coordinates to 3D point |

---

### Memento Class

**Location**: [`Src/Model/FlatFigure.hpp`](Src/Model/FlatFigure.hpp:224)

Implements the Memento pattern for state persistence.

```cpp
class Memento {
    friend class FlatFigures;

private:
    std::vector<std::shared_ptr<IFigure>> Figures_;

public:
    Memento(const std::vector<std::shared_ptr<IFigure>>& Figures);
    ~Memento() = default;
};
```

#### Usage Example

```cpp
// Create a snapshot
auto memento = flatFigures.createMemento();

// ... make changes ...

// Restore state
flatFigures.restoreFromMemento(memento);
```

---

## Controller Layer

The Controller layer manages application state through a finite state machine and handles figure creation using the factory pattern.

### IController Interface

**Location**: [`Src/Controller/IController.hpp`](Src/Controller/IController.hpp:23)

Abstract controller interface for managing user input and application state.

```cpp
namespace controller {
namespace state {
    enum class Button { down, released };
    enum class Workspace { unhovered, hovered };
}

class IController {
public:
    fsm::Machine fsm_;

    // Input state updates
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
}
```

#### Methods

| Method | Description |
|--------|-------------|
| `updateLeftMouseButtonState(state)` | Updates left mouse button state |
| `updateWheelMouseButtonState(state)` | Updates middle mouse button state |
| `updateRightMouseButtonState(state)` | Updates right mouse button state |
| `updateWorkspaceHoverState(state)` | Updates workspace hover state |
| `updateScreenspaceMousePosition(pos)` | Updates mouse position in screen space |
| `updateScroll(delta)` | Updates scroll wheel delta |
| `addTriangleByCenter()` | Initiates triangle creation by center |
| `addTriangleByCorners()` | Initiates triangle creation by corners |
| `addSquareByCenter()` | Initiates square creation by center |
| `addSquareByCorners()` | Initiates square creation by corners |
| `addNgonByCenter()` | Initiates n-gon creation by center |
| `addCircleByCenter()` | Initiates circle creation by center |
| `addLine()` | Initiates line creation |
| `removeFigure()` | Removes selected figure |

---

### FSM Machine Class

**Location**: [`Src/Controller/FSM.hpp`](Src/Controller/FSM.hpp:149)

State Machine wrapper using FSMConfig library. Maintains backward compatibility with the original template-based API.

```cpp
namespace fsm {
class Machine {
public:
    Machine();
    ~Machine();

    // Event processing
    template <typename Event>
    void process_event(const Event& event);

    // State queries
    std::string get_current_state_name() const;
    State get_current_state() const;
    void set_state(State new_state);

    // Model association
    void setModel(model::FlatFigures* model);
    fsmconfig::StateMachine* get_fsm();
    const fsmconfig::StateMachine* get_fsm() const;

    // State callbacks
    void on_idle_enter();
    void on_idle_exit();
    void on_drawing_enter();
    void on_drawing_exit();
    void on_move_first_point_enter();
    void on_move_first_point_exit();
    void on_plane_selection_enter();
    void on_plane_selection_exit();
    void on_sketch_edit_enter();
    void on_sketch_edit_exit();

private:
    std::unique_ptr<fsmconfig::StateMachine> fsm_;
    std::unique_ptr<controller::FigureCreator> currentCreator_;
    std::vector<glm::vec2> collectedPoints_;
    model::FlatFigures* model_;
};
}
```

#### State Enum

```cpp
enum class State {
    Idle,                 // Default state, waiting for input
    DrawingProcessing,    // Drawing a figure
    MoveFirstPoint,       // Moving first point of figure
    PlaneSelection,       // Selecting sketch plane
    SketchEdit            // Editing in sketch mode
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `process_event(event)` | `void` | Processes an event and performs state transitions |
| `get_current_state_name()` | `std::string` | Returns current state name (for debugging) |
| `get_current_state()` | `State` | Returns current state as enum |
| `setModel(model)` | `void` | Sets the model reference for figure creation |
| `get_fsm()` | `StateMachine*` | Returns the underlying FSMConfig StateMachine |

---

### FSM Events

**Location**: [`Src/Controller/FSM.hpp`](Src/Controller/FSM.hpp:30)

All events are defined in the `fsm::events` namespace.

#### Mouse Events

```cpp
struct OnMouseMove {
    glm::vec2 position;
    OnMouseMove(glm::vec2 pos) : position(pos) {}
};
```

#### Figure Creation Events

```cpp
struct OnAddTriangleByCenter {};
struct OnAddTriangleByCorners {};
struct OnAddSquareByCenter {};
struct OnAddSquareByCorners {};
struct OnAddNgonByCenter {};
struct OnAddCircleByCenter {};
struct OnAddLine {};
struct OnFigureComplete {};
struct OnFigureCancel {};
```

#### Sketch Mode Events

```cpp
struct OnEnterSketchMode {};
struct OnExitSketchMode {};
struct OnPlaneSelected {
    int planeIndex;  // 0 = XY, 1 = XZ, 2 = YZ
    OnPlaneSelected(int index) : planeIndex(index) {}
};
```

#### Sketch-Specific Drawing Events

```cpp
struct OnAddTriangleByCenterInSketch {};
struct OnAddCircleByCenterInSketch {};
struct OnAddSquareByCenterInSketch {};
struct OnAddSquareByCornersInSketch {};
struct OnAddNgonByCenterInSketch {};
struct OnAddLineInSketch {};
struct OnFigureCompleteInSketch {};
```

#### Cache Invalidation Events (Phase 6)

```cpp
struct OnFigureAdded { uint32_t figureId; };
struct OnFigureRemoved { uint32_t figureId; };
struct OnFigureModified { uint32_t figureId; };
struct OnSelectionChanged {};
struct OnCameraZoomed {};
struct OnCameraPanned {};
struct OnCameraOrbited {};
struct OnGridSettingsChanged {};
struct OnSnapSettingsChanged {};
struct OnCoordinateInputSettingsChanged {};
struct OnShortcutSettingsChanged {};
```

#### Polish & Optimization Events (Phase 7)

```cpp
struct OnShortcutRegistered {
    std::string shortcut_id;
    std::string action;
    std::string key_combination;
};
struct OnShortcutUnregistered { std::string shortcut_id; };
struct OnShortcutActivated { std::string shortcut_id; };
struct OnContextMenuRequested {
    std::string menu_id;
    glm::vec2 position;
    std::vector<std::string> items;
};
struct OnThemeChanged { std::string theme_name; };
struct OnTooltipChanged {
    std::string tooltip_id;
    std::string content;
    glm::vec2 position;
};
struct OnHelpRequested { std::string topic; };
struct OnPerformanceUpdate {
    float fps;
    float frame_time;
    std::string memory_usage;
};
```

---

### FigureCreator Classes

**Location**: [`Src/Controller/FigureCreator.hpp`](Src/Controller/FigureCreator.hpp:29)

Abstract base class and concrete implementations using the Factory Pattern for creating different geometric figures.

#### FigureCreator Base Class

```cpp
namespace controller {
class FigureCreator {
public:
    model::FlatFigures* model_;

    explicit FigureCreator(model::FlatFigures* model);
    virtual ~FigureCreator() = default;

    virtual size_t getRequiredPoints() const = 0;
    virtual bool validatePoint(const std::vector<glm::vec2>& points,
                              const glm::vec2& newPoint) const;
    virtual bool canComplete(const std::vector<glm::vec2>& points) const;
    virtual void updatePreview(const std::vector<glm::vec2>& points) const;
    virtual void createFigure(const std::vector<glm::vec2>& points) = 0;
    virtual std::string displayName() const = 0;
};
}
```

#### LineCreator

```cpp
class LineCreator : public FigureCreator {
public:
    explicit LineCreator(model::FlatFigures* model);
    size_t getRequiredPoints() const override;  // Returns 2
    std::string displayName() const override;   // Returns "Line"
    bool validatePoint(const std::vector<glm::vec2>& points,
                       const glm::vec2& newPoint) const override;
    bool canComplete(const std::vector<glm::vec2>& points) const override;
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### TriangleByCenterCreator

```cpp
class TriangleByCenterCreator : public FigureCreator {
public:
    explicit TriangleByCenterCreator(model::FlatFigures* model);
    size_t getRequiredPoints() const override;  // Returns 2
    std::string displayName() const override;   // Returns "Triangle (Center)"
    bool validatePoint(const std::vector<glm::vec2>& points,
                       const glm::vec2& newPoint) const override;
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### TriangleByCornersCreator

```cpp
class TriangleByCornersCreator : public FigureCreator {
public:
    explicit TriangleByCornersCreator(model::FlatFigures* model);
    size_t getRequiredPoints() const override;  // Returns 3
    std::string displayName() const override;   // Returns "Triangle (Corners)"
    bool validatePoint(const std::vector<glm::vec2>& points,
                       const glm::vec2& newPoint) const override;
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### SquareByCenterCreator

```cpp
class SquareByCenterCreator : public FigureCreator {
public:
    explicit SquareByCenterCreator(model::FlatFigures* model);
    size_t getRequiredPoints() const override;  // Returns 2
    std::string displayName() const override;   // Returns "Square (Center)"
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### SquareByCornersCreator

```cpp
class SquareByCornersCreator : public FigureCreator {
public:
    explicit SquareByCornersCreator(model::FlatFigures* model);
    size_t getRequiredPoints() const override;  // Returns 2
    std::string displayName() const override;   // Returns "Square (Corners)"
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### NgonByCenterCreator

```cpp
class NgonByCenterCreator : public FigureCreator {
private:
    int numSides_;

public:
    NgonByCenterCreator(model::FlatFigures* model, int numSides = 6);
    size_t getRequiredPoints() const override;  // Returns 2
    std::string displayName() const override;
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### CircleByCenterCreator

```cpp
class CircleByCenterCreator : public FigureCreator {
public:
    explicit CircleByCenterCreator(model::FlatFigures* model);
    size_t getRequiredPoints() const override;  // Returns 2
    std::string displayName() const override;   // Returns "Circle"
    void createFigure(const std::vector<glm::vec2>& points) override;
};
```

#### Base Class Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `getRequiredPoints()` | `size_t` | Returns number of points needed for figure |
| `validatePoint(points, newPoint)` | `bool` | Validates if a new point can be added |
| `canComplete(points)` | `bool` | Checks if figure can be completed |
| `updatePreview(points)` | `void` | Updates preview rendering |
| `createFigure(points)` | `void` | Creates the final figure |
| `displayName()` | `std::string` | Returns display name for UI |

---

## View Layer

The View layer handles all user interface rendering, input processing, and visual feedback.

### IView Interface

**Location**: [`Src/View/IView.hpp`](Src/View/IView.hpp:8)

Abstract view interface for rendering and window management.

```cpp
class IView {
public:
    virtual inline bool shouldClose() const = 0;
    virtual void draw() = 0;
};
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `shouldClose()` | `bool` | Returns true if the window should close |
| `draw()` | `void` | Renders the view |

---

### GUI Class

**Location**: [`Src/View/OpenGL/ImGUI/GUI.hpp`](Src/View/OpenGL/ImGUI/GUI.hpp:29)

Main GUI class that manages all ImGUI panels, docking, and user interaction.

```cpp
class GUI {
private:
    std::shared_ptr<controller::IController> sp_controller_;

    // Dock IDs
    ImGuiID dockId_;
    ImGuiID dockIdTools_;
    ImGuiID dockIdLog_;
    ImGuiID dockIdMouse_;
    ImGuiID dockIdOutliner_;
    ImGuiID dockIdProperties_;
    ImGuiID dockIdCommandHistory_;
    ImGuiID dockIdViewPresets_;
    ImGuiID dockIdGridSettings_;
    ImGuiID dockIdSnapSettings_;
    ImGuiID dockIdCoordinateInput_;

    // Managers and Panels
    std::unique_ptr<view::CameraController> cameraController_;
    std::unique_ptr<view::UIFSMAdapter> uiFSMAdapter_;
    std::unique_ptr<view::ImGUI::ToolOptionsPanel> toolOptionsPanel_;
    std::unique_ptr<view::ImGUI::CommandManager> commandManager_;
    std::unique_ptr<view::SelectionManager> selectionManager_;
    std::unique_ptr<view::OutlinerPanel> outlinerPanel_;
    std::unique_ptr<view::PropertyInspectorPanel> propertyInspectorPanel_;
    std::unique_ptr<view::ExtendedCommandManager> extendedCommandManager_;
    std::unique_ptr<view::CommandHistoryPanel> commandHistoryPanel_;
    std::unique_ptr<view::NavigationManager> navigationManager_;
    std::unique_ptr<view::NavigationEventHandler> navigationEventHandler_;
    std::unique_ptr<view::ViewPresetsPanel> viewPresetsPanel_;
    std::unique_ptr<view::GridSettingsPanel> gridSettingsPanel_;
    std::unique_ptr<view::SnapSettingsPanel> snapSettingsPanel_;
    std::unique_ptr<view::CoordinateInputWidget> coordinateInputWidget_;
    std::unique_ptr<view::MeasurementManager> measurementManager_;
    std::unique_ptr<view::MeasurementDisplay> measurementDisplay_;
    std::unique_ptr<view::ShortcutManager> shortcutManager_;

public:
    GUI(std::shared_ptr<controller::IController> sp_controller);

    // Main rendering method
    std::tuple<ImVec2, float, std::optional<ImVec2>>
    DrawGUI(ImTextureID renderTexture);

private:
    // Panel rendering methods
    void ShowMainMenuBar();
    void ShowDockSpace();
    void ShowLog();
    void ShowSidePanel();
    ImVec2 ShowCanvas(ImTextureID renderTexture);
    void ShowStatusBar();
    void ShowSketchPlaneOverlay();
    void ShowOutlinerPanel();
    void ShowPropertyInspectorPanel();
    void ShowCommandHistoryPanel();
    void ShowViewPresetsPanel();
    void ShowGridSettingsPanel();
    void ShowSnapSettingsPanel();
    void ShowCoordinateInputWidget();
    void ShowMeasurementDisplay();
    void ShowSimpleOverlay();
};
```

#### Public Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `DrawGUI(renderTexture)` | `tuple<ImVec2, float, optional<ImVec2>>` | Renders the entire GUI and returns mouse data |

#### Return Value Description

The `DrawGUI` method returns a tuple containing:
1. `ImVec2` - Mouse position in canvas space
2. `float` - Scroll wheel delta
3. `optional<ImVec2>` - Optional mouse position in world space

---

### ICommand Interface

**Location**: [`Src/View/Commands/ICommand.hpp`](Src/View/Commands/ICommand.hpp:19)

Interface for all command objects in the command system (Command Pattern).

```cpp
namespace view {
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string serialize() const = 0;
    virtual std::string getType() const = 0;
    virtual std::vector<uint32_t> getAffectedFigures() const = 0;
};
}
```

#### Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `execute()` | `bool` | Executes the command; returns true if successful |
| `undo()` | `bool` | Undoes the command; returns true if successful |
| `getDescription()` | `std::string` | Returns human-readable description for UI |
| `serialize()` | `std::string` | Serializes command to JSON for persistence |
| `getType()` | `std::string` | Returns command type identifier |
| `getAffectedFigures()` | `vector<uint32_t>` | Returns IDs of affected figures |

#### Command Implementations

The following concrete command classes implement `ICommand`:

- [`CreateFigureCommand`](Src/View/Commands/CreateFigureCommand.hpp) - Creates new figures
- [`MoveFigureCommand`](Src/View/Commands/MoveFigureCommand.hpp) - Moves figures
- [`RotateFigureCommand`](Src/View/Commands/RotateFigureCommand.hpp) - Rotates figures
- [`ScaleFigureCommand`](Src/View/Commands/ScaleFigureCommand.hpp) - Scales figures
- [`DeleteFiguresCommand`](Src/View/Commands/DeleteFiguresCommand.hpp) - Deletes figures
- [`DuplicateFigureCommand`](Src/View/Commands/DuplicateFigureCommand.hpp) - Duplicates figures
- [`ColorChangeCommand`](Src/View/Commands/ColorChangeCommand.hpp) - Changes figure colors
- [`PropertyChangeCommand`](Src/View/Commands/PropertyChangeCommand.hpp) - Changes properties
- [`GroupFiguresCommand`](Src/View/Commands/GroupFiguresCommand.hpp) - Groups figures
- [`UngroupFiguresCommand`](Src/View/Commands/UngroupFiguresCommand.hpp) - Ungroups figures
- [`MacroCommand`](Src/View/Commands/MacroCommand.hpp) - Executes multiple commands

---

## Integration

### MVC Architecture

TFCADIR follows the Model-View-Controller (MVC) architectural pattern:

```
┌─────────────────────────────────────────────────────────┐
│                     View Layer                          │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │    GUI      │  │   OpenGL     │  │   Panels     │  │
│  │  (ImGUI)    │  │  Rendering   │  │  (Outliner,  │  │
│  │             │  │              │  │   Properties)│  │
│  └─────────────┘  └──────────────┘  └──────────────┘  │
└───────────────────────────┬─────────────────────────────┘
                            │
                            │ User Input
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│                  Controller Layer                       │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │     FSM     │  │FigureCreator │  │   Commands   │  │
│  │ (State      │  │  (Factory    │  │  (Command    │  │
│  │  Machine)   │  │   Pattern)   │  │   Pattern)   │  │
│  └─────────────┘  └──────────────┘  └──────────────┘  │
└───────────────────────────┬─────────────────────────────┘
                            │
                            │ State Changes
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│                   Model Layer                           │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ FlatFigures │  │   Sketch     │  │   Camera     │  │
│  │  (Figure    │  │    Plane     │  │  System      │  │
│  │  Storage)   │  │              │  │              │  │
│  └─────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Data Flow

1. **User Input** → View Layer (GUI captures mouse/keyboard events)
2. **Events** → Controller Layer (FSM processes events and transitions state)
3. **State Changes** → Model Layer (Figures are created/modified)
4. **Model Updates** → View Layer (OpenGL renders updated scene)

### Event Propagation

```
User Action
    │
    ▼
GUI Event Handler
    │
    ├─► NavigationManager (zoom/pan/orbit)
    ├─► UIFSMAdapter (state changes)
    ├─► CommandManager (undo/redo)
    └─► FSM (figure creation)
        │
        ▼
    FigureCreator
        │
        ▼
    FlatFigures (Model)
        │
        ▼
    OpenGL Render (View)
```

### Key Integration Points

| Component | Integration Point | Description |
|-----------|-------------------|-------------|
| `FSM` → `FigureCreator` | State transitions | FSM creates appropriate FigureCreator based on current drawing mode |
| `FigureCreator` → `FlatFigures` | Figure creation | Creator adds completed figures to the model |
| `GUI` → `FSM` | Event forwarding | GUI forwards user input events to FSM for processing |
| `FSM` → `UIFSMAdapter` | State notifications | FSM notifies UI of state changes for visual feedback |
| `CommandManager` → `ICommand` | Undo/redo | Commands encapsulate all reversible operations |
| `SelectionManager` → `FlatFigures` | Figure selection | Manages which figures are currently selected |

---

## Usage Examples

### Creating a Triangle

```cpp
// Get the model
model::FlatFigures figures;

// Create a triangle by center
figures.createFigure<model::Triangle>(100.0f, 100.0f);

// Or using the controller
controller.fsm_.process_event(fsm::events::OnAddTriangleByCenter());
```

### Finding a Figure by Coordinates

```cpp
// Search for a figure near a point
auto figure = figures.findFigureByCoords(glm::vec3(100.0f, 100.0f, 0.0f), 10.0f);

if (figure) {
    std::cout << "Found: " << figure->getName() << std::endl;
}
```

### Using the Sketch Plane

```cpp
// Create a preset plane
model::SketchPlane xyPlane(model::SketchPlane::PresetPlane::XY);

// Project a 3D point to 2D
glm::vec2 coords = xyPlane.projectPoint(glm::vec3(10.0f, 20.0f, 30.0f));

// Unproject back to 3D
glm::vec3 point3d = xyPlane.unprojectPoint(coords);
```

### Processing FSM Events

```cpp
fsm::Machine fsm;
fsm.setModel(&figures);

// Process mouse movement
fsm.process_event(fsm::events::OnMouseMove(glm::vec2(100.0f, 200.0f)));

// Start drawing a circle
fsm.process_event(fsm::events::OnAddCircleByCenter());

// Complete the figure
fsm.process_event(fsm::events::OnFigureComplete());
```

### Creating Custom Commands

```cpp
class MyCustomCommand : public view::ICommand {
public:
    bool execute() override {
        // Do something
        return true;
    }

    bool undo() override {
        // Undo it
        return true;
    }

    std::string getDescription() const override {
        return "My Custom Command";
    }

    std::string serialize() const override {
        nlohmann::json j;
        j["type"] = getType();
        return j.dump();
    }

    std::string getType() const override {
        return "MyCustomCommand";
    }

    std::vector<uint32_t> getAffectedFigures() const override {
        return {}; // No figures affected
    }
};
```

---

## File Reference

| File | Description |
|------|-------------|
| [`Src/Model/IModel.hpp`](Src/Model/IModel.hpp) | Model interfaces (IFigure, ICamera, IModel) |
| [`Src/Model/FlatFigure.hpp`](Src/Model/FlatFigure.hpp) | Figure types and FlatFigures class |
| [`Src/Model/SketchPlane.hpp`](Src/Model/SketchPlane.hpp) | Sketch plane for 2D drawing |
| [`Src/Controller/IController.hpp`](Src/Controller/IController.hpp) | Controller interface |
| [`Src/Controller/FSM.hpp`](Src/Controller/FSM.hpp) | Finite State Machine |
| [`Src/Controller/FigureCreator.hpp`](Src/Controller/FigureCreator.hpp) | Figure creator factory classes |
| [`Src/View/IView.hpp`](Src/View/IView.hpp) | View interface |
| [`Src/View/OpenGL/ImGUI/GUI.hpp`](Src/View/OpenGL/ImGUI/GUI.hpp) | Main GUI class |
| [`Src/View/Commands/ICommand.hpp`](Src/View/Commands/ICommand.hpp) | Command interface |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-02-11 | Initial API documentation for Phase 8.6 |
