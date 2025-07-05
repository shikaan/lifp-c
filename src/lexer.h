#ifndef LEXER_H
#define LEXER_H

#include "./position.h"
#include "./result.h"
#include "alloc.h"
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
  nullptr_t lparen;
  nullptr_t rparen;
  int32_t integer;
} token_value_t;

typedef struct {
  token_value_t value;
  position_t position;
  token_type_t type;
} token_t;

bool tokenEql(token_t *self, token_t *other);
void tokenDealloc(token_t *self);

typedef struct {
  size_t capacity;
  size_t size;
  token_t *data;
} token_list_t;

bool tokenListEql(token_list_t *self, token_list_t *other);
void tokenListDealloc(token_list_t *self);
result_alloc_t tokenListAlloc(size_t capacity);

typedef Result(token_list_t *) result_token_list_t;
result_token_list_t tokenize(const char *source);

#endif // LEXER_H
