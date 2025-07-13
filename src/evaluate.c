#include "./evaluate.h"
#include "arena.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include <stddef.h>

void sum(node_t *result, node_list_t *nodes) {
  result->type = NODE_TYPE_INTEGER;
  result->value.integer = 0;

  for (size_t i = 0; i < nodes->count; i++) {
    node_t current = listGet(node_t, nodes, i);
    // TODO: this should raise an error instead of silently skipping non-numbers
    if (current.type == NODE_TYPE_INTEGER) {
      result->value.integer += current.value.integer;
    }
  }
}

result_reduce_t reduce(arena_t *arena, node_t *syntax_tree) {
  if (syntax_tree->type == NODE_TYPE_LIST &&
      syntax_tree->value.list.count != 0) {
    const auto list = syntax_tree->value.list;
    node_list_t *reduced_list = nullptr;
    node_t *reduced_node = nullptr;

    result_alloc_t allocation = listAlloc(node_t, arena, list.capacity);
    if (!allocation.ok) {
      return error(result_reduce_t, allocation.error);
    }
    reduced_list = allocation.value;

    for (size_t i = 0; i < list.count; i++) {
      node_t node = listGet(node_t, &list, i);
      result_reduce_t reduction = reduce(arena, &node);
      if (!reduction.ok) {
        return error(result_reduce_t, reduction.error);
      }

      allocation = listAppend(node_t, reduced_list, reduction.value);
      if (!allocation.ok) {
        return error(result_reduce_t, allocation.error);
      }
    }

    allocation = nodeAlloc(arena, NODE_TYPE_LIST);
    if (!allocation.ok) {
      return error(result_reduce_t, allocation.error);
    }
    reduced_node = allocation.value;

    auto first_node = listGet(node_t, &list, 0);
    if (first_node.type == NODE_TYPE_SYMBOL) {
      sum(reduced_node, reduced_list);
      return ok(result_reduce_t, reduced_node);
    }

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
