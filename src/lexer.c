#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "position.h"
#include "result.h"

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
    const char CURRENT_CHAR = source[i];

    result_token_list_push_t token_push_result;
    if (CURRENT_CHAR == LPAREN) {
      token_push_result =
          tokenListPush(tokens, (token_t){.type = TOKEN_TYPE_LPAREN,
                                          .value = {.lparen = nullptr},
                                          .position = position});
    } else if (CURRENT_CHAR == RPAREN) {
      token_push_result =
          tokenListPush(tokens, (token_t){.type = TOKEN_TYPE_RPAREN,
                                          .value = {.rparen = nullptr},
                                          .position = position});
    } else if (isspace(CURRENT_CHAR)) {
      token_push_result.ok = true;
      if (CURRENT_CHAR == '\n') {
        position.line++;
        position.column = 0;
      }
      continue;
    } else if (isnumber(CURRENT_CHAR)) {
      token_push_result = tokenListPush(
          tokens, (token_t){.type = TOKEN_TYPE_INTEGER,
                            .value = {.integer = CURRENT_CHAR - '0'},
                            .position = position});
    } else if (isalnum(CURRENT_CHAR)) {
      token_t token = {.type = TOKEN_TYPE_SYMBOL, .position = position};
      memset(token.value.symbol, 0, SYMBOL_SIZE);
      token.value.symbol[0] = CURRENT_CHAR;
      token_push_result = tokenListPush(tokens, token);
    } else {
      exception_payload_t payload = {.unexpected_token = {
                                         .position = position,
                                         .token = CURRENT_CHAR,
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
