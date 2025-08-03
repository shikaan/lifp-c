#pragma once

#include "../lib/arena.h"
#include "../lib/map.h"
#include "value.h"

typedef struct environment_t {
  arena_t *arena;
  Map(value_t) * values;
  struct environment_t *parent;
} environment_t;

result_void_position_t sum(value_t *result, value_list_t *values);

result_ref_t environmentCreate(environment_t *parent);
void environmentDestroy(environment_t **self_ref);

value_t *environmentResolveSymbol(environment_t *self, const char *symbol);
