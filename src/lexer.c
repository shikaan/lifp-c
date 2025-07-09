#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "position.h"
#include "result.h"
#include "token.h"

result_token_list_t tokenize(const char *source) {
  result_alloc_t tokens_result = tokenListAlloc(TOKEN_LIST_STRIDE);

  if (!tokens_result.ok) {
    return result__error(result_token_list_t, tokens_result.error.kind,
                         tokens_result.error.payload);
  }

  result_token_list_t result;
  token_list_t *tokens = tokens_result.value;
  position_t position = {
      .column = 0,
      .line = 1,
  };

  for (int i = 0; source[i] != '\0'; i++) {
    position.column++;
    const char current_char = source[i];

    result_token_list_push_t token_push_result;
    if (current_char == LPAREN) {
      const token_t tok = {.type = TOKEN_TYPE_LPAREN,
                           .value = {.lparen = nullptr},
                           .position = position};
      token_push_result = tokenListPush(tokens, &tok);
    } else if (current_char == RPAREN) {
      const token_t tok = {.type = TOKEN_TYPE_RPAREN,
                           .value = {.rparen = nullptr},
                           .position = position};
      token_push_result = tokenListPush(tokens, &tok);
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
      token_push_result = tokenListPush(tokens, &tok);
    } else if (isalnum(current_char)) {
      const token_t tok = {.type = TOKEN_TYPE_SYMBOL,
                           .position = position,
                           .value = {.symbol = {current_char, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0}}};
      token_push_result = tokenListPush(tokens, &tok);
    } else {
      exception_payload_t payload = {.unexpected_token = {
                                         .position = position,
                                         .token = current_char,
                                     }};
      result = result__error(result_token_list_t,
                             EXCEPTION_KIND_UNEXPECTED_TOKEN, payload);
      goto error;
    }

    if (!token_push_result.ok) {
      result = result__error(result_token_list_t, tokens_result.error.kind,
                             token_push_result.error.payload);
      goto error;
    }
  }

  result = result__ok(result_token_list_t, tokens);
  return result;
error:
  tokenListDealloc(tokens);
  return result;
}
