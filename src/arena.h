#pragma once

#include "alloc.h"
#include "result.h"
#include <stddef.h>

typedef unsigned char byte_t;

typedef struct {
  size_t size;
  size_t offset;
  byte_t memory[];
} arena_t;

static inline result_alloc_t arenaCreate(size_t size) {
  result_alloc_t allocation = allocSafe(sizeof(arena_t) + size);
  if (!allocation.ok) {
    return allocation;
  }
  arena_t *arena = allocation.value;
  arena->size = size;
  arena->offset = 0;
  return ok(result_alloc_t, arena);
}

static inline result_alloc_t arenaAllocate(arena_t *self, size_t size) {
  if (self->offset + size >= self->size) {
    exception_t exception = {.kind = EXCEPTION_KIND_ALLOCATION};
    return error(result_alloc_t, exception);
  }

  byte_t *pointer = &self->memory[self->offset];
  self->offset += size;
  return ok(result_alloc_t, pointer);
}

static inline void arenaDestroy(arena_t *self) { deallocSafe(&self); }

static inline void arenaReset(arena_t *self) { self->offset = 0; }