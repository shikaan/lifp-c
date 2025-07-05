#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "debug.h"
#include "position.h"
#include "result.h"

constexpr size_t BUFFER_INCREMENT = 32;

result_token_list_t tokenize(const char *source) {
  // TODO: handle allocation error
  token_list_t *tokens = malloc(sizeof(token_list_t));
  tokens->capacity = BUFFER_INCREMENT;
  tokens->size = 0;
  // TODO: handle allocation error
  tokens->data = malloc(sizeof(token_t) * BUFFER_INCREMENT);

  result_token_list_t result;
  position_t position = {
      .column = 0,
      .line = 1,
  };

  for (int i = 0; source[i] != '\0'; i++) {
    position.column++;
    const char CURRENT_CHAR = source[i];

    token_t *current_token = &tokens->data[tokens->size];
    current_token->position = position;
    if (CURRENT_CHAR == LPAREN) {
      current_token->type = TOKEN_TYPE_LPAREN;
      current_token->value.lparen = nullptr;
      tokens->size++;
    } else if (CURRENT_CHAR == RPAREN) {
      current_token->type = TOKEN_TYPE_RPAREN;
      current_token->value.rparen = nullptr;
      tokens->size++;
    } else if (isspace(CURRENT_CHAR)) {
      if (CURRENT_CHAR == '\n') {
        position.line++;
        position.column = 0;
      }
      continue;
    } else if (isnumber(CURRENT_CHAR)) {
      current_token->type = TOKEN_TYPE_INTEGER;
      current_token->value.integer = CURRENT_CHAR - '0';
      tokens->size++;
    } else if (isalnum(CURRENT_CHAR)) {
      current_token->type = TOKEN_TYPE_SYMBOL;
      memset(current_token->value.symbol, 0, SYMBOL_SIZE);
      current_token->value.symbol[0] = CURRENT_CHAR;
      tokens->size++;
    } else {
      // TODO: use err macro
      result.ok = false;
      result.error = (exception_t){.kind = ERROR_UNEXPECTED_TOKEN,
                                   .payload = {.unexpected_token = {
                                                   .position = position,
                                                   .token = CURRENT_CHAR,
                                               }}};
      goto error;
    }

    if (tokens->size == tokens->capacity) {
      tokens->capacity += BUFFER_INCREMENT;
      // TODO: handle allocation errors
      tokens->data = realloc(tokens->data, tokens->capacity * sizeof(token_t));
    }
  }

  result = ok(result_token_list_t, tokens);
  return result;
error:
  tokenListFree(tokens);
  return result;
}

bool tokenEql(token_t *first, token_t *second) {
  if (first->type != second->type ||
      !positionEql(first->position, second->position)) {
    return false;
  }

  switch (first->type) {
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
    return true;
  case TOKEN_TYPE_INTEGER:
    return first->value.integer == second->value.integer;
  case TOKEN_TYPE_SYMBOL:
    return strncmp(first->value.symbol, second->value.symbol, SYMBOL_SIZE) == 0;
  default:
    return false;
  }
}

void tokenFree(token_t *ptr) { freeNull(ptr); }

bool tokenListEql(token_list_t *first, token_list_t *second) {
  if (first->size != second->size) {
    return false;
  }

  for (size_t i = 0; i < first->size; i++) {
    if (!tokenEql(&first->data[i], &second->data[i])) {
      return false;
    }
  }

  return true;
}

void tokenListFree(token_list_t *ptr) {
  freeNull(ptr->data);
  freeNull(ptr);
}
