/**
 * @file MeasurementTool.hpp
 * @brief Base interface for measurement tools (Phase 12)
 *
 * This file defines the base interface for all measurement tools
 * including distance, area, angle, volume, coordinate, and length measurements.
 * All tools follow the stateless pattern, delegating state management to the
 * FSM via UIFSMAdapter.
 */

#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace view {

// Forward declarations
class UIFSMAdapter;

/**
 * @brief Measurement unit enumeration
 */
enum class MeasurementUnit {
  Millimeters, ///< Millimeters (mm)
  Centimeters, ///< Centimeters (cm)
  Meters,      ///< Meters (m)
  Inches       ///< Inches (in)
};

/**
 * @brief Measurement display style options
 */
struct MeasurementStyle {
  float textSize{12.0f};                   ///< Text size in points
  float lineWeight{1.0f};                  ///< Line weight in pixels
  glm::vec4 color{1.0f, 1.0f, 0.0f, 1.0f}; ///< Measurement color (yellow)
  glm::vec4 lineColor{1.0f, 1.0f, 1.0f,
                      1.0f}; ///< Line color for preview (white)
  std::string prefix;        ///< Text prefix (e.g., "R" for radius)
  std::string suffix;        ///< Text suffix (e.g., "mm")
  int precision{2};          ///< Decimal places for value display
  MeasurementUnit unit{MeasurementUnit::Millimeters}; ///< Measurement unit
  bool showPreview{true}; ///< Show measurement preview
};

/**
 * @brief Base class for measurement tools
 *
 * MeasurementTool provides the common interface for all measurement tools.
 * Each specific tool type (distance, area, angle, etc.) inherits from
 * this base and implements the pure virtual methods.
 *
 * All tools are stateless - they query the FSM via UIFSMAdapter for
 * current state and trigger FSM events for state changes.
 */
class MeasurementTool {
public:
  /**
   * @brief Tool type identifiers
   */
  enum class Type {
    Distance,   ///< Distance measurement tool
    Area,       ///< Area measurement tool
    Angle,      ///< Angle measurement tool
    Coordinate, ///< Coordinate display tool
    Length      ///< Length along path measurement tool
  };

  /**
   * @brief Construct a MeasurementTool
   * @param type The tool type identifier
   * @param adapter Reference to the UIFSMAdapter for FSM queries
   */
  MeasurementTool(Type type, UIFSMAdapter &adapter);

  /**
   * @brief Virtual destructor
   */
  virtual ~MeasurementTool() = default;

  // Copy prohibition
  MeasurementTool(const MeasurementTool &) = delete;
  MeasurementTool &operator=(const MeasurementTool &) = delete;

  // Move permission
  MeasurementTool(MeasurementTool &&) noexcept = default;
  MeasurementTool &operator=(MeasurementTool &&) noexcept = default;

  /**
   * @brief Get the tool type
   * @return Tool type identifier
   */
  Type getType() const { return type_; }

  /**
   * @brief Get the tool ID string
   * @return Tool identifier string (e.g., "DistanceMeasurement")
   */
  std::string getToolId() const;

  /**
   * @brief Get the tool display name
   * @return Human-readable tool name
   */
  virtual std::string getDisplayName() const = 0;

  /**
   * @brief Get the FSM event name for activating this tool
   * @return FSM event name (e.g., "OnDistanceMeasurement")
   */
  virtual std::string getActivationEvent() const = 0;

  /**
   * @brief Activate this tool
   *
   * Triggers the appropriate FSM event to activate this tool.
   */
  virtual void activate();

  /**
   * @brief Deactivate this tool
   *
   * Triggers the FSM event to deactivate the current tool.
   */
  virtual void deactivate();

  /**
   * @brief Check if this tool is currently active
   * @return true if this tool is active
   */
  virtual bool isActive() const;

  /**
   * @brief Get the current measurement style
   * @return Reference to the measurement style configuration
   */
  const MeasurementStyle &getStyle() const { return style_; }

  /**
   * @brief Set the measurement style
   * @param style New style configuration
   */
  void setStyle(const MeasurementStyle &style) { style_ = style; }

  /**
   * @brief Handle mouse down event
   * @param screenPos Click position in screen coordinates
   * @return true if the event was handled
   */
  virtual bool onMouseDown(const glm::vec2 &screenPos) = 0;

  /**
   * @brief Handle mouse move event
   * @param screenPos Current mouse position
   * @return true if the event was handled
   */
  virtual bool onMouseMove(const glm::vec2 &screenPos) = 0;

  /**
   * @brief Handle mouse up event
   * @param screenPos Release position in screen coordinates
   * @return true if the event was handled
   */
  virtual bool onMouseUp(const glm::vec2 &screenPos) = 0;

  /**
   * @brief Render preview of the measurement
   *
   * Called to render the visual preview of the current measurement state.
   */
  virtual void renderPreview() const = 0;

  /**
   * @brief Complete the current measurement
   *
   * Finalizes the measurement and displays the result.
   */
  virtual void completeMeasurement() = 0;

  /**
   * @brief Cancel the current measurement
   */
  virtual void cancel() = 0;

  /**
   * @brief Get the number of points currently collected
   * @return Number of collected points
   */
  virtual size_t getCollectedPointsCount() const = 0;

  /**
   * @brief Check if the measurement is complete
   * @return true if measurement is complete
   */
  virtual bool isComplete() const = 0;

  /**
   * @brief Get the measurement value as a formatted string
   * @return Formatted measurement value (e.g., "25.50 mm")
   */
  virtual std::string getFormattedValue() const = 0;

  /**
   * @brief Set callback for measurement completion
   * @param callback Function to call when measurement is complete
   */
  void setOnComplete(std::function<void(const std::string &)> callback) {
    onComplete_ = std::move(callback);
  }

  /**
   * @brief Convert tool type to string identifier
   * @param type Tool type
   * @return String identifier
   */
  static std::string typeToString(Type type);

protected:
  /// Tool type identifier
  Type type_;

  /// Reference to UIFSMAdapter for FSM queries (non-owning)
  UIFSMAdapter &adapter_;

  /// Current measurement style
  MeasurementStyle style_;

  /// Callback for measurement completion
  std::function<void(const std::string &)> onComplete_;

  /**
   * @brief Log a measurement message
   * @param message The message to log
   */
  void logMeasurement(const std::string &message) const;

  /**
   * @brief Show the measurement result
   * @param result The formatted result string
   */
  void showResult(const std::string &result) const;

  /**
   * @brief Convert a raw pixel value to the selected unit
   * @param pixelValue The value in pixels
   * @return The converted value in the selected unit
   */
  float convertToUnit(float pixelValue) const;

  /**
   * @brief Get the unit suffix string for display
   * @return The unit suffix (e.g., "mm", "cm", "m", "in")
   */
  std::string getUnitSuffix() const;

  /**
   * @brief Format a value with unit conversion and suffix
   * @param pixelValue The raw pixel value
   * @return Formatted string with converted value and unit suffix
   */
  std::string formatWithUnit(float pixelValue) const;
};

/**
 * @brief Factory for creating measurement tools
 *
 * Provides a centralized way to create measurement tool instances.
 */
class MeasurementToolFactory {
public:
  /**
   * @brief Create a measurement tool by type
   * @param type Tool type to create
   * @param adapter Reference to UIFSMAdapter
   * @return Unique pointer to the created tool
   */
  static std::unique_ptr<MeasurementTool> createTool(MeasurementTool::Type type,
                                                     UIFSMAdapter &adapter);

  /**
   * @brief Get list of all available tool types
   * @return Vector of available tool types
   */
  static std::vector<MeasurementTool::Type> getAvailableTools();

  /**
   * @brief Get display name for a tool type
   * @param type Tool type
   * @return Human-readable name
   */
  static std::string getToolDisplayName(MeasurementTool::Type type);

  /**
   * @brief Get type name for a tool type
   * @param type Tool type
   * @return Type name string
   */
  static std::string getTypeName(MeasurementTool::Type type);

  /**
   * @brief Get description for a tool type
   * @param type Tool type
   * @return Description string
   */
  static std::string getDescription(MeasurementTool::Type type);
};

//------------------------------------------------------------------------------
// Tool Class Declarations
//------------------------------------------------------------------------------

/**
 * @brief Distance measurement tool for measuring distances between points
 *
 * Creates distance measurements showing the distance between two points.
 * Supports 2D and 3D distance calculations.
 */
class DistanceMeasurementTool : public MeasurementTool {
public:
  /**
   * @brief Construct a DistanceMeasurementTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit DistanceMeasurementTool(UIFSMAdapter &adapter);

  std::string getDisplayName() const override { return "Distance Measurement"; }
  std::string getActivationEvent() const override {
    return "OnDistanceMeasurement";
  }
  bool onMouseDown(const glm::vec2 &screenPos) override;
  bool onMouseMove(const glm::vec2 &screenPos) override;
  bool onMouseUp(const glm::vec2 &screenPos) override;
  void renderPreview() const override;
  void completeMeasurement() override;
  void cancel() override;
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the measured distance
  float getDistance() const;
  /// Get the start point
  glm::vec2 getStartPoint() const;
  /// Get the end point
  glm::vec2 getEndPoint() const;

private:
  std::vector<glm::vec2> collectedPoints_;
  glm::vec2 currentMousePos_;
  float distance_;
  std::string formatValue(float value) const;
};

/**
 * @brief Area measurement tool for measuring polygon areas
 *
 * Creates area measurements showing the area of a polygon defined by
 * multiple points. Supports any number of points >= 3.
 */
class AreaMeasurementTool : public MeasurementTool {
public:
  /**
   * @brief Construct an AreaMeasurementTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit AreaMeasurementTool(UIFSMAdapter &adapter);

  std::string getDisplayName() const override { return "Area Measurement"; }
  std::string getActivationEvent() const override {
    return "OnAreaMeasurement";
  }
  bool onMouseDown(const glm::vec2 &screenPos) override;
  bool onMouseMove(const glm::vec2 &screenPos) override;
  bool onMouseUp(const glm::vec2 &screenPos) override;
  void renderPreview() const override;
  void completeMeasurement() override;
  void cancel() override;
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the measured area
  float getArea() const;
  /// Get the polygon points
  const std::vector<glm::vec2> &getPoints() const;

private:
  std::vector<glm::vec2> collectedPoints_;
  glm::vec2 currentMousePos_;
  float area_;
  std::string formatValue(float value) const;
};

/**
 * @brief Angle measurement tool for measuring angles between three points
 *
 * Creates angle measurements showing the angle between three points
 * (vertex at the second point).
 */
class AngleMeasurementTool : public MeasurementTool {
public:
  /**
   * @brief Construct an AngleMeasurementTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit AngleMeasurementTool(UIFSMAdapter &adapter);

  std::string getDisplayName() const override { return "Angle Measurement"; }
  std::string getActivationEvent() const override {
    return "OnAngleMeasurement";
  }
  bool onMouseDown(const glm::vec2 &screenPos) override;
  bool onMouseMove(const glm::vec2 &screenPos) override;
  bool onMouseUp(const glm::vec2 &screenPos) override;
  void renderPreview() const override;
  void completeMeasurement() override;
  void cancel() override;
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the measured angle in degrees
  float getAngle() const;
  /// Get the vertex point
  glm::vec2 getVertex() const;
  /// Get the first point
  glm::vec2 getFirstPoint() const;
  /// Get the second point
  glm::vec2 getSecondPoint() const;

private:
  std::vector<glm::vec2> collectedPoints_;
  glm::vec2 currentMousePos_;
  float angle_;
  float calculateAngle(const glm::vec2 &p1, const glm::vec2 &vertex,
                       const glm::vec2 &p2) const;
  std::string formatValue(float value) const;
};

/**
 * @brief Coordinate measurement tool for displaying point coordinates
 *
 * Displays the coordinates of clicked points.
 */
class CoordinateMeasurementTool : public MeasurementTool {
public:
  /**
   * @brief Construct a CoordinateMeasurementTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit CoordinateMeasurementTool(UIFSMAdapter &adapter);

  std::string getDisplayName() const override {
    return "Coordinate Measurement";
  }
  std::string getActivationEvent() const override {
    return "OnCoordinateMeasurement";
  }
  bool onMouseDown(const glm::vec2 &screenPos) override;
  bool onMouseMove(const glm::vec2 &screenPos) override;
  bool onMouseUp(const glm::vec2 &screenPos) override;
  void renderPreview() const override;
  void completeMeasurement() override;
  void cancel() override;
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the last coordinate
  glm::vec2 getLastCoordinate() const;
  /// Get all collected coordinates
  const std::vector<glm::vec2> &getCoordinates() const;

private:
  std::vector<glm::vec2> collectedPoints_;
  glm::vec2 currentMousePos_;
  std::string formatCoordinate(const glm::vec2 &coord) const;
};

/**
 * @brief Length measurement tool for measuring path lengths
 *
 * Measures the total length along a path defined by multiple points.
 */
class LengthMeasurementTool : public MeasurementTool {
public:
  /**
   * @brief Construct a LengthMeasurementTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit LengthMeasurementTool(UIFSMAdapter &adapter);

  std::string getDisplayName() const override { return "Length Measurement"; }
  std::string getActivationEvent() const override {
    return "OnLengthMeasurement";
  }
  bool onMouseDown(const glm::vec2 &screenPos) override;
  bool onMouseMove(const glm::vec2 &screenPos) override;
  bool onMouseUp(const glm::vec2 &screenPos) override;
  void renderPreview() const override;
  void completeMeasurement() override;
  void cancel() override;
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the total path length
  float getLength() const;
  /// Get the path points
  const std::vector<glm::vec2> &getPoints() const;

private:
  std::vector<glm::vec2> collectedPoints_;
  glm::vec2 currentMousePos_;
  float length_;
  std::string formatValue(float value) const;
};

} // namespace view
