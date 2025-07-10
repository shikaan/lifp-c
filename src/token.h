#pragma once

#include "./alloc.h"
#include "./position.h"
#include "list.h"
#include <stdint.h>

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
  const token_value_t value;
  const position_t position;
  const token_type_t type;
} token_t;

bool tokenEql(const token_t *self, const token_t *other);

typedef FlatList(token_t) token_list_t;

bool tokenListEql(const token_list_t *self, const token_list_t *other);
void tokenListDealloc(token_list_t *self);
result_alloc_t tokenListAlloc(size_t capacity);

result_alloc_t tokenListPush(token_list_t *self, const token_t *token);
