#pragma once

#include "./arena.h"
#include "./environment.h"
#include "./node.h"
#include "./value.h"

typedef Result(value_t *) result_reduce_t;
result_reduce_t reduce(arena_t *arena, node_t *syntax_tree,
                       environment_t *environment);
