#pragma once

#include "./position.h"
#include <wchar.h>

typedef enum {
  ERROR_KIND_ALLOCATION,

  // Syntax Error
  ERROR_KIND_UNEXPECTED_TOKEN,
  ERROR_KIND_INVALID_EXPRESSION,
  ERROR_KIND_UNBALANCED_PARENTHESES,

  // Constraint Errors
  ERROR_KIND_KEY_TOO_LONG,

  // Reference Errors
  ERROR_KIND_SYMBOL_NOT_FOUND,
} error_kind_t;

typedef union {
  struct {
    position_t position;
    char token;
  } unexpected_token;
  struct {
    position_t position;
    char *symbol;
  } symbol_not_found;
  nullptr_t allocation;
  nullptr_t invalid_expression;
  nullptr_t unbalanced_parentheses;
  size_t key_too_long;
} error_payload_t;

typedef struct {
  error_kind_t kind;
  error_payload_t payload;
} error_t;

#define Result(ValueType)                                                      \
  struct {                                                                     \
    bool ok;                                                                   \
    union {                                                                    \
      ValueType value;                                                         \
      error_t error;                                                           \
    };                                                                         \
  }

#define ResultVoid()                                                           \
  struct {                                                                     \
    bool ok;                                                                   \
    error_t error;                                                             \
  }

// Helper functions for creating results
#define ok(T, Value)                                                           \
  (T) { .ok = true, .value = (Value) }
#define error(T, Error)                                                        \
  (T) { .ok = false, .error = (Error) }
