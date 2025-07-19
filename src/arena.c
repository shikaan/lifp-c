#include "arena.h"
#include "alloc.h"
#include "result.h"

result_alloc_t arenaCreate(size_t size) {
  result_alloc_t allocation = allocSafe(sizeof(arena_t) + size);
  if (!allocation.ok) {
    return allocation;
  }
  arena_t *arena = allocation.value;
  arena->size = size;
  arena->offset = 0;
  return ok(result_alloc_t, arena);
}

result_alloc_t arenaAllocate(arena_t *self, size_t size) {
  size_t aligned_offset = (self->offset + 7U) & ~7U;
  size_t aligned_size = (size + 7U) & ~7U;

  if (aligned_offset + aligned_size > self->size) {
    error_t exception = {.kind = ERROR_KIND_ALLOCATION};
    return error(result_alloc_t, exception);
  }

  byte_t *pointer = &self->memory[aligned_offset];
  self->offset = aligned_offset + aligned_size;
  return ok(result_alloc_t, pointer);
}

void arenaDestroy(arena_t *self) { deallocSafe(&self); }

void arenaReset(arena_t *self) { self->offset = 0; }
