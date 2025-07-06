#pragma once

#include "./result.h"
#include "token.h"
#include <stdint.h>

constexpr char LPAREN = '(';
constexpr char RPAREN = ')';

typedef Result(token_list_t *) result_token_list_t;
result_token_list_t tokenize(const char *source);
