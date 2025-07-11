#include "./list.h"
#include <string.h>

result_alloc_t _listAlloc(size_t capacity, size_t list_size, size_t item_size) {
  result_alloc_t list_result = allocSafe(list_size);
  if (!list_result.ok) {
    return list_result;
  }
  // The following dance is required to keep the item_size a const
  const generic_flat_list_t local_list = (const generic_flat_list_t){
      .capacity = capacity,
      .count = 0,
      .item_size = item_size,
  };
  memcpy(list_result.value, &local_list, list_size);

  generic_flat_list_t *list = list_result.value;
  result_alloc_t data_result = allocSafe(item_size * list->capacity);
  if (!data_result.ok) {
    deallocSafe(list);
    return data_result;
  }
  list->data = data_result.value;
  return ok(result_alloc_t, list);
}

void _listDealloc(generic_flat_list_t *self) {
  deallocSafe(self->data);
  deallocSafe(self);
}

result_alloc_t _listPush(generic_flat_list_t *self, const void *item) {
  if (self->count >= self->capacity) {
    size_t new_capacity = self->capacity + LIST_STRIDE;

    result_alloc_t realloc_result = allocSafe(self->item_size * new_capacity);
    if (!realloc_result.ok) {
      return realloc_result;
    }

    void *new_data = realloc_result.value;
    memcpy(new_data, self->data, self->item_size * self->count);

    deallocSafe(self->data);
    self->data = new_data;
    self->capacity = new_capacity;
  }

  // Using char* for byte-wise addressing
  void *destination = (char *)self->data + (self->item_size * self->count);
  memcpy(destination, item, self->item_size);
  self->count++;

  return (result_alloc_t){.ok = true};
}