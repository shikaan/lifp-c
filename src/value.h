#pragma once

#include "list.h"
#include "alloc.h"
#include "arena.h"
#include "position.h"

typedef struct value_t value_t;
typedef List(value_t) value_list_t;
typedef ResultVoid() result_lambda_t;
typedef result_lambda_t (*lambda_t)(value_t *result, value_list_t *nodes);

typedef enum {
  VALUE_TYPE_BOOLEAN,
  VALUE_TYPE_INTEGER,
  VALUE_TYPE_FUNCTION,
  VALUE_TYPE_NIL,
  VALUE_TYPE_LIST,
} value_type_t;

typedef struct value_t {
  value_type_t type;
  union {
    bool boolean;
    int32_t integer;
    lambda_t function;
    nullptr_t nil;
    value_list_t list;
  } value;
  position_t position;
} value_t;

static inline result_alloc_t valueCreate(arena_t *arena) {
  return arenaAllocate(arena, sizeof(value_t));
};
