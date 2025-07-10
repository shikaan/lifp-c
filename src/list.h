#pragma once

#include "alloc.h"
#include <stddef.h>
#include <string.h>

#define FlatList(Type)                                                         \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    Type *data;                                                                \
  }

typedef void unknown_t;
typedef FlatList(unknown_t) generic_flat_list_t;

constexpr size_t LIST_STRIDE = 16;

result_alloc_t listAlloc(size_t capacity, size_t list_size, size_t item_size);
void listDealloc(generic_flat_list_t *self);
result_alloc_t listPush(generic_flat_list_t *self, size_t item_size,
                        const void *item);