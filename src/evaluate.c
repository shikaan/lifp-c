#include "./evaluate.h"
#include "arena.h"
#include "environment.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include "value.h"

#include "specials.c"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Make clang see unreachable
// TODO: under which conditions can this be removed?
#define __need_unreachable 1

static bool isSpecialFormNode(const node_t FIRST_NODE) {
  return (FIRST_NODE.type == NODE_TYPE_SYMBOL &&
          strncmp(FIRST_NODE.value.symbol, "def!", 4) == 0) != 0;
}

static inline void valueListInit(value_t *value, arena_t *arena,
                                 size_t capacity, size_t count,
                                 size_t item_size, void *data) {
  value->type = VALUE_TYPE_LIST;
  value->value.list.arena = arena;
  value->value.list.capacity = capacity;
  value->value.list.count = count;
  value->value.list.item_size = item_size;
  value->value.list.data = data;
}

result_reduce_t reduceList(arena_t *arena, node_t *syntax_tree,
                           environment_t *environment) {
  const auto list = syntax_tree->value.list;

  value_t *result_value = nullptr;
  try(result_reduce_t, valueCreate(arena), result_value);
  result_value->position = syntax_tree->position;

  if (list.count == 0) {
    try(result_reduce_t, valueCreate(arena), result_value);
    valueListInit(result_value, arena, 0, 0, 0, nullptr);
    return ok(result_reduce_t, result_value);
  }

  auto first_node = listGet(node_t, &list, 0);

  if (isSpecialFormNode(first_node)) {
    tryVoid(result_reduce_t, define(environment, &list));

    try(result_reduce_t, valueCreate(arena), result_value);
    result_value->type = VALUE_TYPE_NIL;
    return ok(result_reduce_t, result_value);
  }

  value_list_t *reduced_values = nullptr;
  try(result_reduce_t, listCreate(value_t, arena, list.capacity),
      reduced_values);

  for (size_t i = 0; i < list.count; i++) {
    auto node = listGet(node_t, &list, i);
    value_t *reduced = nullptr;
    try(result_reduce_t, reduce(arena, &node, environment), reduced);
    tryVoid(result_reduce_t, listAppend(value_t, reduced_values, reduced));
  }

  value_t first_value = listGet(value_t, reduced_values, 0);

  if (first_value.type == VALUE_TYPE_FUNCTION) {
    lambda_t lambda = first_value.value.function;
    tryVoid(result_reduce_t, lambda(result_value, reduced_values));
    return ok(result_reduce_t, result_value);
  }

  valueListInit(result_value, arena, reduced_values->capacity,
                reduced_values->count, reduced_values->item_size,
                reduced_values->data);
  return ok(result_reduce_t, result_value);
}

result_reduce_t reduce(arena_t *arena, node_t *syntax_tree,
                       environment_t *environment) {
  value_t *value = nullptr;
  try(result_reduce_t, valueCreate(arena), value);

  switch (syntax_tree->type) {
  case NODE_TYPE_BOOLEAN: {
    value->type = VALUE_TYPE_BOOLEAN;
    value->value.boolean = syntax_tree->value.boolean;
    break;
  }
  case NODE_TYPE_NIL: {
    value->type = VALUE_TYPE_NIL;
    value->value.nil = nullptr;
    break;
  }
  case NODE_TYPE_INTEGER: {
    value->type = VALUE_TYPE_INTEGER;
    value->value.integer = syntax_tree->value.integer;
    break;
  }
  case NODE_TYPE_SYMBOL: {
    value_t *resolved_value =
        environmentResolveSymbol(environment, syntax_tree->value.symbol);

    if (!resolved_value) {
      const error_t error = {
          .kind = ERROR_KIND_SYMBOL_NOT_FOUND,
          .payload.symbol_not_found.symbol = syntax_tree->value.symbol,
          .payload.symbol_not_found.position = syntax_tree->position,
      };
      return error(result_reduce_t, error);
    }

    value->type = resolved_value->type;
    value->value = resolved_value->value;
    break;
  }
  case NODE_TYPE_LIST:
    return reduceList(arena, syntax_tree, environment);
  default:
    unreachable();
  }
  return ok(result_reduce_t, value);
}
