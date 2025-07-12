#include "./evaluate.h"
#include "alloc.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include <stddef.h>

typedef void (*function_t)(node_t *, node_list_t *);

void sum(node_t *result, node_list_t *nodes) {
  result->type = NODE_TYPE_INTEGER;
  result->value.integer = 0;

  for (size_t i = 0; i < nodes->count; i++) {
    // TODO: this should raise an error
    if (nodes->data[i].type == NODE_TYPE_INTEGER) {
      result->value.integer += nodes->data[i].value.integer;
    }
  }
}

result_reduce_t reduce(node_t *syntax_tree) {
  if (syntax_tree->type == NODE_TYPE_LIST) {
    const auto list = syntax_tree->value.list;
    exception_t exception;
    node_list_t *reduced_list = nullptr;
    node_t *reduced_node = nullptr;

    result_alloc_t allocation = listAlloc(node_t, list.capacity);
    if (!allocation.ok) {
      exception = allocation.error;
      goto list_error;
    }
    reduced_list = allocation.value;

    for (size_t i = 0; i < list.count; i++) {
      result_reduce_t reduction = reduce(&list.data[i]);
      if (!reduction.ok) {
        exception = reduction.error;
        goto list_error;
      }

      listAppend(reduced_list, reduction.value);
      nodeDealloc(&reduction.value);
    }

    allocation = allocSafe(sizeof(node_t));
    if (!allocation.ok) {
      exception = allocation.error;
      goto list_error;
    }
    reduced_node = allocation.value;

    auto last_node = list.data[0];
    if (last_node.type == NODE_TYPE_SYMBOL) {
      sum(reduced_node, reduced_list);
      listDealloc(&reduced_list);
      return ok(result_reduce_t, reduced_node);
    }

    reduced_node->type = NODE_TYPE_LIST;
    reduced_node->value.list.capacity = reduced_list->capacity;
    reduced_node->value.list.count = reduced_list->count;
    reduced_node->value.list.data = reduced_list->data;
    reduced_node->position = syntax_tree->position;
    deallocSafe(&reduced_list);
    return ok(result_reduce_t, reduced_node);

  list_error:
    listDealloc(&reduced_list);
    nodeDealloc(&reduced_node);
    return error(result_reduce_t, exception.kind, exception.payload);
  }

  result_alloc_t allocation = allocSafe(sizeof(node_t));
  if (!allocation.ok) {
    return error(result_reduce_t, allocation.error.kind,
                 allocation.error.payload);
  }
  node_t *duplicate = allocation.value;
  *duplicate = *syntax_tree;
  return ok(result_reduce_t, duplicate);
}
