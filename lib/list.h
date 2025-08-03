#pragma once

#include "arena.h"
#include "result.h"

typedef enum {
  LIST_ERROR_ALLOCATION = ARENA_ERROR_OUT_OF_SPACE,
  LIST_ERROR_DESTINATION_TOO_SMALL,
} list_error_t;

// NOLINTBEGIN - Type cannot be put in parentheses
#define List(Type)                                                             \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    arena_t *arena;                                                            \
    Type *data;                                                                \
  }
// NOLINTEND

typedef List(void) generic_list_t;

[[nodiscard]] result_ref_t genericListCreate(arena_t *arena, size_t capacity,
                                             size_t list_size,
                                             size_t item_size);
[[nodiscard]] result_void_t genericListAppend(generic_list_t *self,
                                              const void *item);

[[nodiscard]] void *genericListGet(const generic_list_t *self, size_t index);

[[nodiscard]] result_ref_t genericListCopy(const generic_list_t *source,
                                           generic_list_t *destination);

#define listCreate(ItemType, Arena, Capacity)                                  \
  genericListCreate(Arena, (Capacity), sizeof(List(ItemType)), sizeof(ItemType))

// TODO: can this be made typesafe with static asserts?
#define listAppend(ItemType, List, Item)                                       \
  genericListAppend((generic_list_t *)(List), Item)

// TODO: provide bound checks
#define listGet(ItemType, List, Index) (ItemType)(List)->data[Index]

// TODO: can this be made typesafe with static asserts?
#define listCopy(ItemType, Source, Destination)                                \
  genericListCopy((const generic_list_t *)(Source),                            \
                  (generic_list_t *)(Destination))