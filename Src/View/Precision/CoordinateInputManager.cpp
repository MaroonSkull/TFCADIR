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

  try {
    // Tokenize the expression
    auto tokens = tokenize(trimmed);

    // Parse and evaluate the expression
    size_t pos = 0;
    double result = parseExpression(tokens, pos);

    // Check if we consumed all tokens
    if (pos < tokens.size() && tokens[pos].type != TokenType::End) {
      return CoordinateInputResult{
          std::nullopt, "Unexpected token at position " + std::to_string(pos)};
    }

    // Validate result range
    if (std::abs(result) > 1e6) {
      return CoordinateInputResult{std::nullopt, "Expression result too large"};
    }

    return CoordinateInputResult{static_cast<float>(result), ""};
  } catch (const std::exception &e) {
    return CoordinateInputResult{std::nullopt, "Expression parsing error: " +
                                                   std::string(e.what())};
  }
}

// ==========================================================================
// Expression Parser Implementation
// ==========================================================================

std::vector<CoordinateInputManager::Token>
CoordinateInputManager::tokenize(const std::string &input) const {
  std::vector<Token> tokens;
  size_t pos = 0;

  while (pos < input.length()) {
    // Skip whitespace
    if (std::isspace(static_cast<unsigned char>(input[pos]))) {
      ++pos;
      continue;
    }

    // Check for operators
    if (input[pos] == '+') {
      tokens.push_back({TokenType::Plus, "+", 0.0});
      ++pos;
    } else if (input[pos] == '-') {
      tokens.push_back({TokenType::Minus, "-", 0.0});
      ++pos;
    } else if (input[pos] == '*') {
      tokens.push_back({TokenType::Multiply, "*", 0.0});
      ++pos;
    } else if (input[pos] == '/') {
      tokens.push_back({TokenType::Divide, "/", 0.0});
      ++pos;
    } else if (input[pos] == '(') {
      tokens.push_back({TokenType::LParen, "(", 0.0});
      ++pos;
    } else if (input[pos] == ')') {
      tokens.push_back({TokenType::RParen, ")", 0.0});
      ++pos;
    } else if (std::isalpha(static_cast<unsigned char>(input[pos]))) {
      // Read identifier (function name or constant)
      std::string identifier;
      while (pos < input.length() &&
             std::isalpha(static_cast<unsigned char>(input[pos]))) {
        identifier += input[pos];
        ++pos;
      }

      // Convert to lowercase for case-insensitive comparison
      std::string lowerIdentifier = identifier;
      std::transform(lowerIdentifier.begin(), lowerIdentifier.end(),
                     lowerIdentifier.begin(),
                     [](unsigned char c) { return std::tolower(c); });

      // Check for functions
      if (lowerIdentifier == "sin" || lowerIdentifier == "cos" ||
          lowerIdentifier == "tan" || lowerIdentifier == "sqrt" ||
          lowerIdentifier == "abs") {
        tokens.push_back({TokenType::Function, lowerIdentifier, 0.0});
      }
      // Check for constants
      else if (lowerIdentifier == "pi") {
        tokens.push_back({TokenType::Constant, "pi", PI});
      } else if (lowerIdentifier == "e") {
        tokens.push_back({TokenType::Constant, "e", E});
      } else {
        throw std::runtime_error("Unknown identifier: " + identifier);
      }
    } else if (std::isdigit(static_cast<unsigned char>(input[pos])) ||
               input[pos] == '.') {
      // Read number
      std::string number;
      while (pos < input.length() &&
             (std::isdigit(static_cast<unsigned char>(input[pos])) ||
              input[pos] == '.')) {
        number += input[pos];
        ++pos;
      }

      try {
        double value = std::stod(number);
        tokens.push_back({TokenType::Number, number, value});
      } catch (const std::exception &) {
        throw std::runtime_error("Invalid number: " + number);
      }
    } else {
      throw std::runtime_error(std::string("Unexpected character: ") +
                               input[pos]);
    }
  }

  // Add end token
  tokens.push_back({TokenType::End, "", 0.0});

  return tokens;
}

double CoordinateInputManager::parseExpression(const std::vector<Token> &tokens,
                                               size_t &pos) const {
  // Parse first term
  double result = parseTerm(tokens, pos);

  // Parse additional terms with +/-
  while (pos < tokens.size() && (tokens[pos].type == TokenType::Plus ||
                                 tokens[pos].type == TokenType::Minus)) {
    TokenType op = tokens[pos].type;
    ++pos;

    double term = parseTerm(tokens, pos);

    if (op == TokenType::Plus) {
      result += term;
    } else {
      result -= term;
    }
  }

  return result;
}

double CoordinateInputManager::parseTerm(const std::vector<Token> &tokens,
                                         size_t &pos) const {
  // Parse first factor
  double result = parseFactor(tokens, pos);

  // Parse additional factors with */
  while (pos < tokens.size() && (tokens[pos].type == TokenType::Multiply ||
                                 tokens[pos].type == TokenType::Divide)) {
    TokenType op = tokens[pos].type;
    ++pos;

    double factor = parseFactor(tokens, pos);

    if (op == TokenType::Multiply) {
      result *= factor;
    } else {
      if (factor == 0.0) {
        throw std::runtime_error("Division by zero");
      }
      result /= factor;
    }
  }

  return result;
}

double CoordinateInputManager::parseFactor(const std::vector<Token> &tokens,
                                           size_t &pos) const {
  // Check for unary minus
  bool negative = false;
  if (pos < tokens.size() && tokens[pos].type == TokenType::Minus) {
    negative = true;
    ++pos;
  }

  if (pos >= tokens.size()) {
    throw std::runtime_error("Unexpected end of expression");
  }

  double result = 0.0;

  if (tokens[pos].type == TokenType::Number) {
    result = tokens[pos].value;
    ++pos;
  } else if (tokens[pos].type == TokenType::Constant) {
    result = tokens[pos].value;
    ++pos;
  } else if (tokens[pos].type == TokenType::LParen) {
    ++pos; // Skip '('
    result = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::RParen) {
      throw std::runtime_error("Missing closing parenthesis");
    }
    ++pos; // Skip ')'
  } else if (tokens[pos].type == TokenType::Function) {
    std::string funcName = tokens[pos].text;
    ++pos; // Skip function name

    if (pos >= tokens.size() || tokens[pos].type != TokenType::LParen) {
      throw std::runtime_error("Expected '(' after function " + funcName);
    }
    ++pos; // Skip '('

    /// Validate that function has an argument (not empty call like sin())
    if (pos < tokens.size() && tokens[pos].type == TokenType::RParen) {
      throw std::runtime_error("Function " + funcName +
                               " requires an argument");
    }

    double argument = parseExpression(tokens, pos);

    if (pos >= tokens.size() || tokens[pos].type != TokenType::RParen) {
      throw std::runtime_error("Missing closing parenthesis after function " +
                               funcName);
    }
    ++pos; // Skip ')'

    result = evaluateFunction(funcName, argument);
  } else {
    throw std::runtime_error("Unexpected token in expression");
  }

  return negative ? -result : result;
}

double CoordinateInputManager::evaluateFunction(const std::string &name,
                                                double argument) const {
  if (name == "sin") {
    return std::sin(degreesToRadians(argument));
  } else if (name == "cos") {
    return std::cos(degreesToRadians(argument));
  } else if (name == "tan") {
    return std::tan(degreesToRadians(argument));
  } else if (name == "sqrt") {
    if (argument < 0.0) {
      throw std::runtime_error("Square root of negative number");
    }
    return std::sqrt(argument);
  } else if (name == "abs") {
    return std::abs(argument);
  } else {
    throw std::runtime_error("Unknown function: " + name);
  }
}

double CoordinateInputManager::degreesToRadians(double degrees) {
  return degrees * PI / 180.0;
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
