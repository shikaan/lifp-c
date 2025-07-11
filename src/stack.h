#pragma once

#include "alloc.h"
#include <stddef.h>
#include <string.h>

#define Stack(Type)                                                            \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    const size_t item_size;                                                    \
    Type *data;                                                                \
  }

typedef Stack(void) generic_stack_t;

constexpr size_t STACK_STRIDE = 16;

// TODO: find a meaningful name
result_alloc_t _stackAlloc(size_t capacity, size_t stack_size,
                           size_t item_size);
void _stackDealloc(generic_stack_t *self);
result_alloc_t _stackPush(generic_stack_t *self, const void *item);

#define stackAlloc(ItemType, Capacity)                                         \
  _stackAlloc((Capacity), sizeof(Stack(ItemType)), sizeof(ItemType))

#define stackDealloc(Stack) _stackDealloc((generic_stack_t *)(Stack))

#define stackPush(Stack, Item) _stackPush((generic_stack_t *)(Stack), Item)
