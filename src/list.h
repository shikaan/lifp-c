#pragma once

#include "alloc.h"
#include <stddef.h>
#include <string.h>

#define List(Type)                                                             \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    Type *data;                                                                \
  }

typedef List(void) generic_list_t;

constexpr size_t LIST_STRIDE = 16;

// TODO: find a meaningful name
result_alloc_t _listAlloc(size_t capacity, size_t list_size, size_t item_size);
void _listDealloc(generic_list_t **self);
result_alloc_t _listAppend(generic_list_t *self, const void *item);
void _listShift(generic_list_t *self);

#define listAlloc(Type, Capacity)                                              \
  _listAlloc((Capacity), sizeof(List(Type)), sizeof(Type))

#define listDealloc(List) _listDealloc((generic_list_t **)(List))
#define listAppend(List, Item) _listAppend((generic_list_t *)(List), Item)
#define listShift(List) _listShift((generic_list_t *)(List))