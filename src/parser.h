#pragma once

#include "position.h"
#include "result.h"
#include "token.h"
#include <stddef.h>
#include <stdint.h>

constexpr char TRUE[] = "true";
constexpr char FALSE[] = "false";
constexpr char NIL[] = "nil";

// Forward declarations
typedef struct node_t node_t;
typedef union node_value_t node_value_t;

typedef enum {
  NODE_TYPE_LIST,
  NODE_TYPE_INTEGER,
  NODE_TYPE_SYMBOL,
  NODE_TYPE_BOOLEAN,
  NODE_TYPE_NIL,
} node_type_t;

typedef union node_value_t {
  struct {
    size_t count;
    node_t *items;
  } list;
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

typedef Result(node_t *) result_node_t;
result_node_t parse(token_list_t *tokens, size_t *offset, size_t *depth);
