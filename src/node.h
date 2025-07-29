#pragma once

#include "arena.h"
#include "lexer.h"
#include "list.h"
#include "result.h"
#include <stddef.h>

typedef struct node_t node_t;
typedef union node_value_t node_value_t;
typedef List(node_t) node_list_t;

typedef enum {
  NODE_TYPE_LIST,
  NODE_TYPE_INTEGER,
  NODE_TYPE_SYMBOL,
  NODE_TYPE_BOOLEAN,
  NODE_TYPE_NIL,
} node_type_t;

typedef union node_value_t {
  node_list_t list;
  int32_t integer;
  char symbol[SYMBOL_SIZE];
  bool boolean;
  nullptr_t nil;
} node_value_t;

typedef struct node_t {
  position_t position;
  node_type_t type;
  node_value_t value;
} node_t;

result_ref_t nodeCreate(arena_t *arena, node_type_t type);
result_void_t nodeCopy(const node_t *source, node_t *destination);
