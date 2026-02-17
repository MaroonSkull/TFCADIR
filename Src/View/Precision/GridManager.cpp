#include <View/Precision/GridManager.hpp>

#include <algorithm>
#include <cmath>

namespace view {

// Grid size constants
namespace {
/// Default grid extent (number of lines from origin in each direction)
constexpr int DEFAULT_GRID_EXTENT = 50;

/// Axis length for rendering
constexpr float AXIS_LENGTH = 1000.0f;

/// Axis colors (standard CAD colors)
constexpr glm::vec4 X_AXIS_COLOR{1.0f, 0.0f, 0.0f, 1.0f}; // Red
constexpr glm::vec4 Y_AXIS_COLOR{0.0f, 1.0f, 0.0f, 1.0f}; // Green
constexpr glm::vec4 Z_AXIS_COLOR{0.0f, 0.5f, 1.0f, 1.0f}; // Blue
} // namespace

GridManager::GridManager(UIFSMAdapter &uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter), gridGeometryDirty_(true) {

  // Register callback to mark grid geometry dirty when settings change
  uiFSMAdapter_.setGridGeometryDirtyCallback([this]() {
    gridGeometryDirty_ = true; // THIS is the missing piece
  });
}

GridGeometry GridManager::getGridGeometry(WorkMode mode) const {
  GridGeometry geometry;

  // Query grid settings from UIFSMAdapter (stateless coordinator pattern)
  const GridSettings settings = uiFSMAdapter_.getGridSettings();

  // Set grid properties from settings
  geometry.color = settings.color;
  geometry.minorColor = settings.minorColor;
  geometry.opacity = settings.opacity;
  geometry.majorSpacing = settings.majorSpacing;
  geometry.minorSpacing = settings.showMinorLines
                              ? settings.majorSpacing / settings.minorDivisions
                              : settings.majorSpacing;

  // Calculate grid geometry based on work mode
  if (mode == WorkMode::TwoD) {
    geometry = calculate2DGrid(settings);
  } else {
    geometry = calculate3DGrid(settings);
  }

  return geometry;
}

AxisGeometry GridManager::getAxisGeometry(WorkMode mode) const {
  AxisGeometry geometry;

  // Set axis colors
  geometry.xColor = X_AXIS_COLOR;
  geometry.yColor = Y_AXIS_COLOR;
  geometry.zColor = Z_AXIS_COLOR;

  // Determine if Z axis should be shown
  geometry.showZ = (mode == WorkMode::ThreeD);

  // Create axis lines
  if (mode == WorkMode::TwoD) {
    // 2D mode: X and Y axes only
    geometry.xAxis = {glm::vec3(-AXIS_LENGTH, 0.0f, 0.0f),
                      glm::vec3(AXIS_LENGTH, 0.0f, 0.0f)};
    geometry.yAxis = {glm::vec3(0.0f, -AXIS_LENGTH, 0.0f),
                      glm::vec3(0.0f, AXIS_LENGTH, 0.0f)};
    geometry.zAxis.clear();
  } else {
    // 3D mode: X, Y, and Z axes
    geometry.xAxis = {glm::vec3(-AXIS_LENGTH, 0.0f, 0.0f),
                      glm::vec3(AXIS_LENGTH, 0.0f, 0.0f)};
    geometry.yAxis = {glm::vec3(0.0f, -AXIS_LENGTH, 0.0f),
                      glm::vec3(0.0f, AXIS_LENGTH, 0.0f)};
    geometry.zAxis = {glm::vec3(0.0f, 0.0f, -AXIS_LENGTH),
                      glm::vec3(0.0f, 0.0f, AXIS_LENGTH)};
  }

  return geometry;
}

bool GridManager::isGridVisible() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.visible;
}

bool GridManager::areAxesVisible() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.showAxes;
}

bool GridManager::isOriginVisible() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.showOrigin;
}

glm::vec4 GridManager::getGridColor() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.color;
}

glm::vec4 GridManager::getMinorGridColor() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.minorColor;
}

float GridManager::getGridOpacity() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.opacity;
}

GridGeometry GridManager::calculate2DGrid(const GridSettings &settings) const {
  GridGeometry geometry;

  // Set grid properties
  geometry.color = settings.color;
  geometry.minorColor = settings.minorColor;
  geometry.opacity = settings.opacity;
  geometry.majorSpacing = settings.majorSpacing;

  // Calculate minor spacing
  if (settings.showMinorLines && settings.minorDivisions > 0) {
    geometry.minorSpacing = settings.majorSpacing / settings.minorDivisions;
  } else {
    geometry.minorSpacing = settings.majorSpacing;
  }

  // Calculate grid extent
  const float gridExtent = settings.majorSpacing * DEFAULT_GRID_EXTENT;

  // Generate horizontal lines (along X axis)
  for (int i = -DEFAULT_GRID_EXTENT; i <= DEFAULT_GRID_EXTENT; ++i) {
    const float y = i * settings.majorSpacing;
    const bool isMajorLine = (i % settings.minorDivisions == 0);

    if (isMajorLine) {
      // Major line
      geometry.majorLines.push_back(glm::vec3(-gridExtent, y, 0.0f));
      geometry.majorLines.push_back(glm::vec3(gridExtent, y, 0.0f));
    } else if (settings.showMinorLines) {
      // Minor line
      geometry.minorLines.push_back(glm::vec3(-gridExtent, y, 0.0f));
      geometry.minorLines.push_back(glm::vec3(gridExtent, y, 0.0f));
    }
  }

  // Generate vertical lines (along Y axis)
  for (int i = -DEFAULT_GRID_EXTENT; i <= DEFAULT_GRID_EXTENT; ++i) {
    const float x = i * settings.majorSpacing;
    const bool isMajorLine = (i % settings.minorDivisions == 0);

    if (isMajorLine) {
      // Major line
      geometry.majorLines.push_back(glm::vec3(x, -gridExtent, 0.0f));
      geometry.majorLines.push_back(glm::vec3(x, gridExtent, 0.0f));
    } else if (settings.showMinorLines) {
      // Minor line
      geometry.minorLines.push_back(glm::vec3(x, -gridExtent, 0.0f));
      geometry.minorLines.push_back(glm::vec3(x, gridExtent, 0.0f));
    }
  }

  return geometry;
}

GridGeometry GridManager::calculate3DGrid(const GridSettings &settings) const {
  GridGeometry geometry;

  // Set grid properties
  geometry.color = settings.color;
  geometry.minorColor = settings.minorColor;
  geometry.opacity = settings.opacity;
  geometry.majorSpacing = settings.majorSpacing;

  // Calculate minor spacing
  if (settings.showMinorLines && settings.minorDivisions > 0) {
    geometry.minorSpacing = settings.majorSpacing / settings.minorDivisions;
  } else {
    geometry.minorSpacing = settings.majorSpacing;
  }

  // Calculate grid extent
  const float gridExtent = settings.majorSpacing * DEFAULT_GRID_EXTENT;

  // Generate ground plane grid (XZ plane)
  // Lines along X axis (varying Z)
  for (int i = -DEFAULT_GRID_EXTENT; i <= DEFAULT_GRID_EXTENT; ++i) {
    const float z = i * settings.majorSpacing;
    const bool isMajorLine = (i % settings.minorDivisions == 0);

    if (isMajorLine) {
      // Major line
      geometry.majorLines.push_back(glm::vec3(-gridExtent, 0.0f, z));
      geometry.majorLines.push_back(glm::vec3(gridExtent, 0.0f, z));
    } else if (settings.showMinorLines) {
      // Minor line
      geometry.minorLines.push_back(glm::vec3(-gridExtent, 0.0f, z));
      geometry.minorLines.push_back(glm::vec3(gridExtent, 0.0f, z));
    }
  }

  // Lines along Z axis (varying X)
  for (int i = -DEFAULT_GRID_EXTENT; i <= DEFAULT_GRID_EXTENT; ++i) {
    const float x = i * settings.majorSpacing;
    const bool isMajorLine = (i % settings.minorDivisions == 0);

    if (isMajorLine) {
      // Major line
      geometry.majorLines.push_back(glm::vec3(x, 0.0f, -gridExtent));
      geometry.majorLines.push_back(glm::vec3(x, 0.0f, gridExtent));
    } else if (settings.showMinorLines) {
      // Minor line
      geometry.minorLines.push_back(glm::vec3(x, 0.0f, -gridExtent));
      geometry.minorLines.push_back(glm::vec3(x, 0.0f, gridExtent));
    }
  }

  return geometry;
}

glm::vec2 GridManager::worldToGrid(const glm::vec3 &worldPos) const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  const float spacing = settings.majorSpacing;

  // For 2D mode: use X and Y components
  // For 3D mode: use X and Z components (ground plane)
  // Default to 2D mode for coordinate conversion
  return glm::vec2(worldPos.x / spacing, worldPos.y / spacing);
}

glm::vec3 GridManager::gridToWorld(const glm::vec2 &gridPos) const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  const float spacing = settings.majorSpacing;

  // Default to 2D mode (Z=0)
  return glm::vec3(gridPos.x * spacing, gridPos.y * spacing, 0.0f);
}

glm::vec3 GridManager::snapToGrid(const glm::vec3 &worldPos) const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  const float spacing = settings.majorSpacing;

  // Snap X and Y to grid, keep Z at 0 (2D mode default)
  const float snappedX = std::round(worldPos.x / spacing) * spacing;
  const float snappedY = std::round(worldPos.y / spacing) * spacing;
  return glm::vec3(snappedX, snappedY, 0.0f);
}

// ==========================================================================
// State Query Methods (for GridSettingsPanel)
// These methods delegate to UIFSMAdapter for state queries
// ==========================================================================

float GridManager::getMajorSpacing() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.majorSpacing;
}

float GridManager::getMinorSpacing() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  if (settings.showMinorLines && settings.minorDivisions > 0) {
    return settings.majorSpacing / settings.minorDivisions;
  }
  return settings.majorSpacing;
}

int GridManager::getMinorDivisions() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.minorDivisions;
}

bool GridManager::getShowMinorLines() const {
  const GridSettings settings = uiFSMAdapter_.getGridSettings();
  return settings.showMinorLines;
}

// ==========================================================================
// Dirty Flag Mechanism (for caching grid geometry)
// ==========================================================================

bool GridManager::isGridGeometryDirty() const { return gridGeometryDirty_; }

void GridManager::clearGridGeometryDirty() { gridGeometryDirty_ = false; }

} // namespace view
