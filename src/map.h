#pragma once

#include "alloc.h"
#include "arena.h"
#include <stddef.h>
#include <stdint.h>

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

#define mapCreate(ItemType, Arena, Capacity)                                   \
  genericMapCreate(Arena, Capacity, sizeof(ItemType))

#define mapSet(Map, Key, Value)                                                \
  genericMapSet((generic_map_t *)(Map), Key, Value)

#define mapGet(ItemType, Map, Key)                                             \
  (ItemType *)genericMapGet((generic_map_t *)(Map), Key)

result_alloc_t genericMapCreate(arena_t *arena, size_t capacity,
                                size_t item_size);
result_alloc_t genericMapSet(generic_map_t *self, const char *key, void *value);
void *genericMapGet(generic_map_t *self, const char *key);
