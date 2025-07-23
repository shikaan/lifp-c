#include "node.h"
#include "arena.h"
#include "list.h"

result_alloc_t nodeAlloc(arena_t *arena, node_type_t type) {
  if (type == NODE_TYPE_LIST) {
    result_alloc_t node_allocation = arenaAllocate(arena, sizeof(node_t));
    if (!node_allocation.ok) {
      return node_allocation;
    }
    node_t *node = node_allocation.value;

    result_alloc_t list_allocation = listCreate(node_t, arena, 8);
    if (!list_allocation.ok) {
      return list_allocation;
    }

    node_list_t *list = list_allocation.value;
    node->value.list.capacity = list->capacity;
    node->value.list.count = list->count;
    node->value.list.item_size = list->item_size;
    node->value.list.data = list->data;

    return node_allocation;
  }

  return arenaAllocate(arena, sizeof(node_t));
}
