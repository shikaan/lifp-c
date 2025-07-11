#pragma once

#include "parser.h"

typedef Result(node_t *) result_reduce_t;

result_reduce_t reduce(node_t *syntax_tree) {
  return ok(result_reduce_t, syntax_tree);
}