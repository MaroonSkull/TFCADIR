#pragma once

#include <View/UIFSMAdapter.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <variant>

namespace view {

// ==========================================================================
// Phase 6: Precision & Snapping - CoordinateInputManager
// ==========================================================================

/**
 * @brief Coordinate axis enumeration
 *
 * Defines the coordinate axes for individual coordinate input.
 */
enum class CoordinateAxis {
  X, ///< X axis
  Y, ///< Y axis
  Z  ///< Z axis (for 3D coordinates)
};

/**
 * @brief Coordinate input result structure
 *
 * Contains the result of parsing coordinate input, including
 * the parsed coordinates and any error message if parsing failed.
 */
struct CoordinateInputResult {
  /// Parsed coordinate value (if successful)
  std::optional<float> value;

  /// Error message (if parsing failed)
  std::string errorMessage;

  /// Whether the result is valid
  bool isValid() const { return value.has_value(); }
};

/**
 * @brief Coordinate input manager for precise coordinate entry
 *
 * CoordinateInputManager provides coordinate input functionality with
 * expression parsing support for precise drawing operations. It manages
 * coordinate input settings, provides coordinate parsing methods, and
 * maintains a dirty flag for caching optimization.
 *
 * DESIGN RATIONALE:
 *
 * 1. Stateless Coordinator Pattern:
 *    - CoordinateInputManager maintains no coordinate input state itself
 *    - All coordinate input settings are stored in UIFSMAdapter
 *    - This follows the same pattern as GridManager, SnapManager, and
 * MeasurementManager
 *
 * 2. Caching with Dirty Flag:
 *    - coordinateInputDirty_ flag tracks when cached calculations need refresh
 *    - Set to true when coordinate input settings change
 *    - Cleared after UI panels query the current state
 *    - This prevents expensive recalculations on every frame
 *
 * 3. Callback Registration:
 *    - Constructor registers callback with UIFSMAdapter
 *    - Callback sets coordinateInputDirty_ = true when settings change
 *    - This ensures UI panels always know when to refresh coordinate input
 *
 * 4. State Query Delegation:
 *    - All state query methods delegate to UIFSMAdapter
 *    - UIFSMAdapter is the single source of truth for coordinate input settings
 *    - This maintains consistency across the application
 *
 * 5. Expression Parsing:
 *    - Implements a simple recursive descent parser
 *    - Supports basic operators: +, -, *, /
 *    - Supports parentheses for grouping
 *    - Supports functions: sin, cos, tan, sqrt, abs
 *    - Supports constants: pi, e
 *    - Does NOT use external libraries (TinyExpr integration planned for
 * Subtask 10/10)
 *
 * USAGE:
 *
 * UI panels should:
 * 1. Check isCoordinateInputDirty() to see if refresh is needed
 * 2. Call getCoordinateInputSettings() and other query methods
 * 3. Call clearCoordinateInputDirty() after updating display
 * 4. Use parseAbsoluteCoordinate(), parseRelativeCoordinate(), and
 *    parseExpression() for coordinate parsing
 *
 * @note UI panels should query coordinate input state through UIFSMAdapter,
 * not directly
 */
class CoordinateInputManager {
public:
  /**
   * @brief Constructs a CoordinateInputManager
   * @param uiFSMAdapter Reference to the UI FSM adapter
   *
   * Registers a callback with UIFSMAdapter to set coordinateInputDirty_
   * when coordinate input settings change.
   */
  explicit CoordinateInputManager(UIFSMAdapter &uiFSMAdapter);

  /**
   * @brief Destructor
   */
  ~CoordinateInputManager() = default;

  // ==========================================================================
  // State Query Methods (delegate to UIFSMAdapter)
  // ==========================================================================

  /**
   * @brief Get the current coordinate input settings
   * @return Current coordinate input settings from UIFSMAdapter
   */
  [[nodiscard]] CoordinateInputSettings getCoordinateInputSettings() const;

  /**
   * @brief Check if expression parsing is enabled
   * @return true if expression parsing is enabled
   */
  [[nodiscard]] bool isExpressionParsingEnabled() const;

  /**
   * @brief Get the coordinate precision
   * @return Number of decimal places for coordinate display
   */
  [[nodiscard]] int getPrecision() const;

  /**
   * @brief Get the angular precision
   * @return Number of decimal places for angular display
   */
  [[nodiscard]] int getAngularPrecision() const;

  /**
   * @brief Get the coordinate input mode
   * @return Current coordinate input mode
   */
  [[nodiscard]] CoordinateInputMode getInputMode() const;

  // ==========================================================================
  // Dirty Flag Management
  // ==========================================================================

  /**
   * @brief Check if coordinate input settings are dirty (need refresh)
   * @return true if cached coordinate input data needs to be refreshed
   */
  [[nodiscard]] bool isCoordinateInputDirty() const;

  /**
   * @brief Clear the coordinate input dirty flag
   *
   * Should be called by UI panels after they have refreshed their display.
   */
  void clearCoordinateInputDirty();

  // ==========================================================================
  // Coordinate Parsing Methods
  // ==========================================================================

  /**
   * @brief Parse absolute coordinate input
   * @param input Input string to parse (e.g., "100.5", "100+25.5")
   * @param axis Coordinate axis being parsed
   * @return CoordinateInputResult with parsed value or error message
   *
   * Parses absolute coordinate input. Supports:
   * - Simple numeric values: "100.5"
   * - Expressions (if enabled): "100+25.5", "50*2"
   * - Functions (if enabled): "sin(45)", "sqrt(100)"
   */
  [[nodiscard]] CoordinateInputResult
  parseAbsoluteCoordinate(const std::string &input, CoordinateAxis axis);

  /**
   * @brief Parse relative coordinate input
   * @param input Input string to parse (e.g., "@25.0", "@10+5")
   * @param axis Coordinate axis being parsed
   * @param baseValue Base value for relative calculation
   * @return CoordinateInputResult with parsed value or error message
   *
   * Parses relative coordinate input. The @ prefix indicates relative mode.
   * Supports the same expression syntax as absolute coordinates.
   */
  [[nodiscard]] CoordinateInputResult
  parseRelativeCoordinate(const std::string &input, CoordinateAxis axis,
                          float baseValue);

  /**
   * @brief Parse mathematical expression
   * @param expression Expression string to parse
   * @return CoordinateInputResult with evaluated result or error message
   *
   * Parses and evaluates mathematical expressions. Supports:
   * - Operators: +, -, *, /
   * - Parentheses: (100 + 25) * 2
   * - Functions: sin(x), cos(x), tan(x), sqrt(x), abs(x)
   * - Constants: pi, e
   *
   * All trigonometric functions use degrees.
   */
  [[nodiscard]] CoordinateInputResult
  parseExpression(const std::string &expression);

private:
  /**
   * @name State Caching
   * @brief CoordinateInputManager maintains caching state for performance
   * optimization
   * @details
   * CoordinateInputManager is NOT purely stateless - it maintains a dirty flag
   * for caching. This is an intentional architectural decision for performance:
   * - Expression parsing is expensive (requires tokenization and evaluation)
   * - Coordinate input settings change infrequently compared to frame rate
   * - Caching avoids re-parsing expressions every frame
   *
   * The dirty flag is set via callback from UIFSMAdapter when settings change.
   * This pattern is similar to GridManager's grid geometry caching.
   */
  ///@{
  /// Reference to UIFSMAdapter (non-owning)
  UIFSMAdapter &uiFSMAdapter_;

  /// Flag to track if coordinate input settings have changed
  mutable bool coordinateInputDirty_;
  ///@}

  // ==========================================================================
  // Expression Parser Implementation
  // ==========================================================================

  /**
   * @brief Token type for expression parser
   */
  enum class TokenType {
    Number,   ///< Numeric literal
    Plus,     ///< + operator
    Minus,    ///< - operator
    Multiply, ///< * operator
    Divide,   ///< / operator
    LParen,   ///< ( left parenthesis
    RParen,   ///< ) right parenthesis
    Function, ///< Function name (sin, cos, tan, sqrt, abs)
    Constant, ///< Constant name (pi, e)
    End       ///< End of input
  };

  /**
   * @brief Token structure for expression parser
   */
  struct Token {
    TokenType type;
    std::string text;
    double value; ///< For Number tokens
  };

  /**
   * @brief Tokenize the input string
   * @param input Input string to tokenize
   * @return Vector of tokens
   */
  [[nodiscard]] std::vector<Token> tokenize(const std::string &input) const;

  /**
   * @brief Parse an expression (addition/subtraction)
   * @param tokens Token vector
   * @param pos Current position in token vector
   * @return Parsed value
   */
  [[nodiscard]] double parseExpression(const std::vector<Token> &tokens,
                                       size_t &pos) const;

  /**
   * @brief Parse a term (multiplication/division)
   * @param tokens Token vector
   * @param pos Current position in token vector
   * @return Parsed value
   */
  [[nodiscard]] double parseTerm(const std::vector<Token> &tokens,
                                 size_t &pos) const;

  /**
   * @brief Parse a factor (number, parenthesis, function, constant)
   * @param tokens Token vector
   * @param pos Current position in token vector
   * @return Parsed value
   */
  [[nodiscard]] double parseFactor(const std::vector<Token> &tokens,
                                   size_t &pos) const;

  /**
   * @brief Evaluate a function
   * @param name Function name
   * @param argument Function argument
   * @return Function result
   */
  [[nodiscard]] double evaluateFunction(const std::string &name,
                                        double argument) const;

  /**
   * @brief Convert degrees to radians
   * @param degrees Angle in degrees
   * @return Angle in radians
   */
  [[nodiscard]] static double degreesToRadians(double degrees);

  /**
   * @brief Check if a string is a valid number
   * @param str String to check
   * @return true if string represents a valid number
   */
  [[nodiscard]] static bool isValidNumber(const std::string &str);

  /**
   * @brief Trim whitespace from string
   * @param str String to trim
   * @return Trimmed string
   */
  [[nodiscard]] static std::string trim(const std::string &str);
};

} // namespace view
