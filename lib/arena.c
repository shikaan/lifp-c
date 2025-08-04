#include "arena.h"
#include "result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    throw(result_ref_t, ARENA_ERROR_MALLOC_ERROR, nullptr,
          "Unable to allocate size: %lu", size);
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
  arena_t *arena = nullptr;
  try(result_ref_t, allocSafe(sizeof(arena_t) + size), arena);
  arena->size = size;
  arena->offset = 0;
  return ok(result_ref_t, arena);
}

result_ref_t arenaAllocate(arena_t *self, size_t size) {
  size_t aligned_offset = (self->offset + 7U) & ~7U;
  size_t aligned_size = (size + 7U) & ~7U;

  if (aligned_offset + aligned_size > self->size) {
    throw(result_ref_t, ARENA_ERROR_OUT_OF_SPACE, nullptr,
          "Arena out of memory. Available %lu, requested %lu",
          self->size - aligned_offset, aligned_size);
  }

  byte_t *pointer = &self->memory[aligned_offset];
  memset(pointer, 0, aligned_size);
  self->offset = aligned_offset + aligned_size;
  return ok(result_ref_t, pointer);
}

void arenaDestroy(arena_t *self) { deallocSafe(&self); }

void arenaReset(arena_t *self) { self->offset = 0; }
