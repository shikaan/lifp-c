#include "node.h"
#include "arena.h"
#include "list.h"
#include "result.h"

static constexpr size_t INITIAL_SIZE = 8;

result_alloc_t nodeCreate(arena_t *arena, node_type_t type) {
  node_t *node = nullptr;
  try(result_alloc_t, arenaAllocate(arena, sizeof(node_t)), node);

  if (type == NODE_TYPE_LIST) {
    node_list_t *list = nullptr;
    try(result_alloc_t, listCreate(node_t, arena, INITIAL_SIZE), list);
    bytewiseCopy(&node->value.list, list, sizeof(node_list_t));
  }

  node->type = type;
  return ok(result_alloc_t, node);
}

result_copy_t nodeCopy(const node_t *source, node_t *destination) {
  destination->type = source->type;
  destination->position.column = source->position.column;
  destination->position.line = source->position.line;

  if (source->type == NODE_TYPE_LIST) {
    tryVoid(result_copy_t,
            listCopy(node_t, &source->value.list, &destination->value.list));
  } else {
    destination->value = source->value;
  }

  return (result_copy_t){.ok = true};
}

result_alloc_t nodeClone(arena_t *arena, const node_t *source) {
  node_t *destination = nullptr;
  try(result_alloc_t, nodeCreate(arena, source->type), destination);
  tryVoid(result_alloc_t, nodeCopy(source, destination));
  return ok(result_alloc_t, destination);
}
