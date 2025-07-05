#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "position.h"
#include "result.h"

constexpr size_t TOKEN_LIST_STRIDE = 32;
typedef ResultVoid() result_token_list_push_t;

static result_token_list_push_t tokenListPush(token_list_t *self,
                                              token_t token) {
  if (self->size == self->capacity) {
    self->capacity += TOKEN_LIST_STRIDE;
    result_alloc_t realloc_result =
        reallocSafe(self->data, self->capacity * sizeof(token_t));
    if (!realloc_result.ok) {
      return result__error(result_token_list_push_t, realloc_result.error.kind,
                           realloc_result.error.payload);
    }
    self->data = realloc_result.value;
  }

  self->data[self->size] = token;
  self->size++;
  return (result_token_list_push_t){.ok = true};
}

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

bool tokenEql(token_t *self, token_t *other) {
  if (self->type != other->type ||
      !positionEql(self->position, other->position)) {
    return false;
  }

  switch (self->type) {
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
    return true;
  case TOKEN_TYPE_INTEGER:
    return self->value.integer == other->value.integer;
  case TOKEN_TYPE_SYMBOL:
    return strncmp(self->value.symbol, other->value.symbol, SYMBOL_SIZE) == 0;
  default:
    return false;
  }
}

void tokenDealloc(token_t *self) { deallocSafe(self); }

bool tokenListEql(token_list_t *self, token_list_t *other) {
  if (self->size != other->size) {
    return false;
  }

  for (size_t i = 0; i < self->size; i++) {
    if (!tokenEql(&self->data[i], &other->data[i]))
      return false;
  }

  return true;
}

void tokenListDealloc(token_list_t *self) {
  deallocSafe(self->data);
  deallocSafe(self);
}

result_alloc_t tokenListAlloc(size_t initial_capacity) {
  result_alloc_t list_result = allocSafe(sizeof(token_list_t));
  if (!list_result.ok) {
    return list_result;
  }

  token_list_t *tokens = list_result.value;
  tokens->capacity = initial_capacity;
  tokens->size = 0;

  result_alloc_t data_result =
      allocSafe(sizeof(tokens->data[0]) * tokens->capacity);
  if (!data_result.ok) {
    return data_result;
  }

  tokens->data = data_result.value;
  return result__ok(result_alloc_t, tokens);
}