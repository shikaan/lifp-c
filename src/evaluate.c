#include "./evaluate.h"
#include "alloc.h"
#include "list.h"
#include "parser.h"
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

const char FUNCTION_SYMS[][1] = {"+"};
const function_t FUNCTIONS[1] = {sum};

result_reduce_t reduce(node_t *syntax_tree) {
  if (syntax_tree->type == NODE_TYPE_LIST) {
    const auto list = syntax_tree->value.list;
    exception_t exception;
    node_list_t *reduced = nullptr;

    result_alloc_t allocation = listAlloc(node_t, list.capacity);
    if (!allocation.ok) {
      exception = allocation.error;
      goto list_error;
    }

    reduced = allocation.value;

    // TODO: using a stack can make this more cache friendly
    for (size_t i = list.count - 1; i > 0; i--) {
      result_reduce_t reduction = reduce(&list.data[i]);
      if (!reduction.ok) {
        exception = reduction.error;
        goto list_error;
      }

      listPush(reduced, reduction.value);
    }

    allocation = allocSafe(sizeof(node_t));
    if (!allocation.ok) {
      exception = allocation.error;
      goto list_error;
    }
    node_t *reduced_node = allocation.value;

    auto last_node = list.data[0];
    if (last_node.type == NODE_TYPE_SYMBOL) {
      sum(reduced_node, reduced);
      return ok(result_reduce_t, reduced_node);
    }

    result_reduce_t reduction = reduce(&last_node);
    if (!reduction.ok) {
      exception = reduction.error;
      goto list_error;
    }
    listPush(reduced, reduction.value);

    reduced_node->type = NODE_TYPE_LIST;
    reduced_node->value.list.capacity = reduced->capacity;
    reduced_node->value.list.count = reduced->count;
    reduced_node->value.list.data = reduced->data;
    reduced_node->position = syntax_tree->position;
    return ok(result_reduce_t, reduced_node);

  list_error:
    listDealloc(reduced);
    return error(result_reduce_t, exception.kind, exception.payload);
  }

  return ok(result_reduce_t, syntax_tree);
}
