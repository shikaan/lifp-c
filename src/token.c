#include "token.h"
#include "alloc.h"
#include "list.h"
#include <stddef.h>
#include <string.h>

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
  if (self->count != other->count) {
    return false;
  }
  for (size_t i = 0; i < self->count; i++) {
    if (!tokenEql(&self->data[i], &other->data[i]))
      return false;
  }
  return true;
}

void tokenListDealloc(token_list_t *self) {
  listDealloc((generic_flat_list_t *)self);
}

result_alloc_t tokenListAlloc(size_t initial_capacity) {
  return listAlloc(initial_capacity, sizeof(token_list_t), sizeof(token_t));
}

result_alloc_t tokenListPush(token_list_t *self, const token_t *token) {
  return listPush((generic_flat_list_t *)self, sizeof(*token), token);
}
