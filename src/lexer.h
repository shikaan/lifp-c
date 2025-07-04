#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include "./result.h"

typedef enum {
    TOKEN_TYPE_lparen,
    TOKEN_TYPE_rparen,
    TOKEN_TYPE_symbol,
    TOKEN_TYPE_integer,
  } TOKEN_TYPE;

typedef union {
    void* lparen;
    void* rparen;
    const char *symbol;
    int32_t integer;
} TokenValue;

typedef struct {
    TOKEN_TYPE type;
    TokenValue value;
} Token;

typedef Result(Token*) ResultTokenList;

ResultTokenList tokenize(const char *source);

#endif //LEXER_H
