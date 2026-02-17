#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace view {

// ==========================================================================
// Phase 10: Advanced Selection Types
// These types define selection modes, modifiers, and filters
// ==========================================================================

/**
 * @brief Selection mode enumeration
 *
 * Defines the available selection modes for picking objects in the canvas.
 */
enum class SelectionMode {
  Point, ///< Single point click selection (default)
  Box,   ///< Rectangle drag selection (left-to-right: intersect, right-to-left:
         ///< contain)
  Lasso, ///< Freeform lasso selection
  Polygon ///< Polygon selection (click to add vertices, double-click to close)
};

/**
 * @brief Selection modifier enumeration
 *
 * Defines how the new selection interacts with the existing selection.
 * These correspond to keyboard modifier keys.
 */
enum class SelectionModifier {
  None,   ///< Replace current selection (no modifier)
  Add,    ///< Add to current selection (Shift key)
  Toggle, ///< Toggle selection state (Ctrl key)
  Remove  ///< Remove from current selection (Alt key)
};

/**
 * @brief Figure type filter enumeration
 *
 * Defines the figure types that can be filtered during selection.
 */
enum class FigureTypeFilter {
  None = 0,
  Triangle = 1 << 0,
  Quad = 1 << 1,
  Circle = 1 << 2,
  Ngon = 1 << 3,
  CurveBezier3 = 1 << 4,
  CurveBezier4 = 1 << 5,
  All = Triangle | Quad | Circle | Ngon | CurveBezier3 | CurveBezier4
};

/**
 * @brief Bitwise OR operator for FigureTypeFilter
 */
inline FigureTypeFilter operator|(FigureTypeFilter a, FigureTypeFilter b) {
  return static_cast<FigureTypeFilter>(static_cast<int>(a) |
                                       static_cast<int>(b));
}

/**
 * @brief Bitwise AND operator for FigureTypeFilter
 */
inline FigureTypeFilter operator&(FigureTypeFilter a, FigureTypeFilter b) {
  return static_cast<FigureTypeFilter>(static_cast<int>(a) &
                                       static_cast<int>(b));
}

/**
 * @brief Bitwise OR assignment operator for FigureTypeFilter
 */
inline FigureTypeFilter &operator|=(FigureTypeFilter &a, FigureTypeFilter b) {
  a = a | b;
  return a;
}

/**
 * @brief Check if a figure type filter includes a specific type
 */
inline bool hasFigureType(FigureTypeFilter flags, FigureTypeFilter type) {
  return (flags & type) == type;
}

/**
 * @brief Selection filter configuration
 *
 * Contains all filter criteria for advanced selection operations.
 */
struct SelectionFilter {
  /// Figure types to include in selection (default: All)
  FigureTypeFilter figureTypes = FigureTypeFilter::All;

  /// Layer indices to include in selection (empty = all layers)
  std::set<int> layers;

  /// Whether to filter by visibility (true = only visible figures)
  bool visibleOnly = true;

  /// Whether to filter by lock state (true = exclude locked figures)
  bool excludeLocked = true;

  /// Minimum area threshold for selection (0 = no minimum)
  float minArea = 0.0f;

  /// Maximum area threshold for selection (0 = no maximum)
  float maxArea = 0.0f;

  /// Name pattern filter (empty = no filter, supports wildcards)
  std::string namePattern;

  /**
   * @brief Check if layer filtering is active
   * @return true if specific layers are selected
   */
  bool hasLayerFilter() const { return !layers.empty(); }

  /**
   * @brief Check if a layer is included in the filter
   * @param layerIndex The layer index to check
   * @return true if the layer is included or no layer filter is set
   */
  bool isLayerIncluded(int layerIndex) const {
    return layers.empty() || layers.count(layerIndex) > 0;
  }

  /**
   * @brief Reset all filters to default values
   */
  void reset() {
    figureTypes = FigureTypeFilter::All;
    layers.clear();
    visibleOnly = true;
    excludeLocked = true;
    minArea = 0.0f;
    maxArea = 0.0f;
    namePattern.clear();
  }
};

/**
 * @brief Selection memory entry for persistent selection
 *
 * Stores a named selection set that can be recalled later.
 */
struct SelectionMemoryEntry {
  /// Unique identifier for this selection memory
  uint32_t id;

  /// User-defined name for this selection
  std::string name;

  /// Figure IDs in this selection
  std::vector<uint32_t> figureIds;

  /// Timestamp when this selection was saved (ISO 8601)
  std::string savedAt;
};

/**
 * @brief Selection memory configuration
 *
 * Manages persistent selection sets that can be saved and recalled.
 */
struct SelectionMemory {
  /// Maximum number of saved selections
  static constexpr size_t MAX_SAVED_SELECTIONS = 20;

  /// Saved selection entries
  std::vector<SelectionMemoryEntry> savedSelections;

  /// Next unique ID for saved selections
  uint32_t nextId = 1;

  /**
   * @brief Save the current selection with a name
   * @param name User-defined name for the selection
   * @param figureIds Figure IDs to save
   * @return ID of the saved selection, or 0 if failed
   */
  uint32_t saveSelection(const std::string &name,
                         const std::vector<uint32_t> &figureIds) {
    if (savedSelections.size() >= MAX_SAVED_SELECTIONS) {
      /// Remove oldest entry
      savedSelections.erase(savedSelections.begin());
    }

    SelectionMemoryEntry entry;
    entry.id = nextId++;
    entry.name = name;
    entry.figureIds = figureIds;

    /// Generate timestamp (thread-safe)
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuffer;
    localtime_r(&time, &tmBuffer);
    std::stringstream ss;
    ss << std::put_time(&tmBuffer, "%Y-%m-%dT%H:%M:%S");
    entry.savedAt = ss.str();

    savedSelections.push_back(entry);
    return entry.id;
  }

  /**
   * @brief Delete a saved selection by ID
   * @param id ID of the selection to delete
   * @return true if found and deleted
   */
  bool deleteSelection(uint32_t id) {
    auto it = std::find_if(
        savedSelections.begin(), savedSelections.end(),
        [id](const SelectionMemoryEntry &e) { return e.id == id; });
    if (it != savedSelections.end()) {
      savedSelections.erase(it);
      return true;
    }
    return false;
  }

  /**
   * @brief Get a saved selection by ID
   * @param id ID of the selection to retrieve
   * @return Pointer to the entry, or nullptr if not found
   */
  const SelectionMemoryEntry *getSelection(uint32_t id) const {
    auto it = std::find_if(
        savedSelections.begin(), savedSelections.end(),
        [id](const SelectionMemoryEntry &e) { return e.id == id; });
    return it != savedSelections.end() ? &(*it) : nullptr;
  }

  /**
   * @brief Clear all saved selections
   */
  void clear() { savedSelections.clear(); }
};

/**
 * @brief Selection geometry for box/lasso/polygon selection
 *
 * Stores the geometric data for the current selection operation.
 */
struct SelectionGeometry {
  /// Selection mode for this geometry
  SelectionMode mode = SelectionMode::Point;

  /// Start point for box selection (screen coordinates)
  glm::vec2 startPoint{0.0f, 0.0f};

  /// End point for box selection (screen coordinates)
  glm::vec2 endPoint{0.0f, 0.0f};

  /// Points for lasso/polygon selection (screen coordinates)
  std::vector<glm::vec2> points;

  /// Whether the selection is complete (for polygon: closed)
  bool isComplete = false;

  /**
   * @brief Reset the selection geometry
   */
  void reset() {
    mode = SelectionMode::Point;
    startPoint = glm::vec2(0.0f, 0.0f);
    endPoint = glm::vec2(0.0f, 0.0f);
    points.clear();
    isComplete = false;
  }

  /**
   * @brief Get the bounding box for box selection
   * @return Pair of (min, max) corners
   */
  std::pair<glm::vec2, glm::vec2> getBoundingBox() const {
    return {glm::min(startPoint, endPoint), glm::max(startPoint, endPoint)};
  }

  /**
   * @brief Check if box selection is from left-to-right (intersect mode)
   * @return true if left-to-right selection (intersect), false if right-to-left
   * (contain)
   */
  bool isLeftToRight() const { return endPoint.x >= startPoint.x; }
};

/**
 * @brief Selection settings for the application
 *
 * Contains configuration for selection behavior and visual feedback.
 */
struct SelectionSettings {
  /// Current selection mode
  SelectionMode mode = SelectionMode::Point;

  /// Current selection filter
  SelectionFilter filter;

  /// Selection tolerance in pixels for point selection
  float tolerancePixels = 5.0f;

  /// Whether to show selection preview during drag
  bool showPreview = true;

  /// Selection preview color (RGBA)
  glm::vec4 previewColor = glm::vec4(0.0f, 0.5f, 1.0f, 0.3f);

  /// Selection preview border color (RGBA)
  glm::vec4 previewBorderColor = glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);

  /// Selection highlight color (RGBA)
  glm::vec4 highlightColor = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);

  /// Whether selection memory is enabled
  bool memoryEnabled = true;

  /**
   * @brief Equality operator for SelectionSettings
   */
  bool operator==(const SelectionSettings &other) const {
    return mode == other.mode && tolerancePixels == other.tolerancePixels &&
           showPreview == other.showPreview &&
           previewColor == other.previewColor &&
           previewBorderColor == other.previewBorderColor &&
           highlightColor == other.highlightColor &&
           memoryEnabled == other.memoryEnabled;
  }
};

} // namespace view
