#pragma once

#include "alloc.h"
#include "arena.h"
#include "result.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

constexpr size_t MAX_KEY_LENGTH = 32;

#define Map(Type)                                                              \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    bool *used;                                                                \
    char (*keys)[MAX_KEY_LENGTH];                                              \
    Type *values;                                                              \
    arena_t *arena;                                                            \
  }

typedef Map(void) generic_map_t;

static inline uint64_t hash(size_t len, const char key[static len]) {
  uint64_t hash = 14695981039346656037U;
  const uint64_t prime = 1099511628211U;

  for (size_t i = 0; i < len; i++) {
    hash ^= (uint64_t)(unsigned char)key[i];
    hash *= prime;
  }

  return hash;
}

static inline size_t makeKey(generic_map_t *self, const char *key) {
  uint64_t hashed_key = hash(strlen(key), key);
  return hashed_key % self->capacity;
}

#define mapCreate(ItemType, Arena, Capacity)                                   \
  genericMapCreate(Arena, Capacity, sizeof(ItemType))

#define mapSet(Map, Key, Value)                                                \
  genericMapSet((generic_map_t *)(Map), Key, Value)

#define mapGet(ItemType, Map, Key)                                             \
  (ItemType *)genericMapGet((generic_map_t *)(Map), Key)

static inline result_alloc_t genericMapCreate(arena_t *arena, size_t capacity,
                                              size_t item_size) {
  assert(arena != nullptr);
  assert(capacity > 0);
  assert(item_size > 0);

  result_alloc_t allocation = arenaAllocate(arena, sizeof(generic_map_t));
  if (!allocation.ok) {
    return allocation;
  }

  generic_map_t *map = allocation.value;
  map->count = 0;
  map->capacity = capacity;
  map->item_size = item_size;
  map->arena = arena;

  allocation = arenaAllocate(arena, sizeof(bool) * capacity);
  if (!allocation.ok)
    return allocation;
  map->used = allocation.value;
  memset(map->used, 0, sizeof(bool) * capacity);

  allocation = arenaAllocate(arena, sizeof(char) * MAX_KEY_LENGTH * capacity);
  if (!allocation.ok)
    return allocation;
  map->keys = allocation.value;
  memset(map->keys, 0, MAX_KEY_LENGTH * capacity);

  allocation = arenaAllocate(arena, item_size * capacity);
  if (!allocation.ok)
    return allocation;
  map->values = allocation.value;
  memset(map->values, 0, item_size * capacity);

  return ok(result_alloc_t, map);
}

static inline void rawValueSet(generic_map_t *self, size_t index,
                               const void *value) {
  byte_t *destination = (byte_t *)self->values + (index * self->item_size);
  assert(destination != nullptr); // memcpy on nullptr causes undefined behaviour
  memcpy(destination, value, self->item_size);
}

static inline result_alloc_t genericMapSet(generic_map_t *self, const char *key,
                                           void *value) {
  size_t key_length = strlen(key);
  if (key_length >= MAX_KEY_LENGTH) {
    exception_t exception = {.kind = EXCEPTION_KIND_KEY_TOO_LONG, .payload.key_too_long = key_length };
    return error(result_alloc_t, exception);
  }

  size_t index = makeKey(self, key);
  size_t count = 0;

  while (self->used[index]) {
    const bool is_same_key =
        strncmp(self->keys[index], key, MAX_KEY_LENGTH) == 0;

    if (is_same_key) {
      rawValueSet(self, index, value);
      return (result_alloc_t){.ok = true};
    }

    index = (index + 1) % self->capacity;
    count++;

    if (count == self->capacity) {
      // TODO: extend and rehash
      exception_t exception = {.kind = EXCEPTION_KIND_ALLOCATION};
      return error(result_alloc_t, exception);
    }
  }

  self->used[index] = true;
  strncpy(self->keys[index], key, MAX_KEY_LENGTH - 1);
  self->keys[index][MAX_KEY_LENGTH - 1] = 0;
  rawValueSet(self, index, value);
  self->count++;

  return (result_alloc_t){.ok = true};
}

static inline void *genericMapGet(generic_map_t *self, const char *key) {
  if (self->count == 0)
    return nullptr;

  size_t index = makeKey(self, key);
  size_t count = 0;

  while ((int)self->used[index] && count < self->capacity) {
    if (strncmp(self->keys[index], key, MAX_KEY_LENGTH) == 0) {
      return (byte_t *)self->values + (index * self->item_size);
    }

    index = (index + 1) % self->capacity;
    count++;
  }

  return nullptr;
}
