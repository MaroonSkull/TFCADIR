# TFCADIR Keyboard Shortcuts Reference

## Overview
This document provides a comprehensive reference for all keyboard shortcuts in TFCADIR. Shortcuts are organized by category for quick lookup.

**Version:** 1.0
**Last Updated:** 2025-02-11
**For TFCADIR Version:** Phase 8.1 (Phases 1-7 Complete)

> **NOTE:** This documentation is currently located in `docs/.wip/phase-8-documentation/` as work-in-progress. After validation and review, it will be moved to the permanent documentation location.

---

## Table of Contents
- [File Operations](#file-operations) - Project management
- [Edit Operations](#edit-operations) - Undo, redo, clipboard
- [Tool Shortcuts](#tool-shortcuts) - Drawing and selection tools
- [Navigation Shortcuts](#navigation-shortcuts) - View and camera controls
- [View Presets](#view-presets) - Quick view switching
- [Selection Shortcuts](#selection-shortcuts) - Object selection
- [Precision Tools Shortcuts](#precision-tools-shortcuts) - Snap, grid, coordinates
- [Panel Shortcuts](#panel-shortcuts) - UI panel toggles
- [Help Shortcuts](#help-shortcuts) - Documentation and assistance

---

## File Operations

### Project Management

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Ctrl+N` | New Project | Create new project (clears all figures) |
| `Ctrl+O` | Open Project | Open saved project file |
| `Ctrl+S` | Save Project | Save current project to file |
| `Ctrl+Shift+S` | Save As | Save with options |
| `Ctrl+E` | Export | Export geometry to various formats |

---

## Edit Operations

### Undo/Redo

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Ctrl+Z` | Undo | Revert last action |
| `Ctrl+Y` | Redo | Restore undone action |
| `Ctrl+Shift+Z` | Undo Multiple | Undo multiple steps (5 at a time) |
| `Ctrl+Shift+Y` | Redo Multiple | Redo multiple steps (5 at a time) |

### Clipboard

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Ctrl+X` | Cut | Remove and copy to clipboard |
| `Ctrl+C` | Copy | Copy to clipboard |
| `Ctrl+V` | Paste | Paste from clipboard |
| `Ctrl+D` | Duplicate | Duplicate in place |

### Delete

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Delete` | Delete | Remove selected objects |

---

## Tool Shortcuts

### Drawing Tools

| Shortcut | Tool | Description |
|:---------|------|-------------|
| `L` | Line Tool | Draw line segments |
| `C` | Circle Tool | Draw circles |
| `A` | Arc Tool | Draw arcs |
| `R` | Rectangle Tool | Draw rectangles |
| `P` | Polygon Tool | Draw polygons |
| `N` | N-gon Tool | Draw regular polygons |

### Tool Options

When a tool is active, use these shortcuts:

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `F2` | Rename | Rename selected object |
| `Tab` | Next Option | Cycle through tool options |
| `Shift+Tab` | Previous Option | Cycle backward through options |
| `Enter` | Complete | Finish drawing operation |
| `Esc` | Cancel/Select | Cancel operation or switch to Select tool |

### Sketch Mode

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `K` | Enter Sketch Mode | Enter 2D sketching mode |
| `Esc` | Exit Sketch Mode | Exit sketch mode |

---

## Navigation Shortcuts

### Zoom Controls

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Ctrl++` or `Ctrl+Wheel Up` | Zoom In | Increase zoom level |
| `Ctrl+-` or `Ctrl+Wheel Down` | Zoom Out | Decrease zoom level |
| `Ctrl+F` | Zoom to Fit | Fit all objects in view |
| `Ctrl+Shift+F` | Zoom to Selection | Fit selected objects |

### Pan Controls

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Space + Drag` | Pan | Pan view using mouse |
| `Right Double-Click` | Pan to Cursor | Center view on cursor position |

### 3D Orbit

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Left Drag` | Orbit | Rotate around orbit center |
| `Shift+Left Drag` | Pan Orbit | Move orbit center |
| `Mouse Wheel` | Zoom | Zoom in/out during orbit |
| `Ctrl+Right Drag` | Dolly | Move camera in/out |

**Orbit Centers:**
- **Origin** - Orbit around world origin (0,0,0)
- **Selection Center** - Orbit around selected objects
- **Custom** - User-defined orbit point

---

## View Presets

Quickly switch between predefined camera views:

| Shortcut | Preset | Description |
|:---------|-------|-------------|
| `F5` | Top 2D | View from +Y direction (plan view) |
| `F6` | Front 2D | View from +Z direction (front view) |
| `F7` | Right 2D | View from +X direction (side view) |
| `F8` | Isometric | 3D isometric view |
| `F4` | Four-View | Show four simultaneous views |

**Special Views:**
- `Home` | Reset View | Reset to default view |
- `Ctrl+0-9` | Zoom Levels | Set specific zoom (1-100)

---

## Selection Shortcuts

### Selection Methods

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Left Click` | Single Select | Select one object |
| `Shift+Left Click` | Add to Selection | Add without clearing |
| `Ctrl+Left Click` | Toggle Selection | Add/remove from selection |
| `Ctrl+A` | Select All | Select all objects |
| `Esc` | Deselect All | Clear selection |
| `Shift+Drag` | Box Select | Select in rectangle region |

---

## Precision Tools Shortcuts

### Snap System

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `F9` | Snap Settings | Open snap settings dialog |
| `G` | Toggle Grid | Toggle grid visibility |
| `Shift+G` | Toggle Snap | Toggle grid snap mode |

### Coordinate Input

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `X` | Focus X | Focus X coordinate field |
| `Y` | Focus Y | Focus Y coordinate field |
| `Z` | Focus Z | Focus Z coordinate field |
| `Tab` | Next Field | Move to next input field |
| `Enter` | Apply | Apply coordinate input |

---

## Panel Shortcuts

### Panel Toggles

| Shortcut | Panel | Description |
|:---------|-------|-------------|
| `F10` | Outliner | Toggle outliner panel |
| `F11` | Properties | Toggle property inspector |
| `F12` | Command History | Toggle command history panel |
| `Ctrl+Shift+H` | Tool Options | Show/hide tool options |

### Panel Focus

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `Ctrl+Tab` | Next Panel | Cycle to next panel |
| `Ctrl+Shift+Tab` | Previous Panel | Cycle to previous panel |

---

## Help Shortcuts

| Shortcut | Action | Description |
|:---------|--------|-------------|
| `F1` | Documentation | Open user manual |
| `F2` | Keyboard Reference | Show shortcuts reference |
| `F3` | Settings | Open settings dialog |

---

## Modifier Keys

### Key Modifiers

TFCADIR supports the following modifier combinations:

| Modifier | Symbol | Description |
|:---------|-------|-------------|
| **Ctrl** | ^ | Control key |
| **Shift** | ⇧ | Shift key |
| **Alt** | ⎇ | Alternate key |
| **Ctrl+Shift** | ^⇧ | Control + Shift |
| **Ctrl+Alt** | ^⎇ | Control + Alt |
| **Shift+Alt** | ⇧⎇ | Shift + Alt |
| **Ctrl+Shift+Alt** | ^⇧⎇ | Control + Shift + Alt |

### Modifier Combinations

Common modifier patterns:
- **Ctrl+Key** - Primary command shortcuts
- **Shift+Key** - Extended or alternative actions
- **Alt+Key** - Menu access and special functions
- **Ctrl+Shift+Key** - Advanced operations

---

## Customization

All keyboard shortcuts can be customized through **Settings → Keyboard Shortcuts** or by pressing `F2`.

### Customization Rules

1. Open Settings dialog (`F3` or **Settings → Keyboard Shortcuts**)
2. Select shortcut to customize from list
3. Press new key combination to assign
4. Apply changes
5. Export/import shortcut configurations

### Default Shortcuts

The following are the default key assignments:

| Category | Shortcuts |
|-----------|----------|
| **File** | Ctrl+N, Ctrl+O, Ctrl+S, Ctrl+E |
| **Edit** | Ctrl+Z, Ctrl+Y, Ctrl+X, C, V, D |
| **Tools** | L, C, A, R, P, N, Esc |
| **Navigation** | Ctrl++/-, Ctrl+F, Space+Drag, F5-F8 |
| **Selection** | Click, Shift+Click, Ctrl+Click, Ctrl+A, Esc |
| **Precision** | F9, G, Shift+G, X, Y, Z, Tab |
| **Panels** | F10-F12, Ctrl+Tab |
| **Help** | F1, F2, F3 |

---

## Quick Reference Card

### Essential Shortcuts

```
┌─────────────────────────────────────────────────────┐
│ ESC      │ Cancel / Deselect / Select Tool          │
│ DELETE    │ Delete Selected                         │
│ ENTER     │ Complete Operation / Apply Input         │
│ SPACE     │ Select Tool / Pan                     │
│ CTRL+Z    │ Undo                                 │
│ CTRL+Y    │ Redo                                 │
│ CTRL+A    │ Select All                            │
│ CTRL+C    │ Copy                                 │
│ CTRL+V    │ Paste                                 │
│ CTRL+D    │ Duplicate                             │
│ CTRL+S    │ Save Project                          │
│ L         │ Line Tool                            │
│ C         │ Circle Tool                          │
│ A         │ Arc Tool                             │
│ R         │ Rectangle Tool                       │
│ P         │ Polygon Tool                          │
│ F5-F8     │ View Presets                          │
├─────────────────────────────────────────────────────┤
│ TAB       │ Cycle Options / Next Panel               │
│ SHIFT+TAB  │ Previous Option                         │
└─────────────────────────────────────────────────────┘
```

---

## Appendix

### Default Key Bindings by Tool

| Tool | Key | Modifier | Description |
|-------|-----|---------|-------------|
| **Select** | Space | - | Default selection tool |
| **Line** | L | - | Draw line segments |
| **Circle** | C | - | Draw circles |
| **Arc** | A | - | Draw arcs |
| **Rectangle** | R | - | Draw rectangles |
| **Polygon** | P | - | Draw polygons |
| **N-gon** | N | - | Draw regular polygons |

### Function Keys

| Key | Action | Description |
|-----|------|-------------|
| **F1** | Help | Open documentation |
| **F2** | Shortcuts | Show keyboard reference |
| **F3** | Settings | Open settings dialog |
| **F4** | Four-View | Show four-view layout |
| **F5** | Top 2D | View from +Y direction |
| **F6** | Front 2D | View from +Z direction |
| **F7** | Right 2D | View from +X direction |
| **F8** | Isometric | 3D isometric view |
| **F9** | Snap Settings | Open snap settings dialog |
| **F10-F12** | Panels | Toggle UI panels |
| **Esc** | Cancel | Cancel current operation |

---

## Searching Shortcuts

To quickly find a shortcut:

1. **By Category** - Navigate to relevant section above
2. **By Action** - Look up action in table of contents
3. **By Key** - Find all uses of specific key
4. **Using Browser Search** - Use `Ctrl+F` to find text

---

## Related Documentation

- [User Manual](user_manual.md) - Complete usage guide
- [Settings Dialog](user_manual.md#settings-and-customization) - Customization options
- [Troubleshooting](user_manual.md#troubleshooting) - Common issues

---

**Document Information**

- **Version:** 1.0
- **Last Updated:** 2025-02-11
- **For TFCADIR Version:** Phase 8.1 (Phases 1-7 Complete)

**License:** See LICENSE file in project repository

For the latest updates and complete documentation, visit the [TFCADIR GitHub repository](https://github.com/MaroonSkull/TFCADIR).
