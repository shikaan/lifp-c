#pragma once

#include "./alloc.h"
#include "./arena.h"
#include "./map.h"
#include "./result.h"
#include "./value.h"

typedef ResultVoid() result_builtin_t;

typedef result_builtin_t (*builtin_t)(value_t *result, value_list_t *nodes);

typedef struct environment_t {
  Map(builtin_t) * builtins;
  struct environment_t *parent;
} environment_t;

result_builtin_t sum(value_t *result, value_list_t *values);
result_alloc_t environmentCreate(arena_t *arena, environment_t *parent);
