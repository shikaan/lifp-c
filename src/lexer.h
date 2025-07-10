#pragma once

#include "./list.h"
#include "./result.h"
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
  const token_value_t value;
  const position_t position;
  const token_type_t type;
} token_t;

typedef List(token_t) token_list_t;
typedef Result(token_list_t *) result_token_list_t;

result_token_list_t tokenize(const char *source);
