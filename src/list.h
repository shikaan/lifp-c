#pragma once

#include "arena.h"

#define List(Type)                                                             \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    Type *data;                                                                \
    arena_t *arena;                                                            \
  }

typedef List(void) generic_list_t;

[[nodiscard]] result_alloc_t genericListAlloc(arena_t *arena, size_t capacity,
                                              size_t list_size,
                                              size_t item_size);
[[nodiscard]] result_alloc_t genericListAppend(generic_list_t *self,
                                               const void *item);

void *genericListGet(const generic_list_t *self, size_t index);

#define listCreate(ItemType, Arena, Capacity)                                  \
  genericListAlloc(Arena, (Capacity), sizeof(List(ItemType)), sizeof(ItemType))

// TODO: can this be made typesafe with static asserts?
#define listAppend(ItemType, List, Item)                                       \
  genericListAppend((generic_list_t *)(List), Item)

// TODO: provide bound checks
#define listGet(ItemType, List, Index) (ItemType)(List)->data[Index]
