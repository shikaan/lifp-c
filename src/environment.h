#pragma once

#include "./arena.h"
#include "./map.h"
#include "./value.h"

typedef struct environment_t {
  arena_t *arena;
  Map(value_t) * values;
  // Map(special_form_t) * special_forms;
  struct environment_t *parent;
} environment_t;

result_builtin_t sum(value_t *result, value_list_t *values);
result_alloc_t environmentCreate(arena_t *arena, environment_t *parent);

value_t *environmentResolveSymbol(environment_t *self, const char *symbol);
