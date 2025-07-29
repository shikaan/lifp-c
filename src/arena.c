#include "arena.h"
#include <stdlib.h>

#define deallocSafe(DoublePointer)                                             \
  {                                                                            \
    if (*(DoublePointer) != nullptr) {                                         \
      free((void *)*(DoublePointer));                                          \
      *(DoublePointer) = nullptr;                                              \
    }                                                                          \
  }

static result_ref_t allocSafe(size_t size) {
  void *ptr = malloc(size);

  if (ptr == nullptr) {
    const error_t exception = {.kind = ERROR_KIND_ALLOCATION};
    return error(result_ref_t, exception);
  }

  return ok(result_ref_t, ptr);
}

void bytewiseCopy(void *dest, const void *src, size_t size) {
  const auto dest_bytes = (unsigned char *)dest;
  const auto src_bytes = (const unsigned char *)src;
  for (size_t i = 0; i < size; i++) {
    dest_bytes[i] = src_bytes[i];
  }
}

result_ref_t arenaCreate(size_t size) {
  result_ref_t allocation = allocSafe(sizeof(arena_t) + size);
  if (!allocation.ok) {
    return allocation;
  }
  arena_t *arena = allocation.value;
  arena->size = size;
  arena->offset = 0;
  return ok(result_ref_t, arena);
}

result_ref_t arenaAllocate(arena_t *self, size_t size) {
  size_t aligned_offset = (self->offset + 7U) & ~7U;
  size_t aligned_size = (size + 7U) & ~7U;

  if (aligned_offset + aligned_size > self->size) {
    error_t exception = {.kind = ERROR_KIND_ALLOCATION};
    return error(result_ref_t, exception);
  }

  byte_t *pointer = &self->memory[aligned_offset];
  self->offset = aligned_offset + aligned_size;
  return ok(result_ref_t, pointer);
}

void arenaDestroy(arena_t *self) { deallocSafe(&self); }

void arenaReset(arena_t *self) { self->offset = 0; }
