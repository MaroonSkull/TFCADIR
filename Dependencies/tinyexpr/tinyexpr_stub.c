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

#include "tinyexpr.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

/* Parser state */
typedef struct {
  const char *expression;
  int index;
  int error;
} parser_state;

/* Forward declarations */
static double parse_expression(parser_state *s);
static double parse_term(parser_state *s);
static double parse_factor(parser_state *s);
static double parse_power(parser_state *s);
static double parse_unary(parser_state *s);

/* Skip whitespace */
static void skip_whitespace(parser_state *s) {
  while (isspace((unsigned char)s->expression[s->index])) {
    s->index++;
  }
}

/* Get current character */
static char current_char(parser_state *s) { return s->expression[s->index]; }

/* Consume current character */
static void consume_char(parser_state *s) { s->index++; }

/* Check if current character matches expected */
static int match_char(parser_state *s, char c) {
  if (current_char(s) == c) {
    consume_char(s);
    return 1;
  }
  return 0;
}

/* Parse number */
static double parse_number(parser_state *s) {
  char buffer[256];
  int i = 0;
  int has_decimal = 0;

  while (isdigit((unsigned char)current_char(s)) || current_char(s) == '.') {
    if (current_char(s) == '.') {
      if (has_decimal)
        break;
      has_decimal = 1;
    }
    if (i < 255) {
      buffer[i++] = current_char(s);
    }
    consume_char(s);
  }
  buffer[i] = '\0';
  return atof(buffer);
}

/* Parse identifier */
static const char *parse_identifier(parser_state *s, char *buffer, int size) {
  int i = 0;
  while (isalpha((unsigned char)current_char(s)) ||
         isdigit((unsigned char)current_char(s)) || current_char(s) == '_') {
    if (i < size - 1) {
      buffer[i++] = current_char(s);
    }
    consume_char(s);
  }
  buffer[i] = '\0';
  return buffer;
}

/* Check if identifier is a constant */
static int is_constant(const char *name, double *value) {
  if (strcmp(name, "pi") == 0 || strcmp(name, "PI") == 0) {
    *value = M_PI;
    return 1;
  }
  if (strcmp(name, "e") == 0 || strcmp(name, "E") == 0) {
    *value = M_E;
    return 1;
  }
  return 0;
}

/* Evaluate single-argument function */
static double eval_function1(const char *name, double arg) {
  /* Convert to radians for trig functions (input is degrees) */
  double rad = arg * M_PI / 180.0;

  if (strcmp(name, "sin") == 0)
    return sin(rad);
  if (strcmp(name, "cos") == 0)
    return cos(rad);
  if (strcmp(name, "tan") == 0)
    return tan(rad);
  if (strcmp(name, "asin") == 0)
    return asin(arg) * 180.0 / M_PI;
  if (strcmp(name, "acos") == 0)
    return acos(arg) * 180.0 / M_PI;
  if (strcmp(name, "atan") == 0)
    return atan(arg) * 180.0 / M_PI;
  if (strcmp(name, "sinh") == 0)
    return sinh(arg);
  if (strcmp(name, "cosh") == 0)
    return cosh(arg);
  if (strcmp(name, "tanh") == 0)
    return tanh(arg);
  if (strcmp(name, "sqrt") == 0)
    return sqrt(arg);
  if (strcmp(name, "abs") == 0)
    return fabs(arg);
  if (strcmp(name, "ln") == 0)
    return log(arg);
  if (strcmp(name, "log") == 0)
    return log10(arg);
  if (strcmp(name, "exp") == 0)
    return exp(arg);
  if (strcmp(name, "ceil") == 0)
    return ceil(arg);
  if (strcmp(name, "floor") == 0)
    return floor(arg);
  return NAN;
}

/* Evaluate two-argument function */
static double eval_function2(const char *name, double arg1, double arg2) {
  if (strcmp(name, "atan2") == 0)
    return atan2(arg1, arg2) * 180.0 / M_PI;
  if (strcmp(name, "pow") == 0)
    return pow(arg1, arg2);
  return NAN;
}

/* Parse function call */
static double parse_call(parser_state *s, const char *name) {
  /* Try single-argument function first */
  double arg1 = parse_expression(s);
  skip_whitespace(s);

  if (match_char(s, ',')) {
    /* Two-argument function */
    double arg2 = parse_expression(s);
    skip_whitespace(s);
    if (!match_char(s, ')')) {
      s->error = s->index;
      return NAN;
    }
    return eval_function2(name, arg1, arg2);
  } else {
    /* Single-argument function */
    if (!match_char(s, ')')) {
      s->error = s->index;
      return NAN;
    }
    return eval_function1(name, arg1);
  }
}

/* Parse primary expression */
static double parse_primary(parser_state *s) {
  skip_whitespace(s);

  /* Number */
  if (isdigit((unsigned char)current_char(s)) || current_char(s) == '.') {
    return parse_number(s);
  }

  /* Variable or function or constant */
  if (isalpha((unsigned char)current_char(s))) {
    char buffer[256];
    parse_identifier(s, buffer, sizeof(buffer));

    /* Check for constant */
    double value;
    if (is_constant(buffer, &value)) {
      return value;
    }

    /* Check for function call */
    skip_whitespace(s);
    if (match_char(s, '(')) {
      return parse_call(s, buffer);
    }

    /* Unknown identifier */
    s->error = s->index;
    return NAN;
  }

  /* Parenthesized expression */
  if (match_char(s, '(')) {
    double result = parse_expression(s);
    skip_whitespace(s);
    if (!match_char(s, ')')) {
      s->error = s->index;
      return NAN;
    }
    return result;
  }

  s->error = s->index;
  return NAN;
}

/* Parse power (right associative) */
static double parse_power(parser_state *s) {
  double result = parse_unary(s);

  skip_whitespace(s);
  if (match_char(s, '^')) {
    double exponent = parse_power(s); /* Right associative */
    result = pow(result, exponent);
  }

  return result;
}

/* Parse unary operators */
static double parse_unary(parser_state *s) {
  skip_whitespace(s);

  if (match_char(s, '+')) {
    return parse_unary(s);
  }
  if (match_char(s, '-')) {
    return -parse_unary(s);
  }

  return parse_primary(s);
}

/* Parse factor (multiplication, division) */
static double parse_factor(parser_state *s) {
  double result = parse_power(s);

  while (1) {
    skip_whitespace(s);
    if (match_char(s, '*')) {
      result *= parse_power(s);
    } else if (match_char(s, '/')) {
      double divisor = parse_power(s);
      if (divisor == 0.0) {
        s->error = s->index;
        return NAN;
      }
      result /= divisor;
    } else {
      break;
    }
  }

  return result;
}

/* Parse term (addition, subtraction) */
static double parse_term(parser_state *s) {
  double result = parse_factor(s);

  while (1) {
    skip_whitespace(s);
    if (match_char(s, '+')) {
      result += parse_factor(s);
    } else if (match_char(s, '-')) {
      result -= parse_factor(s);
    } else {
      break;
    }
  }

  return result;
}

/* Parse expression */
static double parse_expression(parser_state *s) { return parse_term(s); }

/* Public API implementation */

te_expr *te_compile(const char *expression, int *error) {
  if (!expression) {
    if (error)
      *error = -1;
    return NULL;
  }

  te_expr *expr = (te_expr *)malloc(sizeof(te_expr));
  if (!expr) {
    if (error)
      *error = -1;
    return NULL;
  }

  expr->expression = strdup(expression);
  if (!expr->expression) {
    free(expr);
    if (error)
      *error = -1;
    return NULL;
  }

  parser_state s = {expression, 0, 0};
  expr->value = parse_expression(&s);
  expr->error = s.error;

  if (error) {
    *error = s.error;
  }

  return expr;
}

double te_eval(const te_expr *expr) {
  if (!expr)
    return NAN;
  if (expr->error)
    return NAN;
  return expr->value;
}

void te_free(te_expr *expr) {
  if (expr) {
    free(expr->expression);
    free(expr);
  }
}

double te_interp(const char *expression, int *error) {
  if (!expression) {
    if (error)
      *error = -1;
    return NAN;
  }

  parser_state s = {expression, 0, 0};
  double result = parse_expression(&s);

  if (error) {
    *error = s.error;
  }

  return result;
}

void te_var(const char *name, double value) {
  /* Placeholder for variable binding */
  (void)name;
  (void)value;
}
