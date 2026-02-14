#include <View/Precision/CoordinateInputManager.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <unordered_map>

namespace view {

// Mathematical constants
namespace {
constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;
} // namespace

CoordinateInputManager::CoordinateInputManager(UIFSMAdapter &uiFSMAdapter)
    : uiFSMAdapter_(uiFSMAdapter), coordinateInputDirty_(true) {

  /// Register callback to mark coordinate input dirty when settings change
  uiFSMAdapter_.setCoordinateInputSettingsChangedCallback(
      [this]() { this->coordinateInputDirty_ = true; });
}

// ==========================================================================
// State Query Methods (delegate to UIFSMAdapter)
// ==========================================================================

CoordinateInputSettings
CoordinateInputManager::getCoordinateInputSettings() const {
  /// Query coordinate input settings from UIFSMAdapter (stateless coordinator
  /// pattern)
  return uiFSMAdapter_.getCoordinateInputSettings();
}

bool CoordinateInputManager::isExpressionParsingEnabled() const {
  const auto settings = getCoordinateInputSettings();
  return settings.expressionParsingEnabled;
}

int CoordinateInputManager::getPrecision() const {
  const auto settings = getCoordinateInputSettings();
  return settings.precision;
}

int CoordinateInputManager::getAngularPrecision() const {
  const auto settings = getCoordinateInputSettings();
  return settings.angularPrecision;
}

CoordinateInputMode CoordinateInputManager::getInputMode() const {
  const auto settings = getCoordinateInputSettings();
  return settings.inputMode;
}

// ==========================================================================
// Dirty Flag Management
// ==========================================================================

bool CoordinateInputManager::isCoordinateInputDirty() const {
  return coordinateInputDirty_;
}

void CoordinateInputManager::clearCoordinateInputDirty() {
  coordinateInputDirty_ = false;
}

// ==========================================================================
// Coordinate Parsing Methods
// ==========================================================================

CoordinateInputResult
CoordinateInputManager::parseAbsoluteCoordinate(const std::string &input,
                                                CoordinateAxis axis) {
  // Trim whitespace from input
  std::string trimmed = trim(input);

  // Check if empty
  if (trimmed.empty()) {
    return CoordinateInputResult{std::nullopt, "Input is empty"};
  }

  // Check for relative prefix (@)
  if (!trimmed.empty() && trimmed[0] == '@') {
    return CoordinateInputResult{std::nullopt,
                                 "Relative prefix (@) not allowed in absolute "
                                 "coordinate mode. Use parseRelativeCoordinate "
                                 "instead."};
  }

  // If expression parsing is enabled, try to parse as expression
  if (isExpressionParsingEnabled()) {
    // Check if input contains operators or functions
    bool hasExpression = trimmed.find('+') != std::string::npos ||
                         trimmed.find('-') != std::string::npos ||
                         trimmed.find('*') != std::string::npos ||
                         trimmed.find('/') != std::string::npos ||
                         trimmed.find('(') != std::string::npos;

    // Check for function names
    std::string lowerInput = trimmed;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    hasExpression = hasExpression ||
                    lowerInput.find("sin") != std::string::npos ||
                    lowerInput.find("cos") != std::string::npos ||
                    lowerInput.find("tan") != std::string::npos ||
                    lowerInput.find("sqrt") != std::string::npos ||
                    lowerInput.find("abs") != std::string::npos;

    if (hasExpression) {
      return parseExpression(trimmed);
    }
  }

  // Try to parse as simple number
  if (!isValidNumber(trimmed)) {
    return CoordinateInputResult{std::nullopt,
                                 "Invalid number format: " + trimmed};
  }

  try {
    float value = std::stof(trimmed);

    // Validate coordinate range
    if (std::abs(value) > 1e6f) {
      return CoordinateInputResult{std::nullopt,
                                   "Coordinate value too large: " + trimmed};
    }

    return CoordinateInputResult{value, ""};
  } catch (const std::exception &) {
    return CoordinateInputResult{std::nullopt,
                                 "Failed to parse number: " + trimmed};
  }
}

CoordinateInputResult CoordinateInputManager::parseRelativeCoordinate(
    const std::string &input, CoordinateAxis axis, float baseValue) {
  // Trim whitespace from input
  std::string trimmed = trim(input);

  // Check if empty
  if (trimmed.empty()) {
    return CoordinateInputResult{std::nullopt, "Input is empty"};
  }

  // Check for relative prefix (@)
  bool hasPrefix = false;
  if (!trimmed.empty() && trimmed[0] == '@') {
    hasPrefix = true;
    trimmed = trim(trimmed.substr(1));
  }

  // If expression parsing is enabled, try to parse as expression
  if (isExpressionParsingEnabled()) {
    // Check if input contains operators or functions
    bool hasExpression = trimmed.find('+') != std::string::npos ||
                         trimmed.find('-') != std::string::npos ||
                         trimmed.find('*') != std::string::npos ||
                         trimmed.find('/') != std::string::npos ||
                         trimmed.find('(') != std::string::npos;

    // Check for function names
    std::string lowerInput = trimmed;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    hasExpression = hasExpression ||
                    lowerInput.find("sin") != std::string::npos ||
                    lowerInput.find("cos") != std::string::npos ||
                    lowerInput.find("tan") != std::string::npos ||
                    lowerInput.find("sqrt") != std::string::npos ||
                    lowerInput.find("abs") != std::string::npos;

    if (hasExpression) {
      auto result = parseExpression(trimmed);
      if (result.isValid()) {
        // Add to base value for relative coordinate
        float value = *result.value + baseValue;
        return CoordinateInputResult{value, ""};
      }
      return result;
    }
  }

  // Try to parse as simple number
  if (!isValidNumber(trimmed)) {
    return CoordinateInputResult{std::nullopt,
                                 "Invalid number format: " + trimmed};
  }

  try {
    float offset = std::stof(trimmed);

    // Validate offset range
    if (std::abs(offset) > 1e6f) {
      return CoordinateInputResult{std::nullopt,
                                   "Offset value too large: " + trimmed};
    }

    // Add to base value for relative coordinate
    float value = offset + baseValue;
    return CoordinateInputResult{value, ""};
  } catch (const std::exception &) {
    return CoordinateInputResult{std::nullopt,
                                 "Failed to parse number: " + trimmed};
  }
}

CoordinateInputResult
CoordinateInputManager::parseExpression(const std::string &expression) {
  // Trim whitespace from input
  std::string trimmed = trim(expression);

  // Check if empty
  if (trimmed.empty()) {
    return CoordinateInputResult{std::nullopt, "Expression is empty"};
  }

  // Use ExpressionEvaluator for expression evaluation
  std::optional<double> result = expressionEvaluator_.evaluate(trimmed);

  if (!result.has_value()) {
    // Return error message from ExpressionEvaluator
    return CoordinateInputResult{std::nullopt,
                                 expressionEvaluator_.getLastError()};
  }

  double value = *result;

  // Validate result range
  if (std::abs(value) > 1e6) {
    return CoordinateInputResult{std::nullopt, "Expression result too large"};
  }

  return CoordinateInputResult{static_cast<float>(value), ""};
}

bool CoordinateInputManager::isValidNumber(const std::string &str) {
  if (str.empty()) {
    return false;
  }

  std::istringstream iss(str);
  double value;
  iss >> value;

  // Check if entire string was consumed and no error occurred
  return iss.eof() && !iss.fail();
}

std::string CoordinateInputManager::trim(const std::string &str) {
  /// Use standard library utilities for more idiomatic implementation
  /// find_first_not_of() returns the position of the first character that
  /// does NOT match any character in the given set (whitespace)
  size_t start = str.find_first_not_of(" \t\n\r\f\v");

  /// If the string is all whitespace or empty, return empty string
  if (start == std::string::npos) {
    return "";
  }

  /// find_last_not_of() returns the position of the last character that
  /// does NOT match any character in the given set (whitespace)
  size_t end = str.find_last_not_of(" \t\n\r\f\v");

  /// Return the substring without leading/trailing whitespace
  return str.substr(start, end - start + 1);
}

} // namespace view
