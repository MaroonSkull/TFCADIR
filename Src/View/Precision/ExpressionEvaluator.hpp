#pragma once

#include <memory>
#include <optional>
#include <string>

// Forward declare TinyExpr types
extern "C" {
typedef struct te_expr te_expr;
typedef struct te_variable te_variable;
te_expr *te_compile(const char *expression, const te_variable *variables,
                    int var_count, int *error);
double te_eval(const te_expr *expr);
void te_free(te_expr *expr);
double te_interp(const char *expression, int *error);
}

namespace view {

/**
 * @brief RAII wrapper for TinyExpr compiled expressions
 *
 * ExpressionEvaluator provides a safe C++ wrapper around TinyExpr's
 * C API, ensuring proper resource management through RAII principles.
 *
 * DESIGN RATIONALE:
 *
 * 1. RAII Resource Management:
 *    - Unique pointer with custom deleter ensures te_free() is called
 *    - No manual memory management required by users
 *    - Exception-safe and thread-safe evaluation
 *
 * 2. Stateless Coordinator Pattern:
 *    - ExpressionEvaluator maintains NO visibility state
 *    - All settings queried from UIFSMAdapter
 *    - Follows same pattern as GridManager, SnapManager, MeasurementManager
 *
 * 3. Error Handling:
 *    - Uses std::optional for optional return values
 *    - Provides error messages for invalid expressions
 *    - Graceful handling of compilation errors
 *
 * 4. Thread Safety:
 *    - Each ExpressionEvaluator instance is independent
 *    - Compiled expressions are immutable after creation
 *    - Safe for concurrent use from multiple threads
 *
 * USAGE:
 *
 * @code
 * ExpressionEvaluator evaluator;
 *
 * // Compile and evaluate expression
 * auto result = evaluator.evaluate("50+25.5");
 * if (result.has_value()) {
 *     float value = result.value();
 * }
 *
 * // Check for errors
 * if (!result.has_value()) {
 *     std::string error = evaluator.getLastError();
 * }
 * @endcode
 */
class ExpressionEvaluator {
public:
  /**
   * @brief Custom deleter for TinyExpr expressions
   */
  struct ExpressionDeleter {
    void operator()(te_expr *expr) const;
  };

  /**
   * @brief Construct an ExpressionEvaluator
   */
  ExpressionEvaluator() = default;

  /**
   * @brief Destructor
   */
  ~ExpressionEvaluator() = default;

  // Disable copy (compiled expressions are not copyable)
  ExpressionEvaluator(const ExpressionEvaluator &) = delete;
  ExpressionEvaluator &operator=(const ExpressionEvaluator &) = delete;

  // Enable move
  ExpressionEvaluator(ExpressionEvaluator &&) noexcept = default;
  ExpressionEvaluator &operator=(ExpressionEvaluator &&) noexcept = default;

  /**
   * @brief Compile and evaluate an expression string
   * @param expression The expression string to evaluate
   * @return std::optional<double> Result if successful, std::nullopt on error
   *
   * Evaluates a mathematical expression. Supports:
   * - Operators: +, -, *, /, ^ (power)
   * - Parentheses: (100 + 25) * 2
   * - Functions: sin(x), cos(x), tan(x), sqrt(x), abs(x), ln(x), log(x), exp(x)
   * - Constants: pi, e
   *
   * All trigonometric functions use degrees.
   */
  [[nodiscard]] std::optional<double> evaluate(const std::string &expression);

  /**
   * @brief Compile an expression for repeated evaluation
   * @param expression The expression string to compile
   * @return true if compilation successful, false otherwise
   *
   * Compiles an expression for faster repeated evaluation.
   * Use getCompiledResult() to evaluate the compiled expression.
   */
  [[nodiscard]] bool compile(const std::string &expression);

  /**
   * @brief Evaluate the previously compiled expression
   * @return std::optional<double> Result if successful, std::nullopt on error
   *
   * Evaluates the expression compiled with compile().
   * Returns std::nullopt if no expression is compiled.
   */
  [[nodiscard]] std::optional<double> evaluateCompiled() const;

  /**
   * @brief Get the last error message
   * @return Error message from the last failed evaluation
   */
  [[nodiscard]] const std::string &getLastError() const;

  /**
   * @brief Check if an expression is currently compiled
   * @return true if a compiled expression exists
   */
  [[nodiscard]] bool hasCompiledExpression() const;

  /**
   * @brief Clear the compiled expression
   */
  void clearCompiledExpression();

private:
  /// Compiled expression (RAII-managed)
  std::unique_ptr<te_expr, ExpressionDeleter> compiledExpression_;

  /// Last error message
  std::string lastError_;

  /// Error position in expression (0 if no error)
  int errorPosition_{0};
};

} // namespace view
