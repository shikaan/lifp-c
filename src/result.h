#pragma once

#include "./position.h"

typedef enum {
  EXCEPTION_KIND_UNEXPECTED_TOKEN,
  EXCEPTION_KIND_ALLOCATION,
  EXCEPTION_KIND_INVALID_EXPRESSION,
  EXCEPTION_KIND_UNBALANCED_PARENTHESES,
  EXCEPTION_KIND_KEY_TOO_LONG,
} exception_kind_t;

typedef struct {
  char token;
  position_t position;
} unexpected_token_payload_t;

typedef union {
  unexpected_token_payload_t unexpected_token;
  nullptr_t allocation;
  nullptr_t invalid_expression;
  nullptr_t unbalanced_parentheses;
  size_t key_too_long;
} exception_payload_t;

typedef struct {
  exception_kind_t kind;
  exception_payload_t payload;
} exception_t;

#define Result(ValueType)                                                      \
  struct {                                                                     \
    bool ok;                                                                   \
    union {                                                                    \
      ValueType value;                                                         \
      exception_t error;                                                       \
    };                                                                         \
  }

#define ResultVoid()                                                           \
  struct {                                                                     \
    bool ok;                                                                   \
    exception_t error;                                                         \
  }

// Helper functions for creating results
#define ok(T, Value)                                                           \
  (T) { .ok = true, .value = (Value) }
#define error(T, Error)                                                        \
  (T) { .ok = false, .error = (Error) }
