#pragma once

#include "alloc.h"
#include <stddef.h>
#include <string.h>

#define List(Type)                                                             \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    Type *data;                                                                \
  }

typedef List(void) generic_flat_list_t;

constexpr size_t LIST_STRIDE = 16;

// TODO: find a meaningful name
result_alloc_t _listAlloc(size_t capacity, size_t list_size, size_t item_size);
void _listDealloc(generic_flat_list_t *self);
result_alloc_t _listPush(generic_flat_list_t *self, size_t item_size,
                         const void *item);

#define listAlloc(ItemType, Capacity)                                          \
  _listAlloc((Capacity), sizeof(List(ItemType)), sizeof(ItemType))

#define listDealloc(List) _listDealloc((generic_flat_list_t *)(List))

#define listPush(List, Item)                                                   \
  _listPush((generic_flat_list_t *)(List), sizeof(*(Item)), Item)