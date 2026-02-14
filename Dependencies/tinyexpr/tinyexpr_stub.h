/*
 * TINYEXPR - Tiny recursive descent expression parser and evaluator
 *
 * Copyright (c) 2015-2020 Blake Morrison
 * Copyright (c) 2021-2023 TinyExpr contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TINYEXPR_H
#define TINYEXPR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct te_expr {
  char *expression;
  double value;
  int error;
} te_expr;

/**
 * @brief Parses and compiles an expression string
 * @param expression The expression string to compile
 * @param error Pointer to store error position (0 if no error)
 * @return Compiled expression or NULL on error
 *
 * Compiles an expression for fast evaluation. The returned pointer must be
 * freed with te_free() when no longer needed.
 */
te_expr *te_compile(const char *expression, int *error);

/**
 * @brief Evaluates a compiled expression
 * @param expr The compiled expression from te_compile()
 * @return The result of the evaluation
 */
double te_eval(const te_expr *expr);

/**
 * @brief Frees a compiled expression
 * @param expr The expression to free
 */
void te_free(te_expr *expr);

/**
 * @brief Evaluates an expression string directly
 * @param expression The expression string to evaluate
 * @return The result of the evaluation
 *
 * This is a convenience function that compiles and evaluates an expression
 * in one step. For repeated evaluations, use te_compile() and te_eval().
 */
double te_interp(const char *expression, int *error);

/**
 * @brief Sets a variable in the expression context
 * @param name The variable name
 * @param value The variable value
 *
 * Note: This function is a placeholder for variable binding.
 * The current implementation uses a simple lookup mechanism.
 */
void te_var(const char *name, double value);

#ifdef __cplusplus
}
#endif

#endif /* TINYEXPR_H */
