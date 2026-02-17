# TFCADIR User Manual

## Overview
TFCADIR is a professional CAD (Computer-Aided Design) application built with modern C++ and OpenGL. This manual provides comprehensive guidance for using all features implemented in Phases 1-7 of the TFCADIR UI/UX roadmap.

**Version:** 1.0
**Last Updated:** 2025-02-11
**Document Version:** Phase 8.1 (Reworked)

> **NOTE:** This documentation is currently located in `docs/.wip/phase-8-documentation/` as work-in-progress. After validation and review, it will be moved to the permanent documentation location.

## Table of Contents
- [Getting Started](#getting-started) - Installation and first launch
- [Interface Overview](#interface-overview) - Menus, toolbars, panels, and navigation
- [2D Drafting Workflow](#2d-drafting-workflow) - Creating 2D geometry in sketch mode
- [3D Modeling Workflow](#3d-modeling-workflow) - Creating 3D geometry in free space
- [Object Management](#object-management) - Selection, outliner, and properties
- [Command System](#command-system) - Undo, redo, and command history
- [Navigation Controls](#navigation-controls) - Zoom, pan, orbit, and view presets
- [Precision Tools](#precision-tools) - Snap, coordinate input, and measurements
- [Keyboard Shortcuts](#keyboard-shortcuts) - Complete shortcuts reference
- [Settings and Customization](#settings-and-customization) - Themes, preferences, and configuration
- [Tips and Tricks](#tips-and-tricks) - Productivity tips
- [Troubleshooting](#troubleshooting) - Common issues and solutions

---

## Getting Started

### Installation

#### Prerequisites

TFCADIR requires the following dependencies:

| Dependency | Version | Purpose |
|------------|---------|---------|
| CMake | 3.20+ | Build system |
| C++ Compiler | C++20 | Modern C++ standard |
| OpenGL | 3.3+ | Graphics rendering |
| GLFW | Latest | Window and input |
| Dear ImGui | Latest | Immediate mode GUI |
| GLM | Latest | OpenGL mathematics |
| spdlog | Latest | Logging |
| FSMConfig | Latest | State machine configuration |

#### Building on Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt update && sudo apt-get install xorg-dev libglu1-mesa-dev libspdlog-dev

# Clone with submodules
git clone --recurse-submodules https://github.com/MaroonSkull/TFCADIR.git
cd TFCADIR

# Configure and build
cmake -B build && cmake --build build --parallel $(nproc)
```

#### Building on Windows

1. Install Git for Windows (if not already installed)
2. Install Visual Studio 2019 or later
3. Clone repository with submodules
4. Open CMake GUI and configure
5. Generate project files and build in Visual Studio

### First Launch

When you first launch TFCADIR, you will see:

```
┌─────────────────────────────────────────────────────────┐
│                                                     │
│                   TFCADIR                              │
│              (The First CAD In Russia)              │
│                                                     │
└─────────────────────────────────────────────────────────┘
```

The application opens in **2D Top View** mode by default, with the main toolbar on the left and the canvas occupying the central area.

---

## Interface Overview

### Main Window Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│ Menu │ Toolbar │              Canvas              │ Outliner │ Properties│
│ Bar  └──────────┘              Area                └──────────┴──────────┘│
│                                                         │                   │
└─────────────────────────────────────────────────────────────────────────┘
```

**Component Description:**

| Component | Location | Purpose |
|------------|---------|---------|
| **Menu Bar** | Top | File, Edit, View, Tools, Help menus |
| **Main Toolbar** | Left | Tool buttons (Line, Circle, Arc, Rectangle, Polygon, N-gon) |
| **Canvas Area** | Center | 3D/2D drawing viewport with grid |
| **Outliner Panel** | Right | Hierarchical object list |
| **Property Inspector** | Right | Object properties editor |
| **Status Bar** | Bottom | Current mode, coordinates, hints |
| **Command History** | Right (dockable) | Undo/Redo history |

### Menu Bar

#### File Menu
- **New Project** - Create new project (clears all figures)
- **Open Project** - Open saved project file
- **Save Project** - Save current project to file
- **Export** - Export geometry to various formats
- **Exit** - Close application

#### Edit Menu
- **Undo** - Undo last action (`Ctrl+Z`)
- **Redo** - Redo last undone action (`Ctrl+Y`)
- **Cut** - Cut selected figures (`Ctrl+X`)
- **Copy** - Copy selected figures (`Ctrl+C`)
- **Paste** - Paste figures (`Ctrl+V`)
- **Duplicate** - Duplicate selected (`Ctrl+D`)
- **Delete** - Delete selected (`Delete`)

#### View Menu
- **Zoom In** - Zoom in (`Ctrl++` or `Ctrl+Wheel Up`)
- **Zoom Out** - Zoom out (`Ctrl+-` or `Ctrl+Wheel Down`)
- **Zoom to Fit** - Fit all objects in view (`Ctrl+F`)
- **Reset View** - Reset to default view (`Home`)
- **Toggle Grid** - Show/hide grid (`G`)
- **Four-View Layout** - Show multiple views simultaneously (`F4`)
- **View Presets** - Quick view switching

#### Tools Menu
- **Select Tool** - Selection mode (`Esc` or `Space`)
- **Line Tool** - Draw lines (`L`)
- **Circle Tool** - Draw circles (`C`)
- **Arc Tool** - Draw arcs (`A`)
- **Rectangle Tool** - Draw rectangles (`R`)
- **Polygon Tool** - Draw polygons (`P`)
- **N-gon Tool** - Draw regular polygons (`N`)
- **Enter Sketch Mode** - 2D sketching on plane (`K`)

#### Help Menu
- **Documentation** - Open user manual (`F1`)
- **Keyboard Shortcuts** - Show shortcuts reference (`F2`)
- **Settings** - Open settings dialog (`F3`)

### Main Toolbar

The main toolbar provides quick access to all drawing tools:

```
┌──────────────────┐
│ [Select] [Line] │  Tool Buttons
│ [Circle] [Arc]  │
│ [Rect] [Poly]  │
│ [N-gon] [Sketch]│
└──────────────────┘
```

**Tool Buttons:**
- **Select/Arrow** - Selection and object manipulation
- **Line** - Draw line segments
- **Circle** - Draw circles (center+radius or 3-point)
- **Arc** - Draw arcs (start, control, end)
- **Rectangle** - Draw rectangles
- **Polygon** - Draw polygons
- **N-gon** - Draw regular polygons
- **Sketch Mode** - Enter 2D sketching on selected plane

### Canvas Area

The canvas is the main drawing area featuring:

- **3D/2D Viewport** - OpenGL rendering context
- **Grid System** - Configurable reference grid
- **Coordinate Axes** - X (red), Y (green), Z (blue)
- **Origin Point** - World origin marker
- **Snap Indicators** - Visual snap feedback
- **Preview Geometry** - Ghost/preview of shapes being drawn
- **Selection Highlight** - Visual feedback for selected objects

### Outliner Panel

The Outliner panel displays all objects in the scene:

```
┌──────────────────────┐
│ □ Scene            │  Tree View
│   □ Triangle 1      │
│   □ Circle 2         │
│   □ Rectangle 3       │
│   └─ ...             │
└──────────────────────┘
```

**Features:**
- Hierarchical object tree
- Expand/collapse groups
- Rename objects (double-click or `F2`)
- Show/hide objects
- Drag to reorder
- Right-click context menu

### Property Inspector

The Property Inspector shows detailed properties of selected objects:

```
┌──────────────────────┐
│ Properties:         │
│ Type: Circle        │
│ Center: X: 10.5    │
│         Y: 20.0    │
│         Z: 0.0     │
│ Radius: 15.0       │
│ Color: [Color]      │
│ Layer: Default      │
└──────────────────────┘
```

**Editable Properties:**
- Position (X, Y, Z)
- Dimensions (radius, width, height, etc.)
- Color (color picker)
- Layer assignment
- Name/label

### Status Bar

The status bar displays:
- **Current Mode** - Active tool or mode
- **Cursor Coordinates** - Real-time position
- **Action Hints** - Context-sensitive help
- **Measurement Info** - Distance, angle, area
- **Snap Status** - Active snap modes

---

## 2D Drafting Workflow

### Sketch Mode Overview

Sketch Mode allows you to draw 2D geometry on a selected plane, which can then be used as reference for 3D operations.

#### Entering Sketch Mode

1. Press `K` or click **Sketch Mode** button in toolbar
2. Select a plane in the plane selection dialog:
   - **Right Plane** - XY plane (Z=0)
   - **Top Plane** - XZ plane (Y=0)
   - **Front Plane** - YZ plane (X=0)
3. The selected plane appears in the 3D view

#### Drawing in Sketch Mode

Once a plane is selected, you can draw 2D shapes:

**Available Tools in Sketch Mode:**
- **Line** - Draw line segments
- **Circle** - Draw circles
- **Square** - Draw squares (center or corners)
- **Triangle** - Draw triangles (center or corners)
- **N-gon** - Draw regular polygons

#### Drawing Process

1. Select a drawing tool
2. Click to place points:
   - Line: 2 points (start, end)
   - Circle: 2 points (center, edge)
   - Square/Triangle: 2-4 points depending on method
3. Press `Enter` or click first point again to complete shape
4. Shape is finalized and added to the model

#### Sketch Plane Features

- **Grid Snapping** - Automatically snap to grid intersections
- **Endpoint Snapping** - Snap to existing shape endpoints
- **Real-time Measurements** - See dimensions as you draw
- **Coordinate Input** - Type exact coordinates

---

## 3D Modeling Workflow

### Drawing Tools

TFCADIR provides several 3D drawing tools for creating geometry in free space.

#### Line Tool

**Purpose:** Draw 3D line segments

**Usage:**
1. Select Line tool from toolbar or press `L`
2. Click to place start point
3. Move cursor and click to place end point
4. Line is automatically completed

**Options:**
- None (lines always defined by 2 points)

#### Circle Tool

**Purpose:** Draw 3D circles

**Usage:**
1. Select Circle tool from toolbar or press `C`
2. Choose creation method in Tool Options panel:
   - **Center + Edge** - Click center, then edge point
   - **3 Points** - Click 3 points on circumference
3. Set radius (if using Center+Edge method)
4. Click to place final point
5. Circle is completed

**Options:**
- **Radius** - Circle radius
- **Creation Method** - Center+Edge or 3 Points

#### Arc Tool

**Purpose:** Draw 3D arcs

**Usage:**
1. Select Arc tool from toolbar or press `A`
2. Click to place start point
3. Click to place control point (defines curvature)
4. Click to place end point
5. Arc is completed

**Options:**
- None (arcs defined by 3 points)

#### Rectangle Tool

**Purpose:** Draw 3D rectangles

**Usage:**
1. Select Rectangle tool from toolbar or press `R`
2. Choose creation method in Tool Options panel:
   - **Center + Corner** - Click center, then opposite corner
   - **2 Corners** - Click 2 diagonal corners
3. Click to place points
4. Rectangle is completed

**Options:**
- **Width** - Rectangle width (for center method)
- **Height** - Rectangle height (for center method)
- **Creation Method** - Center+Corner or 2 Corners

#### Polygon Tool

**Purpose:** Draw 3D polygons

**Usage:**
1. Select Polygon tool from toolbar or press `P`
2. Click to place each vertex
3. Press `Enter` or double-click to complete
4. Polygon is closed automatically

**Options:**
- None (polygons defined by N vertices)

#### N-gon Tool

**Purpose:** Draw regular polygons (N-sided shapes)

**Usage:**
1. Select N-gon tool from toolbar or press `N`
2. Set number of sides in Tool Options panel (3-12)
3. Click center point
4. Set radius
5. N-gon is completed

**Options:**
- **Sides** - Number of sides (3-12)
- **Radius** - Distance from center to vertices

---

## Object Management

### Selection System

TFCADIR provides a robust selection system for working with multiple objects.

#### Selection Methods

| Method | Action | Shortcut |
|---------|---------|---------|
| **Single Select** | Click on object | Left Click |
| **Add to Selection** | Add without clearing | `Shift+Click` |
| **Toggle Selection** | Add/remove from selection | `Ctrl+Click` |
| **Select All** | Select all objects | `Ctrl+A` |
| **Clear Selection** | Deselect all | `Esc` |
| **Box Select** | Select in region | `Shift+Drag` |

#### Selection Display

Selected objects are highlighted with a distinct color (configurable in settings). The primary selection is shown with a brighter highlight.

#### Multi-Selection Workflow

1. Select first object (becomes primary selection)
2. Hold `Shift` and click additional objects
3. All selected objects can be manipulated together
4. Properties panel shows properties of primary selection
5. Operations affect all selected objects

### Outliner Panel

The Outliner provides a hierarchical view of all objects in the scene.

#### Features

- **Tree Structure** - Parent-child relationships
- **Expand/Collapse** - Show/hide children
- **Rename** - Double-click or press `F2`
- **Visibility Toggle** - Show/hide objects
- **Drag Reorder** - Change object order
- **Selection Sync** - Clicking selects in 3D view

#### Context Menu Actions

Right-click on objects in Outliner:
- **Select** - Select object
- **Deselect** - Deselect object
- **Delete** - Remove object(s)
- **Duplicate** - Copy object(s)
- **TODO: Group** - Create group (not yet implemented)
- **TODO: Ungroup** - Dissolve group (not yet implemented)

### Property Inspector

The Property Inspector allows precise editing of object properties.

#### Property Categories

**Transform Properties:**
- Position (X, Y, Z coordinates)
- Rotation (Euler angles or quaternions)
- Scale (X, Y, Z factors)

**Geometry Properties:**
- Radius (for circles/arcs)
- Width/Height/Depth (for rectangles)
- Side count (for polygons)

**Appearance Properties:**
- Color (RGB color picker)
- Layer assignment
- Visibility toggle
- Name/Label

#### Editing Properties

1. Select object in Outliner or 3D view
2. Properties appear in Property Inspector
3. Click property value to edit
4. Type new value or use color picker
5. Press `Enter` to apply changes
6. Changes are recorded in command history (undoable)

---

## Command System

### Undo/Redo

TFCADIR maintains a complete command history allowing you to undo and redo actions.

| Action | Shortcut | Description |
|---------|---------|-------------|
| **Undo** | `Ctrl+Z` | Revert last action |
| **Redo** | `Ctrl+Y` | Restore undone action |
| **Undo Multiple** | `Ctrl+Shift+Z` | Undo multiple steps |
| **Redo Multiple** | `Ctrl+Shift+Y` | Redo multiple steps |

### Command History Panel

The Command History panel shows all executed commands:

```
┌──────────────────────┐
│ Command History     │
│ □ Create Circle    │
│ □ Move Rectangle  │
│ ▶ Change Color     │  ← Current Position
│ □ Delete Triangle   │
│   ...             │
└──────────────────────┘
```

**Features:**
- Scroll through all commands
- Click any command to jump to that state
- Undo to selected position
- Redo from selected position
- Clear history

### Clipboard Operations

| Operation | Shortcut | Description |
|------------|---------|-------------|
| **Cut** | `Ctrl+X` | Remove and copy to clipboard |
| **Copy** | `Ctrl+C` | Copy to clipboard |
| **Paste** | `Ctrl+V` | Paste from clipboard |
| **Duplicate** | `Ctrl+D` | Duplicate in place |

### Command Types

All commands in TFCADIR implement the `ICommand` interface:

- **CreateFigureCommand** - Creates new geometry
- **MoveFigureCommand** - Translates objects
- **RotateFigureCommand** - Rotates objects
- **ScaleFigureCommand** - Scales objects
- **ColorChangeCommand** - Changes object color
- **DeleteFiguresCommand** - Removes objects
- **DuplicateFigureCommand** - Creates copies
- **TODO: GroupFiguresCommand** - Creates object groups (not yet implemented)
- **PropertyChangeCommand** - Modifies properties

---

## Navigation Controls

### Zoom Controls

TFCADIR provides multiple ways to zoom the view.

| Method | Shortcut | Description |
|---------|---------|-------------|
| **Zoom In** | `Ctrl++` or `Ctrl+Wheel Up` | Increase zoom |
| **Zoom Out** | `Ctrl+-` or `Ctrl+Wheel Down` | Decrease zoom |
| **Zoom to Fit** | `Ctrl+F` | Fit all objects |
| **Zoom to Selection** | `Ctrl+Shift+F` | Fit selected objects |

### Zoom Box Selection

For precise zoom control:

1. Press and hold `Ctrl+Shift`
2. Click and drag to create zoom box
3. Release to zoom to box region

### Pan Controls

| Method | Shortcut | Description |
|---------|---------|-------------|
| **Pan** | Middle Mouse Drag or `Space+Drag` | Move view |
| **Pan to Cursor** | `Right Mouse Double-Click` | Center on cursor |

### 3D Orbit Controls

Orbit around the scene to view from different angles.

| Method | Shortcut | Description |
|---------|---------|-------------|
| **Orbit** | Left Mouse Drag | Rotate around orbit center |
| **Pan Orbit** | `Shift+Left Drag` | Pan orbit center |
| **Zoom Orbit** | Mouse Wheel | Zoom in/out |
| **Dolly** | `Ctrl+Right Drag` | Move in/out |

**Orbit Centers:**
- **Origin** - Orbit around world origin (0,0,0)
- **Selection Center** - Orbit around selected objects
- **Custom** - User-defined orbit point

### View Presets

Quickly switch between predefined views:

| Preset | Shortcut | Description |
|---------|---------|-------------|
| **Top 2D** | `F5` | View from +Y direction |
| **Front 2D** | `F6` | View from +Z direction |
| **Right 2D** | `F7` | View from +X direction |
| **Isometric** | `F8` | 3D isometric view |

### Four-View Layout

Press `F4` to display four simultaneous views:

```
┌──────────┬──────────┐
│ Top      │ Front     │
│ 2D       │ 2D        │
│          │           │
├──────────┼──────────┤
│ Right    │ Isometric  │
│ 2D       │           │
└──────────┴──────────┘
```

---

## Precision Tools

### Snap System

The snap system provides precise positioning by automatically snapping to key geometry points.

#### Snap Modes

| Snap Type | Description | Default |
|-----------|-------------|---------|
| **Grid** | Snap to grid intersections | Enabled |
| **Endpoint** | Snap to shape endpoints | Enabled |
| **Midpoint** | Snap to edge midpoints | Enabled |
| **Center** | Snap to circle/arc centers | Enabled |
| **Intersection** | Snap to line intersections | Enabled |
| **Nearest** | Snap to nearest point | Disabled |
| **Tangent** | Snap to tangent points | Disabled |
| **Perpendicular** | Snap to perpendicular points | Disabled |

#### Snap Indicators

When snap is active, visual indicators appear:
- **Green Circle** - Snap point location
- **Dashed Line** - Connection to cursor
- **Tooltip** - Snap type and distance

#### Snap Settings

Access via **Settings → Snap Settings** or press `F9`:

- Toggle individual snap modes
- Adjust snap tolerance (pixels)
- Change indicator color
- Enable/disable visual indicators

**Technical Note:** The SnapManager uses a dirty flag pattern with 100ms rate limiting to optimize performance when querying snap settings. Snap settings are stored centrally in UIFSMAdapter as the single source of truth.

### Grid System

The configurable grid provides spatial reference for drawing.

#### Grid Settings

Access via **Settings → Grid Settings**:

- **Visibility** - Show/hide grid
- **Major Spacing** - Distance between major lines
- **Minor Divisions** - Subdivisions between major lines
- **Colors** - Major/minor line colors
- **Opacity** - Grid transparency
- **Axes Display** - Show X, Y, Z axes
- **Origin Marker** - Show world origin

#### Grid Snapping

When grid snap is enabled, the cursor automatically snaps to grid intersections.

### Coordinate Input

For precise coordinate entry, use the coordinate input fields.

#### Input Modes

| Mode | Description | Example |
|-------|-------------|---------|
| **Absolute** | World coordinates | `X: 100.5, Y: 50.0` |
| **Relative** | Offset from last point | `@X: 25.0, @Y: 10.0` |

#### Coordinate Input Panel

```
┌──────────────────────┐
│ X: [100.5    │  Coordinate Input
│ Y: [50.0      │  Fields
│ Z: [0.0        │
│                  │
│ [Apply] [Cancel] │
└──────────────────────┘
```

**Features:**
- Expression parsing (e.g., `100/2`, `sin(45)`)
- Precision control (decimal places)
- Snap to precision grid
- Angular input for rotations

**Note:** Expression evaluation is implemented using a basic parser. For complex mathematical expressions, consider using external calculation tools.

### Measurement Tools

Real-time measurement feedback during drawing operations.

#### Measurement Types

| Type | Description |
|-------|-------------|
| **Distance** | Length between points |
| **Angle** | Angle between segments |
| **Area** | Area of closed shapes |
| **Perimeter** | Perimeter of closed shapes |

#### Measurement Settings

Access via **Settings → Measurement Settings**:

- Show/hide measurements
- Real-time feedback toggle
- Precision (decimal places)
- Toggle individual measurement types

---

## Keyboard Shortcuts

For a complete keyboard shortcuts reference, see [Shortcuts Reference](shortcuts_reference.md).

**Essential Shortcuts:**
| Shortcut | Action |
|---------|--------|
| `Esc` | Cancel current operation / Deselect all |
| `Delete` | Delete selected objects |
| `Space` | Select tool / Pan with mouse |
| `Enter` | Complete current operation |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl+S` | Save project |
| `Ctrl+O` | Open project |

---

## Settings and Customization

### Theme System

TFCADIR includes multiple theme presets for different working environments.

**Technical Note:** The ThemeManager uses mutexes for thread-safe access to theme data. Theme changes are applied atomically to prevent rendering inconsistencies.

#### Theme Presets

| Theme | Description | Use Case |
|-------|-------------|----------|
| **Dark** | Professional CAD theme (default) | General use |
| **Light** | Bright environments | Well-lit rooms |
| **High Contrast** | Accessibility | Low vision users |
| **Custom** | User-defined colors | Personalized workflow |

#### Theme Settings

Access via **Settings → Theme** or press `F3`:

- Select theme preset
- Customize individual colors
- Adjust UI element colors
- Export/import theme configurations

**Customizable Colors:**
- Window backgrounds
- Text colors
- Button colors
- Grid colors
- Selection colors
- Accent colors

### Keyboard Shortcuts Customization

All keyboard shortcuts can be customized:

1. Open **Settings → Keyboard Shortcuts**
2. Select shortcut to modify
3. Press new key combination
4. Apply changes
5. Export/import shortcut configurations

### Settings Dialog

Access all application settings:

**Categories:**
- **General** - Application behavior
- **Display** - Grid, measurements, tooltips
- **Navigation** - Camera controls, orbit settings
- **Performance** - FPS limiting, quality settings
- **Keybindings** - All shortcut customizations

---

## Tips and Tricks

### Productivity Tips

1. **Use Sketch Mode for 2D Layouts**
   - Create floor plans on sketch planes
   - Use as reference for 3D modeling

2. **Master Selection Shortcuts**
   - `Ctrl+A` selects all quickly
   - `Shift+Click` builds selection efficiently
   - `Ctrl+Click` toggles without losing selection

3. **Use View Presets**
   - `F5`-`F8` quickly switch between standard views
   - `F4` for four-view layout for technical drawings

4. **Enable Appropriate Snaps**
   - Grid snap for alignment
   - Endpoint snap for precision
   - Disable unused snaps to reduce clutter

5. **Leverage Command History**
   - Click history panel to jump to previous states
   - Undo multiple steps with `Ctrl+Shift+Z`
   - Use redo after exploring alternatives

6. **Keyboard for Precision**
   - Use coordinate input fields for exact values
   - Type expressions like `100/2` for quick calculations
   - Use relative coordinates (`@X:`) for offsets

7. **Group Related Objects** (TODO: Feature not yet implemented)
   - Keep Outliner organized
   - Group functional assemblies
   - Use descriptive names

8. **Customize Your Workspace**
   - Set theme for comfort
   - Arrange panels for your workflow
   - Create custom shortcuts for frequent actions

### Advanced Techniques

#### Construction Geometry

1. **Create Reference Planes**
   - Use sketch mode to draw reference shapes
   - Extrude or use as guides for 3D modeling

2. **Use Grid for Alignment**
   - Enable grid snap
   - Set appropriate spacing for your scale
   - Use major grid lines for visual reference

3. **Precision Input**
   - Combine coordinate input with snap
   - Use expressions for calculated positions
   - Chain relative coordinates

#### Workflow Optimization

1. **Template Projects**
   - Save project with standard layers
   - Include reference geometry
   - Pre-configure view settings

2. **Layer Management**
   - Organize objects by function or system
   - Use layer visibility to simplify complex scenes
   - Name layers descriptively

3. **Session Management**
   - Save frequently during long sessions
   - Use command history for experimentation
   - Clear history when starting new project

---

## Troubleshooting

### Common Issues

#### Issue: Objects Not Visible

**Symptoms:**
- Created objects don't appear in viewport
- Objects disappear when zooming

**Solutions:**
1. Check view clipping distance in settings
2. Ensure objects are within camera frustum
3. Reset view with `Home` key
4. Verify layer visibility is enabled

#### Issue: Snapping Not Working

**Symptoms:**
- Cursor doesn't snap to expected points
- Snap indicators don't appear

**Solutions:**
1. Verify snap modes are enabled in settings (press `F9`)
2. Check snap tolerance (may be too small/large)
3. Ensure grid is visible for grid snap
4. Check if objects are on locked layers

#### Issue: Performance Problems

**Symptoms:**
- Low frame rate
- Stuttering during orbit
- Slow response to input

**Solutions:**
1. Reduce viewport size or use windowed mode
2. Disable real-time measurements if not needed
3. Reduce grid complexity (fewer minor lines)
4. Check Performance Monitor for bottlenecks
5. Update graphics drivers

#### Issue: Commands Not Available

**Symptoms:**
- Undo/Redo shortcuts don't work
- Command history is empty

**Solutions:**
1. Check if another operation is in progress
2. Press `Esc` to complete current operation
3. Verify command history isn't full (max 1000 commands)
4. Clear history if needed

#### Issue: Selection Problems

**Symptoms:**
- Can't select multiple objects
- Wrong object selected
- Selection highlight not visible

**Solutions:**
1. Ensure in Select tool mode (not drawing tool)
2. Check if objects are on visible layers
3. Verify selection isn't locked by another operation
4. Try clicking from Outliner panel instead

### Getting Help

If you encounter issues not covered here:

1. **Press `F1`** - Open documentation
2. **Press `F2`** - Show keyboard shortcuts
3. **Press `F3`** - Open settings dialog
4. **Help Menu** - Access built-in help topics

### Performance Monitor

Access via **View → Performance Monitor** to see:
- Frame rate (FPS)
- Frame time (ms)
- Draw calls per frame
- Memory usage
- Geometry count

Use this information to identify performance bottlenecks.

---

## Appendix

### Appendix A: Mouse Controls

| Button | Action | Description |
|---------|---------|-------------|
| **Left Click** | Select / Place point |
| **Left Drag** | Box select / Draw |
| **Right Click** | Context menu |
| **Right Drag** | Orbit camera |
| **Middle Drag** | Pan view |
| **Middle Click** | Pan to cursor |
| **Scroll Wheel** | Zoom in/out |
| **Double Click** | Confirm operation |

### Appendix B: File Formats

**Supported Import/Export Formats:**
- **TFCADIR Project** (.tfcadir) - Native format with all data
- **JSON** (.json) - Geometry data interchange
- **TODO: OBJ** (.obj) - 3D mesh format (planned)
- **TODO: STL** (.stl) - 3D printing (planned)
- **TODO: DXF** (.dxf) - CAD interchange (planned)
- **TODO: CSV** (.csv) - Point data (planned)

### Appendix C: Technical Specifications

**System Requirements:**
- OS: Linux, Windows, macOS
- RAM: 4GB+ recommended
- Graphics: OpenGL 3.3+ compatible GPU
- Display: 1920x1080 minimum

**Performance Targets:**
- 60 FPS at 1920x1080
- 30 FPS at 4K resolution
- <16ms frame time

### Appendix D: Architecture Notes

**Stateless Coordinator Pattern:**

TFCADIR uses a stateless coordinator pattern for UI management:
- **UIFSMAdapter** serves as the single source of truth for all UI state
- **NavigationManager**, **SelectionManager** are stateless - they query UIFSMAdapter for state
- **SnapManager** maintains local dirty flag for performance optimization (100ms rate limiting)

This pattern ensures:
- Consistent state across all components
- No duplicate state management
- Clear separation of concerns
- Thread-safe access via mutexes where needed

---

## References

- [TFCADIR Repository](https://github.com/MaroonSkull/TFCADIR) - Source code
- [FSMConfig Documentation](https://github.com/MaroonSkull/FSMConfig) - State machine library
- [Dear ImGui](https://github.com/ocornut/imgui) - UI framework
- [OpenGL Documentation](https://www.opengl.org/) - Graphics API
- [GLM Documentation](https://github.com/g-truc/glm) - Math library

---

**Document Information**

- **Version:** 1.0
- **Last Updated:** 2025-02-11
- **For TFCADIR Version:** Phase 8.1 (Phases 1-7 Complete)

**License:** See LICENSE file in project repository

For the latest updates and documentation, visit the TFCADIR GitHub repository.
