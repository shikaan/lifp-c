#pragma once

#include "./alloc.h"
#include "./arena.h"
#include "./map.h"
#include "./node.h"
#include "./result.h"

typedef ResultVoid() result_builtin_t;

typedef result_builtin_t (*builtin_t)(node_t *result, node_list_t *nodes);

typedef struct environment_t {
  Map(builtin_t) * builtins;
  struct environment_t *parent;
} environment_t;

result_builtin_t sum(node_t* result, node_list_t* nodes);
result_alloc_t environmentCreate(arena_t *arena, environment_t *parent);
