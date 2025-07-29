#include "list.h"
#include "arena.h"
#include <assert.h>

result_ref_t genericListAlloc(arena_t *arena, size_t capacity, size_t list_size,
                              size_t item_size) {
  assert(arena);
  generic_list_t *list = nullptr;
  tryAssign(result_ref_t, arenaAllocate(arena, list_size), list);

  list->count = 0;
  list->capacity = capacity;
  list->item_size = item_size;
  list->arena = arena;
  tryAssign(result_ref_t, arenaAllocate(arena, item_size * list->capacity),
            list->data);

  return ok(result_ref_t, list);
}

result_ref_t genericListAppend(generic_list_t *self, const void *item) {
  assert(self);
  if (!item)
    return (result_ref_t){.ok = true};

  if (self->count >= self->capacity) {
    size_t new_capacity = self->capacity * 2;

    void *new_data = nullptr;
    tryAssign(result_ref_t,
              arenaAllocate(self->arena, self->item_size * new_capacity),
              new_data);

    bytewiseCopy(new_data, self->data, self->item_size * self->count);

    self->data = new_data;
    self->capacity = new_capacity;
  }

  void *destination = (byte_t *)self->data + (self->item_size * self->count);
  bytewiseCopy(destination, item, self->item_size);
  self->count++;

  return (result_ref_t){.ok = true};
}

void *genericListGet(const generic_list_t *self, size_t index) {
  assert(self);
  if (index >= self->count)
    return nullptr;

  return (byte_t *)self->data + (self->item_size * index);
}

result_ref_t genericListCopy(const generic_list_t *source,
                             generic_list_t *destination) {
  assert(source);
  assert(destination);

  if (destination->capacity < source->count) {
    const error_t error = {.kind = ERROR_KIND_ALLOCATION};
    return error(result_ref_t, error);
  }

  destination->item_size = source->item_size;

  for (size_t i = 0; i < source->count; i++) {
    void *source_node = genericListGet(source, i);
    tryVoid(result_ref_t, genericListAppend(destination, source_node));
  }

  destination->count = source->count;
  return ok(result_ref_t, destination);
}
