#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"
#include "arena.h"
#include "list.h"
#include "position.h"
#include "result.h"

result_token_list_t tokenize(arena_t *arena, const char *source) {
  result_token_list_t result;
  position_t position = {.column = 0, .line = 1};

  result_alloc_t allocation = listAlloc(token_t, arena, 32);
  if (!allocation.ok) {
    return error(result_token_list_t, allocation.error);
  }
  token_list_t *tokens = allocation.value;

  for (int i = 0; source[i] != '\0'; i++) {
    position.column++;
    const char current_char = source[i];

    result_alloc_t token_push_result;
    if (current_char == LPAREN) {
      token_t tok = {.type = TOKEN_TYPE_LPAREN,
                     .value.lparen = nullptr,
                     .position = position};
      token_push_result = listAppend(tokens, &tok);
    } else if (current_char == RPAREN) {
      const token_t tok = {.type = TOKEN_TYPE_RPAREN,
                           .value = {.rparen = nullptr},
                           .position = position};
      token_push_result = listAppend(tokens, &tok);
    } else if (isspace(current_char)) {
      token_push_result.ok = true;
      if (current_char == '\n') {
        position.line++;
        position.column = 0;
      }
      continue;
    } else if (isnumber(current_char)) {
      const token_t tok = {.type = TOKEN_TYPE_INTEGER,
                           .value = {.integer = current_char - '0'},
                           .position = position};
      token_push_result = listAppend(tokens, &tok);
    } else if (isprint(current_char)) {
      const token_t tok = {.type = TOKEN_TYPE_SYMBOL,
                           .position = position,
                           .value = {.symbol = {current_char, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0}}};
      token_push_result = listAppend(tokens, &tok);
    } else {
      exception_t exception = {
          .kind = EXCEPTION_KIND_UNEXPECTED_TOKEN,
          .payload.unexpected_token.position = position,
          .payload.unexpected_token.token = current_char,
      };
      return error(result_token_list_t, exception);
    }

    if (!token_push_result.ok) {
      return error(result_token_list_t, allocation.error);
    }
  }

  result = ok(result_token_list_t, tokens);
  return result;
}
