#include "./evaluate.h"
#include "arena.h"
#include "environment.h"
#include "list.h"
#include "map.h"
#include "node.h"
#include "parser.h"
#include "result.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

result_reduce_t reduce(arena_t *arena, node_t *syntax_tree,
                       environment_t *environment) {
  if (syntax_tree->type == NODE_TYPE_LIST &&
      syntax_tree->value.list.count != 0) {
    const auto list = syntax_tree->value.list;
    node_list_t *reduced_list = nullptr;
    node_t *reduced_node = nullptr;

    result_alloc_t allocation = listCreate(node_t, arena, list.capacity);
    if (!allocation.ok) {
      return error(result_reduce_t, allocation.error);
    }
    reduced_list = allocation.value;

    for (size_t i = 0; i < list.count; i++) {
      node_t node = listGet(node_t, &list, i);
      result_reduce_t reduction = reduce(arena, &node, environment);
      if (!reduction.ok) {
        return error(result_reduce_t, reduction.error);
      }

      allocation = listAppend(node_t, reduced_list, reduction.value);
      if (!allocation.ok) {
        return error(result_reduce_t, allocation.error);
      }
    }

    auto first_node = listGet(node_t, &list, 0);
    if (first_node.type == NODE_TYPE_SYMBOL) {
      const char *symbol = first_node.value.symbol;

      // Allocate a result node for the function to populate
      allocation = nodeAlloc(arena, NODE_TYPE_INTEGER);
      if (!allocation.ok) {
        return error(result_reduce_t, allocation.error);
      }
      node_t *result_node = allocation.value;

      builtin_t *builtin_ptr = mapGet(builtin_t, environment->builtins, symbol);
      if (!builtin_ptr) {
        // FIXME: wrong error type
        return error(result_reduce_t,
                     (exception_t){.kind = EXCEPTION_KIND_ALLOCATION});
      }

      result_builtin_t invocation = (*builtin_ptr)(result_node, reduced_list);
      if (!invocation.ok) {
        return error(result_reduce_t, invocation.error);
      }

      return ok(result_reduce_t, result_node);
    }

    allocation = nodeAlloc(arena, NODE_TYPE_LIST);
    if (!allocation.ok) {
      return error(result_reduce_t, allocation.error);
    }
    reduced_node = allocation.value;
    reduced_node->type = NODE_TYPE_LIST;
    reduced_node->value.list.capacity = reduced_list->capacity;
    reduced_node->value.list.count = reduced_list->count;
    reduced_node->value.list.data = reduced_list->data;
    reduced_node->position = syntax_tree->position;
    return ok(result_reduce_t, reduced_node);
  }

  result_alloc_t duplication = nodeClone(arena, *syntax_tree);
  if (!duplication.ok) {
    return error(result_reduce_t, duplication.error);
  }
  node_t *duplicate = duplication.value;
  return ok(result_reduce_t, duplicate);
}
