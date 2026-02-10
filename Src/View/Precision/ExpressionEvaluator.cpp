#include "ExpressionEvaluator.hpp"
#include <cmath>
#include <sstream>
#include <tinyexpr.h>

namespace view {

void ExpressionEvaluator::ExpressionDeleter::operator()(te_expr *expr) const {
  if (expr) {
    te_free(expr);
  }
}

std::optional<double>
ExpressionEvaluator::evaluate(const std::string &expression) {
  if (expression.empty()) {
    lastError_ = "Empty expression";
    errorPosition_ = 0;
    return std::nullopt;
  }

  int errorPos = 0;
  te_expr *compiled = te_compile(expression.c_str(), nullptr, 0, &errorPos);

  if (!compiled || errorPos != 0) {
    std::ostringstream oss;
    if (errorPos > 0 && errorPos <= static_cast<int>(expression.length())) {
      oss << "Syntax error at position " << errorPos;
      if (errorPos > 1) {
        oss << ": \"" << expression.substr(errorPos - 1, 10) << "\"";
      }
    } else {
      oss << "Syntax error in expression";
    }
    lastError_ = oss.str();
    errorPosition_ = errorPos;
    if (compiled) {
      te_free(compiled);
    }
    return std::nullopt;
  }

  double result = te_eval(compiled);
  te_free(compiled);

  // Check for NaN or Inf
  if (std::isnan(result)) {
    lastError_ = "Result is NaN (Not a Number)";
    errorPosition_ = 0;
    return std::nullopt;
  }

  if (std::isinf(result)) {
    lastError_ = "Result is infinite";
    errorPosition_ = 0;
    return std::nullopt;
  }

  lastError_.clear();
  errorPosition_ = 0;
  return result;
}

bool ExpressionEvaluator::compile(const std::string &expression) {
  if (expression.empty()) {
    lastError_ = "Empty expression";
    errorPosition_ = 0;
    return false;
  }

  int errorPos = 0;
  te_expr *compiled = te_compile(expression.c_str(), nullptr, 0, &errorPos);

  if (!compiled || errorPos != 0) {
    std::ostringstream oss;
    if (errorPos > 0 && errorPos <= static_cast<int>(expression.length())) {
      oss << "Syntax error at position " << errorPos;
      if (errorPos > 1) {
        oss << ": \"" << expression.substr(errorPos - 1, 10) << "\"";
      }
    } else {
      oss << "Syntax error in expression";
    }
    lastError_ = oss.str();
    errorPosition_ = errorPos;
    if (compiled) {
      te_free(compiled);
    }
    compiledExpression_.reset();
    return false;
  }

  compiledExpression_.reset(compiled);
  lastError_.clear();
  errorPosition_ = 0;
  return true;
}

std::optional<double> ExpressionEvaluator::evaluateCompiled() const {
  if (!compiledExpression_) {
    return std::nullopt;
  }

  double result = te_eval(compiledExpression_.get());

  // Check for NaN or Inf
  if (std::isnan(result)) {
    return std::nullopt;
  }

  if (std::isinf(result)) {
    return std::nullopt;
  }

  return result;
}

const std::string &ExpressionEvaluator::getLastError() const {
  return lastError_;
}

bool ExpressionEvaluator::hasCompiledExpression() const {
  return compiledExpression_ != nullptr;
}

void ExpressionEvaluator::clearCompiledExpression() {
  compiledExpression_.reset();
}

} // namespace view
