#pragma once

#include "arena.h"
#include "list.h"
#include "node.h"
#include "position.h"
#include "result.h"
#include <stdint.h>

typedef struct value_t value_t;
typedef List(value_t) value_list_t;
typedef Result(value_t *) result_value_ref_t;
typedef result_void_t (*builtin_t)(value_t *result, value_list_t *nodes);

typedef enum {
  VALUE_TYPE_BOOLEAN,
  VALUE_TYPE_INTEGER,
  VALUE_TYPE_BUILTIN,
  VALUE_TYPE_CLOSURE,
  VALUE_TYPE_NIL,
  VALUE_TYPE_LIST,
} value_type_t;

typedef struct {
  node_t form;
  node_list_t arguments;
} closure_t;

typedef struct value_t {
  value_type_t type;
  position_t position;
  union {
    bool boolean;
    int32_t integer;
    closure_t closure;
    builtin_t builtin;
    nullptr_t nil;
    value_list_t list;
  } value;
} value_t;

constexpr size_t VALUE_LIST_INITIAL_SIZE = 8;

static inline result_ref_t valueCreate(arena_t *arena, value_type_t type) {
  value_t *value = nullptr;
  tryAssign(result_ref_t, arenaAllocate(arena, sizeof(value_t)), value);
  value->type = type;

  if (value->type == VALUE_TYPE_LIST) {
    value_list_t *list = nullptr;
    tryAssign(result_ref_t, listCreate(value_t, arena, VALUE_LIST_INITIAL_SIZE),
              list);
    bytewiseCopy(&value->value.list, list, sizeof(value_list_t));
  }

  if (value->type == VALUE_TYPE_CLOSURE) {
    value_list_t *list = nullptr;
    tryAssign(result_ref_t, listCreate(value_t, arena, VALUE_LIST_INITIAL_SIZE),
              list);
    bytewiseCopy(&value->value.closure.arguments, list, sizeof(value_list_t));

    // TODO: this is pessimistic, forms can be also non-lists and therefore
    // require less memory. Can we clean this up?
    node_t *form = nullptr;
    tryAssign(result_ref_t, nodeCreate(arena, NODE_TYPE_LIST), form);
    value->value.closure.form = *form;
  }

  return ok(result_ref_t, value);
};

static inline result_value_ref_t valueClone(arena_t *arena,
                                            const value_t *source) {
  value_t *destination = nullptr;
  tryAssign(result_value_ref_t, valueCreate(arena, source->type), destination);
  destination->position.line = source->position.line;
  destination->position.column = source->position.column;

  switch (source->type) {
  case VALUE_TYPE_BOOLEAN:
    destination->value.boolean = source->value.boolean;
    break;
  case VALUE_TYPE_INTEGER:
    destination->value.integer = source->value.integer;
    break;
  case VALUE_TYPE_BUILTIN:
    destination->value.builtin = source->value.builtin;
    break;
  case VALUE_TYPE_NIL:
    destination->value.nil = source->value.nil;
    break;
  case VALUE_TYPE_CLOSURE:
    nodeCopy(&source->value.closure.form, &destination->value.closure.form);

    try(result_value_ref_t, listCopy(value_t, &source->value.closure.arguments,
                                     &destination->value.closure.arguments));
    break;
  case VALUE_TYPE_LIST:
    try(result_value_ref_t,
        listCopy(value_t, &source->value.list, &destination->value.list));
    break;
  default:
    unreachable();
  }

  return ok(result_value_ref_t, destination);
}
