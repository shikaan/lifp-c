#pragma once

#include "./list.h"
#include "alloc.h"
#include "arena.h"
#include "position.h"

typedef struct value_t value_t;
typedef List(struct value_t) value_list_t;

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
    void *function; // TODO: how to type a generic function?
    nullptr_t nil;
    value_list_t list;
  } value;
  position_t position;
} value_t;

static inline result_alloc_t valueAlloc(arena_t *arena) {
  return arenaAllocate(arena, sizeof(value_t));
};
