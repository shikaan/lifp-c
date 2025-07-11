#include "./stack.h"
#include <string.h>

result_alloc_t _stackAlloc(size_t capacity, size_t stack_size,
                           size_t item_size) {
  result_alloc_t stack_result = allocSafe(stack_size);
  if (!stack_result.ok) {
    return stack_result;
  }

  // The following dance is required to keep the item_size a const
  const generic_stack_t local_stack = (generic_stack_t){
      .capacity = capacity,
      .count = 0,
      .item_size = item_size,
  };
  memcpy(stack_result.value, &local_stack, stack_size);

  generic_stack_t *stack = stack_result.value;
  result_alloc_t data_result = allocSafe(stack->item_size * stack->capacity);
  if (!data_result.ok) {
    deallocSafe(stack);
    return data_result;
  }
  stack->data = data_result.value;
  return ok(result_alloc_t, stack);
}

void _stackDealloc(generic_stack_t *self) {
  deallocSafe(self->data);
  deallocSafe(self);
}

result_alloc_t _stackPush(generic_stack_t *self, const void *item) {
  if (self->count >= self->capacity) {
    size_t new_capacity = self->capacity + STACK_STRIDE;

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
  void *destination = (char *)self->data +
                      (self->item_size * (self->capacity - self->count - 1));
  memcpy(destination, item, self->item_size);
  self->count++;

  return (result_alloc_t){.ok = true};
}
