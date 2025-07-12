#pragma once

#include "alloc.h"
#include "arena.h"
#include <stddef.h>
#include <string.h>

#define List(Type)                                                             \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    Type *data;                                                                \
    arena_t *arena;                                                            \
  }

typedef List(void) generic_list_t;

constexpr size_t LIST_STRIDE = 16;

result_alloc_t genericListAlloc(arena_t *arena, size_t capacity,
                                size_t list_size, size_t item_size);
result_alloc_t genericListAppend(generic_list_t *self, const void *item);

#define listAlloc(Type, Arena, Capacity)                                       \
  genericListAlloc(Arena, (Capacity), sizeof(List(Type)), sizeof(Type))
#define listAppend(List, Item) genericListAppend((generic_list_t *)(List), Item)
