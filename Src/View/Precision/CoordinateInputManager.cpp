#include <View/Precision/CoordinateInputManager.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stack>
#include <string>

namespace view {

// Constants for coordinate validation
namespace {
/// Minimum reasonable coordinate value (millimeters)
constexpr float MIN_COORDINATE = -1000000.0f;

/// Maximum reasonable coordinate value (millimeters)
constexpr float MAX_COORDINATE = 1000000.0f;

/// Default precision for coordinate display
constexpr int DEFAULT_PRECISION = 4;

/// Minimum precision value
constexpr int MIN_PRECISION = 1;

/// Maximum precision value
constexpr int MAX_PRECISION = 6;
} // namespace

// ============================================================================
// ExpressionParser Implementation
// ============================================================================

bool ExpressionParser::isValidExpression(const std::string &expr) const {
  if (expr.empty()) {
    return false;
  }

  // Check for valid characters: digits, operators, parentheses, decimal point
  for (char c : expr) {
    if (!std::isdigit(c) && c != '+' && c != '-' && c != '*' && c != '/' &&
        c != '.' && c != '(' && c != ')' && c != ' ' && c != '\t') {
      return false;
    }
  }

  // Check for balanced parentheses
  int balance = 0;
  for (char c : expr) {
    if (c == '(') {
      balance++;
    } else if (c == ')') {
      balance--;
      if (balance < 0) {
        return false;
      }
    }
  }
  if (balance != 0) {
    return false;
  }

  return true;
}

std::optional<float>
ExpressionParser::evaluateBasic(const std::string &expr) const {
  try {
    // Simple expression evaluation using stringstream
    // This is a basic implementation - full TinyExpr integration in Subtask
    // 10/10
    std::istringstream iss(expr);
    float result = 0.0f;
    char op = '+';
    std::stack<float> values;
    std::stack<char> ops;

    auto applyOperation = [](float a, float b, char op) -> float {
      switch (op) {
      case '+':
        return a + b;
      case '-':
        return a - b;
      case '*':
        return a * b;
      case '/':
        if (b == 0.0f) {
          return std::numeric_limits<float>::quiet_NaN();
        }
        return a / b;
      default:
        return b;
      }
    };

    auto precedence = [](char op) -> int {
      if (op == '+' || op == '-') {
        return 1;
      }
      if (op == '*' || op == '/') {
        return 2;
      }
      return 0;
    };

    float value;
    while (iss >> value) {
      values.push(value);

      char nextOp;
      if (iss >> nextOp) {
        while (!ops.empty() && precedence(ops.top()) >= precedence(nextOp)) {
          if (values.size() < 2) {
            return std::nullopt;
          }
          float b = values.top();
          values.pop();
          float a = values.top();
          values.pop();
          char op = ops.top();
          ops.pop();
          float result = applyOperation(a, b, op);
          if (!std::isfinite(result)) {
            return std::nullopt;
          }
          values.push(result);
        }
        ops.push(nextOp);
      } else {
        break;
      }
    }

    while (!ops.empty()) {
      if (values.size() < 2) {
        return std::nullopt;
      }
      float b = values.top();
      values.pop();
      float a = values.top();
      values.pop();
      char op = ops.top();
      ops.pop();
      float result = applyOperation(a, b, op);
      if (!std::isfinite(result)) {
        return std::nullopt;
      }
      values.push(result);
    }

    if (values.size() != 1 || !ops.empty()) {
      return std::nullopt;
    }

    return values.top();
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<float> ExpressionParser::evaluate(const std::string &expr) const {
  if (!isValidExpression(expr)) {
    return std::nullopt;
  }

  return evaluateBasic(expr);
}

// ============================================================================
// CoordinateInputManager Implementation
// ============================================================================

CoordinateInputManager::CoordinateInputManager(UIFSMAdapter &fsmAdapter,
                                               SnapManager &snapManager)
    : fsmAdapter_(fsmAdapter), snapManager_(snapManager),
      inputMode_(InputMode::Absolute), precision_(DEFAULT_PRECISION) {}

// ============================================================================
// Coordinate Parsing
// ============================================================================

bool CoordinateInputManager::parseCoordinateInput(const std::string &input,
                                                  glm::vec3 &result) const {
  std::string trimmed = trimString(input);

  if (trimmed.empty()) {
    return false;
  }

  // Check if this is a relative coordinate (starts with @)
  if (trimmed[0] == '@') {
    // Relative coordinates require a reference point
    // For now, return false - user should use parseRelativeCoordinate
    return false;
  }

  // Try to parse as "X,Y" or "X Y" format
  std::istringstream iss(trimmed);
  float x = 0.0f, y = 0.0f;
  char separator = 0;

  // Try to parse X value
  if (!(iss >> x)) {
    return false;
  }

  // Try to parse separator (comma or space)
  iss >> separator;

  // Try to parse Y value
  if (!(iss >> y)) {
    // If only one value provided, use it for X and set Y to 0
    y = 0.0f;
  }

  // Validate coordinates
  if (!validateCoordinate(x) || !validateCoordinate(y)) {
    return false;
  }

  // Set result (Z is always 0 for 2D coordinates)
  result = glm::vec3(x, y, 0.0f);
  return true;
}

bool CoordinateInputManager::parseRelativeCoordinate(const std::string &input,
                                                     const glm::vec3 &reference,
                                                     glm::vec3 &result) const {
  std::string trimmed = trimString(input);

  if (trimmed.empty()) {
    return false;
  }

  // Check if this starts with @ (relative coordinate indicator)
  if (trimmed[0] != '@') {
    return false;
  }

  // Remove the @ symbol
  trimmed.erase(0, 1);

  // Trim again after removing @
  trimmed = trimString(trimmed);

  // Try to parse offset as "X,Y" or "X Y" format
  std::istringstream iss(trimmed);
  float offsetX = 0.0f, offsetY = 0.0f;
  char separator = 0;

  // Try to parse X offset
  if (!(iss >> offsetX)) {
    return false;
  }

  // Try to parse separator (comma or space)
  iss >> separator;

  // Try to parse Y offset
  if (!(iss >> offsetY)) {
    // If only one value provided, use it for X and set Y to 0
    offsetY = 0.0f;
  }

  // Calculate result by adding offset to reference
  result = glm::vec3(reference.x + offsetX, reference.y + offsetY, reference.z);

  // Validate result
  if (!validateCoordinate(result.x) || !validateCoordinate(result.y)) {
    return false;
  }

  return true;
}

// ============================================================================
// Input Parsing (Required by Architecture)
// ============================================================================

std::optional<glm::vec3>
CoordinateInputManager::parseExpression(const std::string &input) const {
  std::string trimmed = trimString(input);

  if (trimmed.empty()) {
    return std::nullopt;
  }

  // Check if this is a relative coordinate
  if (trimmed[0] == '@') {
    return std::nullopt;
  }

  // Split by comma or space to get X and Y components
  std::istringstream iss(trimmed);
  std::string xStr, yStr;

  // Try to parse X component
  if (!std::getline(iss, xStr, ',') && !std::getline(iss, xStr, ' ')) {
    return std::nullopt;
  }
  xStr = trimString(xStr);

  // Try to parse Y component
  std::getline(iss, yStr);
  yStr = trimString(yStr);

  // If Y is empty, set to 0
  if (yStr.empty()) {
    yStr = "0";
  }

  // Evaluate expressions using ExpressionParser
  std::optional<float> x = parser_.evaluate(xStr);
  if (!x.has_value()) {
    return std::nullopt;
  }

  std::optional<float> y = parser_.evaluate(yStr);
  if (!y.has_value()) {
    return std::nullopt;
  }

  // Validate coordinates
  if (!validateCoordinate(x.value()) || !validateCoordinate(y.value())) {
    return std::nullopt;
  }

  return glm::vec3(x.value(), y.value(), 0.0f);
}

std::optional<glm::vec3>
CoordinateInputManager::evaluateInput(const std::string &input,
                                      const glm::vec3 &referencePoint) const {
  std::string trimmed = trimString(input);

  if (trimmed.empty()) {
    return std::nullopt;
  }

  // Check if this is a relative coordinate (starts with @)
  if (trimmed[0] == '@') {
    // Parse as relative coordinate
    glm::vec3 result;
    if (parseRelativeCoordinate(input, referencePoint, result)) {
      return result;
    }
    return std::nullopt;
  }

  // Try to parse as expression first
  auto exprResult = parseExpression(input);
  if (exprResult.has_value()) {
    return exprResult;
  }

  // Fall back to simple coordinate parsing
  glm::vec3 result;
  if (parseCoordinateInput(input, result)) {
    return result;
  }

  return std::nullopt;
}

bool CoordinateInputManager::validateInput(const std::string &input) const {
  std::string trimmed = trimString(input);

  if (trimmed.empty()) {
    return false;
  }

  // Check for multiple consecutive commas
  if (trimmed.find(",,") != std::string::npos) {
    return false;
  }

  // Check for invalid characters
  for (char c : trimmed) {
    // Allow: digits, operators, parentheses, decimal point, @, comma, space,
    // tab
    if (!std::isdigit(c) && c != '+' && c != '-' && c != '*' && c != '/' &&
        c != '.' && c != '(' && c != ')' && c != '@' && c != ',' && c != ' ' &&
        c != '\t' && c != '\n' && c != '\r') {
      return false;
    }
  }

  // Check for balanced parentheses
  int balance = 0;
  for (char c : trimmed) {
    if (c == '(') {
      balance++;
    } else if (c == ')') {
      balance--;
      if (balance < 0) {
        return false;
      }
    }
  }
  if (balance != 0) {
    return false;
  }

  // Check if @ is only at the beginning (for relative coordinates)
  size_t atPos = trimmed.find('@');
  if (atPos != std::string::npos && atPos != 0) {
    return false;
  }

  return true;
}

// ============================================================================
// Coordinate Conversion (Required by Architecture)
// ============================================================================

glm::vec3
CoordinateInputManager::relativeToAbsolute(const glm::vec3 &relative,
                                           const glm::vec3 &reference) const {
  return reference + relative;
}

glm::vec3
CoordinateInputManager::absoluteToRelative(const glm::vec3 &absolute,
                                           const glm::vec3 &reference) const {
  return absolute - reference;
}

// ============================================================================
// Coordinate Validation
// ============================================================================

bool CoordinateInputManager::validateCoordinate(float value) const {
  // Check if value is finite (not NaN or infinity)
  if (!std::isfinite(value)) {
    return false;
  }

  // Check if value is within reasonable range
  if (value < MIN_COORDINATE || value > MAX_COORDINATE) {
    return false;
  }

  return true;
}

// ============================================================================
// Coordinate Formatting
// ============================================================================

std::string CoordinateInputManager::formatCoordinate(float value,
                                                     int precision) const {
  // Clamp precision to valid range
  precision = std::clamp(precision, MIN_PRECISION, MAX_PRECISION);

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(precision) << value;

  std::string result = oss.str();

  // Remove trailing zeros
  size_t dotPos = result.find('.');
  if (dotPos != std::string::npos) {
    size_t lastNonZero = result.find_last_not_of('0');
    if (lastNonZero != std::string::npos && lastNonZero > dotPos) {
      result.erase(lastNonZero + 1);
    } else if (lastNonZero == dotPos) {
      // Remove the decimal point if no decimal digits
      result.erase(dotPos);
    }
  }

  return result;
}

// ============================================================================
// Input Mode Management
// ============================================================================

void CoordinateInputManager::setInputMode(InputMode mode) {
  inputMode_ = mode;
  // FSM state update would be triggered here if needed
}

InputMode CoordinateInputManager::getInputMode() const { return inputMode_; }

// ============================================================================
// Precision Management
// ============================================================================

void CoordinateInputManager::setPrecision(int precision) {
  if (isValidPrecision(precision)) {
    precision_ = precision;
  }
}

int CoordinateInputManager::getPrecision() const { return precision_; }

// ============================================================================
// Private Helper Methods
// ============================================================================

std::string CoordinateInputManager::trimString(const std::string &str) const {
  std::string trimmed = str;
  trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
  trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
  return trimmed;
}

bool CoordinateInputManager::isValidPrecision(int precision) {
  return precision >= MIN_PRECISION && precision <= MAX_PRECISION;
}

} // namespace view
