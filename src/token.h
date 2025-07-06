#pragma once

#include "./alloc.h"
#include "./position.h"
#include <stdint.h>

constexpr size_t SYMBOL_SIZE = 16;
constexpr size_t TOKEN_LIST_STRIDE = 32;

typedef enum {
  TOKEN_TYPE_LPAREN,
  TOKEN_TYPE_RPAREN,
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_INTEGER,
} token_type_t;

typedef union {
  char symbol[SYMBOL_SIZE];
  nullptr_t lparen;
  nullptr_t rparen;
  int32_t integer;
} token_value_t;

typedef struct {
  token_value_t value;
  position_t position;
  token_type_t type;
} token_t;

bool tokenEql(const token_t *self, const token_t *other);

typedef struct {
  size_t capacity;
  size_t size;
  token_t *data;
} token_list_t;

bool tokenListEql(const token_list_t *self, const token_list_t *other);
void tokenListDealloc(token_list_t *self);
result_alloc_t tokenListAlloc(size_t capacity);

typedef ResultVoid() result_token_list_push_t;
result_token_list_push_t tokenListPush(token_list_t *self, token_t token);
