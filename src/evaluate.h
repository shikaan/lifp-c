#pragma once

#include "./arena.h"
#include "./environment.h"
#include "./node.h"
#include "./value.h"

typedef Result(value_t *) result_valuep_t;
result_valuep_t evaluate(arena_t *arena, node_t *syntax_tree,
                         environment_t *environment);
