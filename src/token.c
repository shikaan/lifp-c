#include "token.h"
#include <string.h>
#include <stddef.h>

bool tokenEql(const token_t *self, const token_t *other) {
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

bool tokenListEql(const token_list_t *self, const token_list_t *other) {
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

result_token_list_push_t tokenListPush(token_list_t *self, token_t token) {
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
