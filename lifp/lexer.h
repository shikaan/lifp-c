#pragma once

#include "../lib/arena.h"
#include "../lib/list.h"
#include "../lib/result.h"
#include "position.h"
#include <stdint.h>

constexpr char LPAREN = '(';
constexpr char RPAREN = ')';

constexpr size_t SYMBOL_SIZE = 16;

typedef enum {
  TOKEN_TYPE_LPAREN,
  TOKEN_TYPE_RPAREN,
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_INTEGER,
} token_type_t;

typedef union {
  char symbol[SYMBOL_SIZE];
  int32_t integer;
  nullptr_t lparen;
  nullptr_t rparen;
} token_value_t;

typedef struct {
  token_value_t value;
  position_t position;
  token_type_t type;
} token_t;

typedef List(token_t) token_list_t;
typedef Result(token_list_t *, position_t) result_token_list_ref_t;

result_token_list_ref_t tokenize(arena_t *arena, const char *source);
