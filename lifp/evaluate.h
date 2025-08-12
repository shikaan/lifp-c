#pragma once

#include "../lib/arena.h"
#include "node.h"
#include "value.h"
#include "virtual_machine.h"

result_value_ref_t evaluate(arena_t *arena, node_t *syntax_tree,
                            environment_t *environment);
