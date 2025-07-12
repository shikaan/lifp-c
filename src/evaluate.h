#pragma once

#include "arena.h"
#include "parser.h"

typedef Result(node_t *) result_reduce_t;
result_reduce_t reduce(arena_t *arena, node_t *syntax_tree);