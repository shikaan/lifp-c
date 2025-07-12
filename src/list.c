#include "./list.h"
#include "alloc.h"
#include "debug.h"

result_alloc_t _listAlloc(size_t capacity, size_t list_size, size_t item_size) {
  result_alloc_t allocation = allocSafe(list_size);
  if (!allocation.ok) {
    return allocation;
  }
  generic_list_t *list = allocation.value;
  list->capacity = capacity;
  list->count = 0;
  list->item_size = item_size;

  allocation = allocSafe(item_size * list->capacity);
  if (!allocation.ok) {
    deallocSafe(&list);
    return allocation;
  }
  list->data = allocation.value;
  return ok(result_alloc_t, list);
}

void _listDealloc(generic_list_t **self) {
  if (self && *self) {
    deallocSafe(&(*self)->data);
    deallocSafe(self);
  }
}

result_alloc_t _listAppend(generic_list_t *self, const void *item) {
  if (self->count >= self->capacity) {
    size_t new_capacity = self->capacity + LIST_STRIDE;

    result_alloc_t realloc_result = allocSafe(self->item_size * new_capacity);
    if (!realloc_result.ok) {
      return realloc_result;
    }

    void *new_data = realloc_result.value;
    bytewiseCopy(new_data, self->data, self->item_size * self->count);

    deallocSafe(&self->data);
    self->data = new_data;
    self->capacity = new_capacity;
  }

  // Using char* for byte-wise addressing
  void *destination =
      (unsigned char *)self->data + (self->item_size * self->count);
  bytewiseCopy(destination, item, self->item_size);
  self->count++;

  return (result_alloc_t){.ok = true};
}

void _listShift(generic_list_t *self) {
  self->count--;
  self->data = (char *)self->data + self->item_size;
}