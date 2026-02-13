/**
 * @file DimensionTool.hpp
 * @brief Base interface for dimension and annotation tools (Phase 11)
 *
 * This file defines the base interface for all dimension and annotation tools
 * including linear, angular, radial, diameter dimensions, leader lines, and
 * text annotations. All tools follow the stateless pattern, delegating state
 * management to the FSM via UIFSMAdapter.
 */

#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace view {

// Forward declarations
class UIFSMAdapter;

/**
 * @brief Dimension arrow style options
 */
enum class ArrowStyle {
  Filled,      ///< Filled triangular arrows
  Open,        ///< Open triangular arrows
  Closed,      ///< Closed triangular arrows
  Architect,   ///< Architectural tick marks
  None         ///< No arrows
};

/**
 * @brief Dimension text position options
 */
enum class TextPosition {
  AboveLine,   ///< Text above dimension line
  OnLine,      ///< Text on dimension line (with gap)
  BelowLine    ///< Text below dimension line
};

/**
 * @brief Dimension text alignment options
 */
enum class TextAlignment {
  Center,      ///< Center aligned with dimension line
  Start,       ///< Aligned with start of dimension
  End          ///< Aligned with end of dimension
};

/**
 * @brief Base configuration for dimension tools
 */
struct DimensionStyle {
  ArrowStyle arrowStyle{ArrowStyle::Filled};
  TextPosition textPosition{TextPosition::AboveLine};
  TextAlignment textAlignment{TextAlignment::Center};
  float arrowSize{3.0f};           ///< Arrow size in pixels
  float textSize{12.0f};           ///< Text size in points
  float lineWeight{1.0f};          ///< Line weight in pixels
  float extensionLineOffset{2.0f}; ///< Offset from measured points
  float extensionLineExtension{2.0f}; ///< Extension beyond dimension line
  glm::vec4 color{0.0f, 0.0f, 0.0f, 1.0f}; ///< Dimension color
  std::string prefix;              ///< Text prefix (e.g., "R" for radius)
  std::string suffix;              ///< Text suffix (e.g., "mm")
  int precision{2};                ///< Decimal places for value display
};

/**
 * @brief Base class for dimension and annotation tools
 *
 * DimensionTool provides the common interface for all annotation tools.
 * Each specific tool type (linear, angular, radial, etc.) inherits from
 * this base and implements the pure virtual methods.
 *
 * All tools are stateless - they query the FSM via UIFSMAdapter for
 * current state and trigger FSM events for state changes.
 */
class DimensionTool {
public:
  /**
   * @brief Tool type identifiers
   */
  enum class Type {
    LinearDimension,    ///< Linear dimension tool
    AngularDimension,   ///< Angular dimension tool
    RadialDimension,    ///< Radial dimension tool
    DiameterDimension,  ///< Diameter dimension tool
    LeaderLine,         ///< Leader line tool
    TextAnnotation      ///< Text annotation tool
  };

  /**
   * @brief Construct a DimensionTool
   * @param type The tool type identifier
   * @param adapter Reference to the UIFSMAdapter for FSM queries
   */
  DimensionTool(Type type, UIFSMAdapter& adapter);

  /**
   * @brief Virtual destructor
   */
  virtual ~DimensionTool() = default;

  // Copy prohibition
  DimensionTool(const DimensionTool&) = delete;
  DimensionTool& operator=(const DimensionTool&) = delete;

  // Move permission
  DimensionTool(DimensionTool&&) noexcept = default;
  DimensionTool& operator=(DimensionTool&&) noexcept = default;

  /**
   * @brief Get the tool type
   * @return Tool type identifier
   */
  Type getType() const { return type_; }

  /**
   * @brief Get the tool ID string
   * @return Tool identifier string (e.g., "LinearDimension")
   */
  std::string getToolId() const;

  /**
   * @brief Get the tool display name
   * @return Human-readable tool name
   */
  virtual std::string getDisplayName() const = 0;

  /**
   * @brief Get the FSM event name for activating this tool
   * @return FSM event name (e.g., "OnActivateLinearDimension")
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
   * @brief Get the current dimension style
   * @return Reference to the dimension style configuration
   */
  const DimensionStyle& getStyle() const { return style_; }

  /**
   * @brief Set the dimension style
   * @param style New style configuration
   */
  void setStyle(const DimensionStyle& style) { style_ = style; }

  /**
   * @brief Handle mouse click event
   * @param position Click position in canvas coordinates
   * @param modifiers Keyboard modifiers (Ctrl, Shift, Alt)
   * @return true if the event was handled
   */
  virtual bool handleClick(const glm::vec2& position, int modifiers) = 0;

  /**
   * @brief Handle mouse move event
   * @param position Current mouse position
   * @return true if the event was handled
   */
  virtual bool handleMouseMove(const glm::vec2& position) = 0;

  /**
   * @brief Handle key press event
   * @param key Key code
   * @param modifiers Keyboard modifiers
   * @return true if the event was handled
   */
  virtual bool handleKeyPress(int key, int modifiers) = 0;

  /**
   * @brief Cancel the current operation
   */
  virtual void cancel() = 0;

  /**
   * @brief Check if the tool has a preview to render
   * @return true if preview should be rendered
   */
  virtual bool hasPreview() const = 0;

  /**
   * @brief Get preview points for rendering
   * @return Vector of points defining the preview geometry
   */
  virtual std::vector<glm::vec2> getPreviewPoints() const = 0;

  /**
   * @brief Get the number of points required to complete the dimension
   * @return Number of required points
   */
  virtual size_t getRequiredPoints() const = 0;

  /**
   * @brief Get the number of points currently collected
   * @return Number of collected points
   */
  virtual size_t getCollectedPointsCount() const = 0;

  /**
   * @brief Check if the dimension is complete
   * @return true if dimension is complete
   */
  virtual bool isComplete() const = 0;

  /**
   * @brief Get the dimension value as a formatted string
   * @return Formatted dimension value (e.g., "25.50 mm")
   */
  virtual std::string getFormattedValue() const = 0;

  /**
   * @brief Set callback for dimension completion
   * @param callback Function to call when dimension is complete
   */
  void setOnComplete(std::function<void(const std::string&)> callback) {
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
  UIFSMAdapter& adapter_;

  /// Current dimension style
  DimensionStyle style_;

  /// Callback for dimension completion
  std::function<void(const std::string&)> onComplete_;
};

/**
 * @brief Factory for creating dimension tools
 *
 * Provides a centralized way to create dimension tool instances.
 */
class DimensionToolFactory {
public:
  /**
   * @brief Create a dimension tool by type
   * @param type Tool type to create
   * @param adapter Reference to UIFSMAdapter
   * @return Unique pointer to the created tool
   */
  static std::unique_ptr<DimensionTool>
  createTool(DimensionTool::Type type, UIFSMAdapter& adapter);

  /**
   * @brief Get list of all available tool types
   * @return Vector of available tool types
   */
  static std::vector<DimensionTool::Type> getAvailableTools();

  /**
   * @brief Get display name for a tool type
   * @param type Tool type
   * @return Human-readable name
   */
  static std::string getToolDisplayName(DimensionTool::Type type);
};

//------------------------------------------------------------------------------
// Tool Class Declarations
//------------------------------------------------------------------------------

/**
 * @brief Linear dimension tool for measuring distances
 *
 * Creates linear dimensions showing distance between two points.
 * Supports horizontal, vertical, and aligned dimensions.
 */
class LinearDimensionTool : public DimensionTool {
public:
  /// Linear dimension orientation mode
  enum class Orientation {
    Aligned,    ///< Aligned with the measured line
    Horizontal, ///< Horizontal dimension
    Vertical    ///< Vertical dimension
  };

  /**
   * @brief Construct a LinearDimensionTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit LinearDimensionTool(UIFSMAdapter& adapter);

  std::string getDisplayName() const override { return "Linear Dimension"; }
  std::string getActivationEvent() const override {
    return "OnActivateLinearDimension";
  }
  bool handleClick(const glm::vec2& position, int modifiers) override;
  bool handleMouseMove(const glm::vec2& position) override;
  bool handleKeyPress(int key, int modifiers) override;
  void cancel() override;
  bool hasPreview() const override;
  std::vector<glm::vec2> getPreviewPoints() const override;
  size_t getRequiredPoints() const override { return 2; }
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Set the orientation mode
  void setOrientation(Orientation orientation);
  /// Get the current orientation mode
  Orientation getOrientation() const;
  /// Get the measured distance
  float getDistance() const;
  /// Get the dimension line angle
  float getAngle() const;
  /// Get the start point
  glm::vec2 getStartPoint() const;
  /// Get the end point
  glm::vec2 getEndPoint() const;

private:
  std::vector<glm::vec2> points_;
  glm::vec2 currentMousePos_;
  Orientation orientation_{Orientation::Aligned};
  std::pair<glm::vec2, glm::vec2>
  calculateDimensionLine(const glm::vec2& start, const glm::vec2& end,
                         const glm::vec2& mousePos) const;
  std::string formatValue(float value) const;
};

/**
 * @brief Angular dimension tool for measuring angles
 *
 * Creates angular dimensions showing angle between two lines.
 * Requires three points: vertex and two points defining the lines.
 */
class AngularDimensionTool : public DimensionTool {
public:
  /**
   * @brief Construct an AngularDimensionTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit AngularDimensionTool(UIFSMAdapter& adapter);

  std::string getDisplayName() const override { return "Angular Dimension"; }
  std::string getActivationEvent() const override {
    return "OnActivateAngularDimension";
  }
  bool handleClick(const glm::vec2& position, int modifiers) override;
  bool handleMouseMove(const glm::vec2& position) override;
  bool handleKeyPress(int key, int modifiers) override;
  void cancel() override;
  bool hasPreview() const override;
  std::vector<glm::vec2> getPreviewPoints() const override;
  size_t getRequiredPoints() const override { return 3; }
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the measured angle in degrees
  float getAngle() const;
  /// Get the vertex point
  glm::vec2 getVertex() const;
  /// Get the first line point
  glm::vec2 getFirstPoint() const;
  /// Get the second line point
  glm::vec2 getSecondPoint() const;

private:
  std::vector<glm::vec2> points_;
  glm::vec2 currentMousePos_;
  float calculateAngle(const glm::vec2& v1, const glm::vec2& v2) const;
  std::string formatValue(float value) const;
};

/**
 * @brief Radial dimension tool for measuring radii
 *
 * Creates radial dimensions showing radius of circles/arcs.
 * Requires two points: center and a point on the circle.
 */
class RadialDimensionTool : public DimensionTool {
public:
  /**
   * @brief Construct a RadialDimensionTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit RadialDimensionTool(UIFSMAdapter& adapter);

  std::string getDisplayName() const override { return "Radial Dimension"; }
  std::string getActivationEvent() const override {
    return "OnActivateRadialDimension";
  }
  bool handleClick(const glm::vec2& position, int modifiers) override;
  bool handleMouseMove(const glm::vec2& position) override;
  bool handleKeyPress(int key, int modifiers) override;
  void cancel() override;
  bool hasPreview() const override;
  std::vector<glm::vec2> getPreviewPoints() const override;
  size_t getRequiredPoints() const override { return 2; }
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the measured radius
  float getRadius() const;
  /// Get the center point
  glm::vec2 getCenter() const;
  /// Get the point on circle
  glm::vec2 getPointOnCircle() const;

private:
  std::vector<glm::vec2> points_;
  glm::vec2 currentMousePos_;
  std::string formatValue(float value) const;
};

/**
 * @brief Diameter dimension tool for measuring diameters
 *
 * Creates diameter dimensions showing diameter of circles/arcs.
 * Requires two points: center and a point on the circle.
 */
class DiameterDimensionTool : public DimensionTool {
public:
  /**
   * @brief Construct a DiameterDimensionTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit DiameterDimensionTool(UIFSMAdapter& adapter);

  std::string getDisplayName() const override { return "Diameter Dimension"; }
  std::string getActivationEvent() const override {
    return "OnActivateDiameterDimension";
  }
  bool handleClick(const glm::vec2& position, int modifiers) override;
  bool handleMouseMove(const glm::vec2& position) override;
  bool handleKeyPress(int key, int modifiers) override;
  void cancel() override;
  bool hasPreview() const override;
  std::vector<glm::vec2> getPreviewPoints() const override;
  size_t getRequiredPoints() const override { return 2; }
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the measured diameter
  float getDiameter() const;
  /// Get the center point
  glm::vec2 getCenter() const;
  /// Get the point on circle
  glm::vec2 getPointOnCircle() const;

private:
  std::vector<glm::vec2> points_;
  glm::vec2 currentMousePos_;
  std::string formatValue(float value) const;
};

/**
 * @brief Leader line tool for creating annotations with arrows
 *
 * Creates leader lines with arrows pointing to features,
 * with associated text annotations.
 */
class LeaderTool : public DimensionTool {
public:
  /**
   * @brief Construct a LeaderTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit LeaderTool(UIFSMAdapter& adapter);

  std::string getDisplayName() const override { return "Leader Line"; }
  std::string getActivationEvent() const override { return "OnActivateLeader"; }
  bool handleClick(const glm::vec2& position, int modifiers) override;
  bool handleMouseMove(const glm::vec2& position) override;
  bool handleKeyPress(int key, int modifiers) override;
  void cancel() override;
  bool hasPreview() const override;
  std::vector<glm::vec2> getPreviewPoints() const override;
  size_t getRequiredPoints() const override { return 2; }
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the arrow point (start of leader)
  glm::vec2 getArrowPoint() const;
  /// Get the text position (end of leader)
  glm::vec2 getTextPosition() const;
  /// Get the annotation text
  const std::string& getText() const { return text_; }
  /// Set the annotation text
  void setText(const std::string& text);
  /// Get the leader line length
  float getLength() const;

private:
  std::vector<glm::vec2> points_;
  glm::vec2 currentMousePos_;
  std::string text_;
  static constexpr const char* DEFAULT_TEXT = "Note";
};

/**
 * @brief Text annotation tool for creating text labels
 *
 * Creates text labels at specified positions on the canvas.
 * Supports configurable font size and text content.
 */
class TextAnnotationTool : public DimensionTool {
public:
  /**
   * @brief Construct a TextAnnotationTool
   * @param adapter Reference to UIFSMAdapter for FSM queries
   */
  explicit TextAnnotationTool(UIFSMAdapter& adapter);

  std::string getDisplayName() const override { return "Text Annotation"; }
  std::string getActivationEvent() const override {
    return "OnActivateTextAnnotation";
  }
  bool handleClick(const glm::vec2& position, int modifiers) override;
  bool handleMouseMove(const glm::vec2& position) override;
  bool handleKeyPress(int key, int modifiers) override;
  void cancel() override;
  bool hasPreview() const override;
  std::vector<glm::vec2> getPreviewPoints() const override;
  size_t getRequiredPoints() const override { return 1; }
  size_t getCollectedPointsCount() const override;
  bool isComplete() const override;
  std::string getFormattedValue() const override;

  /// Get the text position
  glm::vec2 getPosition() const;
  /// Get the annotation text
  const std::string& getText() const { return text_; }
  /// Set the annotation text
  void setText(const std::string& text);
  /// Get the font size
  float getFontSize() const { return fontSize_; }
  /// Set the font size
  void setFontSize(float size);

private:
  glm::vec2 position_;
  bool hasPosition_;
  glm::vec2 currentMousePos_;
  std::string text_;
  float fontSize_;
  static constexpr const char* DEFAULT_TEXT = "Text";
};

} // namespace view
