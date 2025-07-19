#include "./evaluate.h"
#include "alloc.h"
#include "arena.h"
#include "environment.h"
#include "list.h"
#include "map.h"
#include "node.h"
#include "result.h"
#include "value.h"
#include <__stddef_unreachable.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

result_reduce_t reduceList(arena_t *arena, node_t *syntax_tree,
                           environment_t *environment) {
  const auto list = syntax_tree->value.list;

  result_alloc_t allocation = listCreate(value_t, arena, list.capacity);
  if (!allocation.ok) {
    return error(result_reduce_t, allocation.error);
  }
  value_list_t *primitives = allocation.value;

  for (size_t i = 0; i < list.count; i++) {
    node_t node = listGet(node_t, &list, i);
    result_reduce_t reduction = reduce(arena, &node, environment);
    if (!reduction.ok) {
      return error(result_reduce_t, reduction.error);
    }

    allocation = listAppend(value_t, primitives, reduction.value);
    if (!allocation.ok) {
      return error(result_reduce_t, allocation.error);
    }
  }

  allocation = valueAlloc(arena);
  if (!allocation.ok) {
    return error(result_reduce_t, allocation.error);
  }
  value_t *result_value = allocation.value;
  result_value->position = syntax_tree->position;

  auto first_node = listGet(node_t, &list, 0);
  if (first_node.type == NODE_TYPE_SYMBOL) {
    const char *symbol = first_node.value.symbol;

    builtin_t *builtin = mapGet(builtin_t, environment->builtins, symbol);
    if (!builtin) {
      // FIXME: wrong error type
      return error(result_reduce_t,
                   (exception_t){.kind = EXCEPTION_KIND_ALLOCATION});
    }

    result_builtin_t invocation = (*builtin)(result_value, primitives);
    if (!invocation.ok) {
      return error(result_reduce_t, invocation.error);
    }

    return ok(result_reduce_t, result_value);
  }

  result_value->type = VALUE_TYPE_LIST;
  result_value->value.list.arena = arena;
  result_value->value.list.capacity = primitives->capacity;
  result_value->value.list.count = primitives->count;
  result_value->value.list.item_size = primitives->item_size;
  result_value->value.list.data = primitives->data;

  return ok(result_reduce_t, result_value);
}

result_reduce_t reduce(arena_t *arena, node_t *syntax_tree,
                       environment_t *environment) {
  if (syntax_tree->type == NODE_TYPE_LIST &&
      syntax_tree->value.list.count != 0) {
    return reduceList(arena, syntax_tree, environment);
  }

  result_alloc_t duplication = arenaAllocate(arena, sizeof(value_t));
  if (!duplication.ok) {
    return error(result_reduce_t, duplication.error);
  }
  value_t *value = duplication.value;

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
    // TODO: rest of enviroment resolution
    value->type = VALUE_TYPE_FUNCTION;
    value->value.function = (void *)mapGet(builtin_t, environment->builtins,
                                           syntax_tree->value.symbol);
    break;
  }
  case NODE_TYPE_LIST: {
    // this case will only occur for an empty list
    result_alloc_t allocation = listCreate(byte_t, arena, 0);
    if (!allocation.ok) {
      return error(result_reduce_t, allocation.error);
    }
    value->type = VALUE_TYPE_LIST;
    value->value.list.capacity = 0;
    value->value.list.count = 0;
    break;
  }
  default:
    unreachable();
  }

  return ok(result_reduce_t, value);
}
