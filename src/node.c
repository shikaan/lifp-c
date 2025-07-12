#include "node.h"
#include "alloc.h"
#include "list.h"

result_alloc_t nodeAlloc(node_type_t type) {
  if (type == NODE_TYPE_LIST) {
    result_alloc_t node_allocation = allocSafe(sizeof(node_t));
    if (!node_allocation.ok) {
      return node_allocation;
    }
    node_t *node = node_allocation.value;

    result_alloc_t list_allocation = listAlloc(node_t, LIST_STRIDE);
    if (!list_allocation.ok) {
      deallocSafe(&node);
      return list_allocation;
    }

    node_list_t *list = list_allocation.value;
    node->value.list.capacity = list->capacity;
    node->value.list.count = list->count;
    node->value.list.item_size = list->item_size;
    node->value.list.data = list->data;
    deallocSafe(&list);

    return node_allocation;
  }

  return allocSafe(sizeof(node_t));
}

result_alloc_t nodeDuplicate(node_t node) {
  result_alloc_t allocation = nodeAlloc(node.type);
  if (!allocation.ok) {
    return allocation;
  }
  node_t *duplicate = allocation.value;
  duplicate->type = node.type;
  duplicate->position = node.position;
  bytewiseCopy(&duplicate->value, &node.value, sizeof(duplicate->value));
  return allocation;
}

void nodeDealloc(node_t **self) {
  node_t *ptr = (*self);
  if (ptr->type == NODE_TYPE_LIST) {
    deallocSafe(&ptr->value.list.data);
  }
  deallocSafe(self);
}
