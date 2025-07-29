#pragma once

#include "position.h"
#include <stddef.h>

typedef enum {
  ERROR_KIND_ALLOCATION,

  // Syntax Error
  ERROR_KIND_UNEXPECTED_TOKEN,
  ERROR_KIND_INVALID_EXPRESSION,
  ERROR_KIND_UNBALANCED_PARENTHESES,

  // Constraint Errors
  ERROR_KIND_KEY_TOO_LONG,
  ERROR_KIND_INVALID_TOKEN_SIZE,

  // Reference Errors
  ERROR_KIND_SYMBOL_NOT_FOUND,

  // Type Error
  ERROR_KIND_UNEXPECTED_TYPE,
  ERROR_KIND_UNEXPECTED_VALUE,
  ERROR_KIND_UNEXPECTED_ARITY,
} error_kind_t;

typedef union {
  nullptr_t allocation;
  char unexpected_token;
  nullptr_t invalid_expression;
  nullptr_t unbalanced_parentheses;
  size_t key_too_long;
  size_t invalid_token_size;
  char *symbol_not_found;

  struct {
    // NOTE: these need casting to value_type_t
    int expected;
    int actual;
  } unexpected_type;

  struct {
    void *expected;
    void *actual;
  } unexpected_value;

  struct {
    size_t expected;
    size_t actual;
  } unexpected_arity;
} error_payload_t;

typedef struct {
  error_kind_t kind;
  error_payload_t payload;
  position_t position;
  const char *example;
} error_t;

typedef struct {
  bool ok;
  error_t error;
} result_void_t;

#define Result(ValueType)                                                      \
  struct {                                                                     \
    bool ok;                                                                   \
    union {                                                                    \
      ValueType value;                                                         \
      error_t error;                                                           \
    };                                                                         \
  }

#define _concat_detail(x, y) x##y
#define _concat(x, y) _concat_detail(x, y)
#define _result_name(ResultType) _concat(ResultType, __LINE__)

#define tryAssign(ResultType, Action, Destination)                             \
  {                                                                            \
    auto _result_name(ResultType) = Action;                                    \
    if (!_result_name(ResultType).ok) {                                        \
      return error(ResultType, _result_name(ResultType).error);                \
    }                                                                          \
    (Destination) = (_result_name(ResultType).value);                          \
  }

#define try(ResultType, Action)                                                \
  {                                                                            \
    auto _result_name(ResultType) = Action;                                    \
    if (!_result_name(ResultType).ok) {                                        \
      return error(ResultType, _result_name(ResultType).error);                \
    }                                                                          \
  }

// Helper functions for creating results
#define ok(T, Value)                                                           \
  (T) { .ok = true, .value = (Value) }
#define error(T, Error)                                                        \
  (T) { .ok = false, .error = (Error) }
