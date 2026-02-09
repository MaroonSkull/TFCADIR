#pragma once

#include <View/Precision/SnapManager.hpp>
#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace view {

/**
 * @brief Input mode enumeration for coordinate input
 *
 * Defines the types of coordinate input that can be processed
 * by the CoordinateInputManager.
 */
enum class InputMode {
  /// Absolute coordinates (e.g., "100.5, 50.0")
  Absolute,
  /// Relative coordinates (e.g., "@25.5, 10.0")
  Relative
};

/**
 * @brief Expression parser for mathematical expressions
 *
 * Evaluates mathematical expressions in coordinate input strings.
 * Supports basic arithmetic operations (addition, subtraction,
 * multiplication, division). Full TinyExpr integration will be
 * implemented in Subtask 10/10.
 */
class ExpressionParser {
public:
  ExpressionParser() = default;
  ~ExpressionParser() = default;

  /**
   * @brief Evaluates a mathematical expression string
   * @param expr Expression to evaluate (e.g., "100+50", "200*2")
   * @return std::optional<float> with result or std::nullopt on failure
   *
   * Evaluates basic arithmetic expressions. Returns std::nullopt
   * if the expression is invalid or cannot be parsed.
   */
  std::optional<float> evaluate(const std::string &expr) const;

private:
  /**
   * @brief Validates expression string format
   * @param expr Expression to validate
   * @return true if expression format is valid
   */
  bool isValidExpression(const std::string &expr) const;

  /**
   * @brief Evaluates basic arithmetic expression
   * @param expr Expression to evaluate
   * @return std::optional<float> with result or std::nullopt on failure
   */
  std::optional<float> evaluateBasic(const std::string &expr) const;
};

/**
 * @brief Manager for coordinate input parsing and validation
 *
 * Provides coordinate input functionality for parsing user input strings,
 * validating coordinate values, and formatting coordinates for display.
 * Follows the stateless coordinator pattern - no local domain state storage.
 *
 * CoordinateInputManager is responsible for:
 * - Parsing coordinate input strings (absolute, relative, expressions)
 * - Validating coordinate values
 * - Converting between coordinate systems
 * - Supporting mathematical expressions (basic implementation)
 * - Formatting coordinates for display
 */
class CoordinateInputManager {
public:
  /**
   * @brief Construct a new Coordinate Input Manager object
   * @param fsmAdapter Reference to UIFSMAdapter for state queries
   * @param snapManager Reference to SnapManager for snap operations
   *
   * All references are stored for state queries. CoordinateInputManager
   * does not manage the lifecycle of these components.
   */
  CoordinateInputManager(UIFSMAdapter &fsmAdapter, SnapManager &snapManager);

  /**
   * @brief Destroy the Coordinate Input Manager object
   */
  ~CoordinateInputManager() = default;

  // Coordinate parsing

  /**
   * @brief Parse coordinate input string
   * @param input Input string (e.g., "100.5, 50.0" or "100.5 50.0")
   * @param result Output parsed coordinate (X, Y, Z=0)
   * @return true if parsing succeeded, false otherwise
   *
   * Parses the input string as absolute coordinates in "X,Y" or "X Y" format.
   * Supports decimal numbers and validates the range.
   */
  bool parseCoordinateInput(const std::string &input, glm::vec3 &result) const;

  /**
   * @brief Parse relative coordinate input
   * @param input Input string starting with '@' (e.g., "@25.5, 10.0")
   * @param reference Reference point for relative calculation
   * @param result Output parsed coordinate (X, Y, Z=0)
   * @return true if parsing succeeded, false otherwise
   *
   * Parses the input string as relative coordinates starting with '@'.
   * Adds the offset to the reference point and validates the result.
   */
  bool parseRelativeCoordinate(const std::string &input,
                               const glm::vec3 &reference,
                               glm::vec3 &result) const;

  // Input parsing (stateless) - Required by architecture

  /**
   * @brief Parse expression string to coordinate
   * @param input Input string (e.g., "100+50, 200*2")
   * @return std::optional<glm::vec3> with result or std::nullopt on failure
   *
   * Parses expression string using ExpressionParser for evaluation.
   * Supports basic arithmetic operations in coordinate input.
   */
  std::optional<glm::vec3> parseExpression(const std::string &input) const;

  /**
   * @brief Evaluate input string with reference point
   * @param input Input string (absolute, relative, or expression)
   * @param referencePoint Reference point for relative coordinates
   * @return std::optional<glm::vec3> with result or std::nullopt on failure
   *
   * Evaluates input string based on format:
   * - Absolute: "100, 200"
   * - Relative: "@50, 100"
   * - Expression: "100+50, 200*2"
   */
  std::optional<glm::vec3> evaluateInput(const std::string &input,
                                         const glm::vec3 &referencePoint) const;

  /**
   * @brief Validate input format before parsing
   * @param input Input string to validate
   * @return true if input format is valid, false otherwise
   *
   * Checks for valid characters, format, and structure.
   * Validates against multiple commas, invalid characters, etc.
   */
  bool validateInput(const std::string &input) const;

  // Coordinate conversion - Required by architecture

  /**
   * @brief Convert relative coordinates to absolute
   * @param relative Relative coordinate offset
   * @param reference Reference point
   * @return Absolute coordinate
   *
   * Adds relative offset to reference point.
   */
  glm::vec3 relativeToAbsolute(const glm::vec3 &relative,
                               const glm::vec3 &reference) const;

  /**
   * @brief Convert absolute coordinates to relative
   * @param absolute Absolute coordinate
   * @param reference Reference point
   * @return Relative coordinate offset
   *
   * Subtracts reference point from absolute coordinate.
   */
  glm::vec3 absoluteToRelative(const glm::vec3 &absolute,
                               const glm::vec3 &reference) const;

  // Coordinate validation

  /**
   * @brief Validate coordinate value
   * @param value Coordinate value to validate
   * @return true if value is valid, false otherwise
   *
   * Checks if the value is finite and within a reasonable range.
   */
  bool validateCoordinate(float value) const;

  // Coordinate formatting

  /**
   * @brief Format coordinate for display
   * @param value Coordinate value to format
   * @param precision Number of decimal places (1-6, default 4)
   * @return Formatted string (e.g., "100.5")
   *
   * Formats the float value to a string with specified precision.
   * Removes trailing zeros for cleaner display.
   */
  std::string formatCoordinate(float value, int precision = 4) const;

  // Input mode management

  /**
   * @brief Set the input mode
   * @param mode New input mode (Absolute or Relative)
   *
   * Updates the input mode and notifies FSM of the state change.
   */
  void setInputMode(InputMode mode);

  /**
   * @brief Get the current input mode
   * @return Current input mode
   */
  InputMode getInputMode() const;

  // Precision management

  /**
   * @brief Set the precision for coordinate display
   * @param precision Number of decimal places (1-6)
   *
   * Validates the range and updates the precision setting.
   */
  void setPrecision(int precision);

  /**
   * @brief Get the current precision
   * @return Current precision (number of decimal places)
   */
  int getPrecision() const;

private:
  /// Reference to UIFSMAdapter for state queries
  UIFSMAdapter &fsmAdapter_;

  /// Reference to SnapManager for snap operations
  SnapManager &snapManager_;

  /// Expression parser for mathematical expressions
  ExpressionParser parser_;

  /// Current input mode (UI state, not domain state)
  InputMode inputMode_;

  /// Precision for coordinate display (UI state, not domain state)
  int precision_;

  /**
   * @brief Trims whitespace from both ends of a string
   * @param str String to trim
   * @return Trimmed string
   */
  std::string trimString(const std::string &str) const;

  /**
   * @brief Validate precision value
   * @param precision Precision value to validate
   * @return true if precision is in valid range (1-6), false otherwise
   */
  static bool isValidPrecision(int precision);
};

} // namespace view
