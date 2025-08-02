#pragma once

#include "../lib/arena.h"
#include "environment.h"
#include "node.h"
#include "value.h"

result_value_ref_t evaluate(arena_t *arena, node_t *syntax_tree,
                            environment_t *environment);
