#include "./list.h"
#include "alloc.h"
#include "arena.h"
#include <stddef.h>

result_alloc_t genericListAlloc(arena_t *arena, size_t capacity,
                                size_t list_size, size_t item_size) {
  result_alloc_t allocation = arenaAllocate(arena, list_size);
  if (!allocation.ok) {
    return allocation;
  }
  generic_list_t *list = allocation.value;
  list->count = 0;
  list->capacity = capacity;
  list->item_size = item_size;
  list->arena = arena;

  allocation = arenaAllocate(arena, item_size * list->capacity);
  if (!allocation.ok)
    return allocation;

  list->offset = allocation.value;
  return ok(result_alloc_t, list);
}

result_alloc_t genericListAppend(generic_list_t *self, const void *item) {
  if (self->count >= self->capacity) {
    size_t new_capacity = self->capacity + LIST_STRIDE;

    result_alloc_t realloc_result =
        arenaAllocate(self->arena, self->item_size * new_capacity);
    if (!realloc_result.ok) {
      return realloc_result;
    }

    void *new_data = realloc_result.value;
    bytewiseCopy(new_data, self->offset, self->item_size * self->count);

    self->offset = new_data;
    self->capacity = new_capacity;
  }

  void *destination = (byte_t *)self->offset + (self->item_size * self->count);
  memcpy(destination, item, self->item_size);
  self->count++;

  return (result_alloc_t){.ok = true};
}