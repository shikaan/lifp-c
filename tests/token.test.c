#include "../src/token.h"
#include "./test.h"
#include "./utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void tokenToString(const token_t *token, size_t n, char dst[static n]) {
  switch (token->type) {
  case TOKEN_TYPE_LPAREN:
    snprintf(dst, n, "token(LPAREN, '(', [%lu, %lu])", token->position.column,
             token->position.line);
    return;
  case TOKEN_TYPE_RPAREN:
    snprintf(dst, n, "token(RPAREN, ')', [%lu, %lu])", token->position.column,
             token->position.line);
    return;
  case TOKEN_TYPE_INTEGER:
    snprintf(dst, n, "token(INTEGER, %d, [%lu, %lu])", token->value.integer,
             token->position.column, token->position.line);
    return;
  case TOKEN_TYPE_SYMBOL:
    snprintf(dst, n, "token(SYMBOL, %s, [%lu, %lu])", token->value.symbol,
             token->position.column, token->position.line);
    return;
  default:
    return;
  }
}

static void expectEqlToken(const token_t *a, const token_t *b,
                           const char *name) {
  char msg[256];
  char bufa[64];
  char bufb[64];
  tokenToString(a, 64, bufa);
  tokenToString(b, 64, bufb);
  snprintf(msg, 256, "Expected %s to equal '%s'", bufa, bufb);
  expect(tokenEql(a, b), name, msg);
}
static void expectNeqToken(const token_t *a, const token_t *b,
                           const char *name) {
  char msg[256];
  char bufa[64];
  char bufb[64];
  tokenToString(a, 64, bufa);
  tokenToString(b, 64, bufb);
  snprintf(msg, 256, "Expected %s not to equal '%s'", bufa, bufb);
  expect(!tokenEql(a, b), name, msg);
}

void equality() {
  const position_t position = {.column = 1, .line = 1};
  token_t tok1 = {.type = TOKEN_TYPE_INTEGER,
                  .value = {.integer = 42},
                  .position = position};
  token_t tok2 = {.type = TOKEN_TYPE_INTEGER,
                  .value = {.integer = 42},
                  .position = position};
  token_t tok3 = {.type = TOKEN_TYPE_INTEGER,
                  .value = {.integer = 43},
                  .position = position};
  token_t tok4 = {.type = TOKEN_TYPE_SYMBOL,
                  .value = {.symbol = {'a'}},
                  .position = position};
  token_t tok5 = {.type = TOKEN_TYPE_SYMBOL,
                  .value = {.symbol = {'a'}},
                  .position = position};
  token_t tok6 = {.type = TOKEN_TYPE_SYMBOL,
                  .value = {.symbol = {'b'}},
                  .position = position};

  expectEqlToken(&tok1, &tok2, "integers");
  expectNeqToken(&tok1, &tok3, "different integers");
  expectEqlToken(&tok4, &tok5, "symbols");
  expectNeqToken(&tok4, &tok6, "different symbols");
}

void listEquality() {
  const position_t position = {.column = 1, .line = 1};
  token_t arr1[2] = {{.type = TOKEN_TYPE_INTEGER,
                      .value = {.integer = 1},
                      .position = position},
                     {.type = TOKEN_TYPE_SYMBOL,
                      .value = {.symbol = {'a'}},
                      .position = position}};
  token_t arr2[2] = {{.type = TOKEN_TYPE_INTEGER,
                      .value = {.integer = 1},
                      .position = position},
                     {.type = TOKEN_TYPE_SYMBOL,
                      .value = {.symbol = {'a'}},
                      .position = position}};
  token_t arr3[2] = {{.type = TOKEN_TYPE_INTEGER,
                      .value = {.integer = 2},
                      .position = position},
                     {.type = TOKEN_TYPE_SYMBOL,
                      .value = {.symbol = {'a'}},
                      .position = position}};
  token_list_t *list1 = makeTokenList(arr1, 2);
  token_list_t *list2 = makeTokenList(arr2, 2);
  token_list_t *list3 = makeTokenList(arr3, 2);

  expect(tokenListEql(list1, list2), "lists", "Expected lists to be equal");
  expect(!tokenListEql(list1, list3), "different lists",
         "Expected lists to be equal");

  tokenListDealloc(list1);
  tokenListDealloc(list2);
  tokenListDealloc(list3);
}

void listAllocations() {
  result_alloc_t result = tokenListAlloc(4);
  assert(result.ok);
  token_list_t *list = result.value;
  expectEqlSize(list->capacity, 4, "correct capacity");
  expectEqlSize(list->size, 0, "correct size");
  tokenListDealloc(list);
}

void listPush() {
  result_alloc_t result = tokenListAlloc(1);
  assert(result.ok);
  token_list_t *list = result.value;
  token_t token = tInt(1);
  result_token_list_push_t push_result = tokenListPush(list, &token);
  expectTrue(push_result.ok, "pushes");
  expectEqlSize(list->size, 1, "increases size");
  expectEqlSize(list->capacity, 1, "doesn't change capacity");
  expectEqlToken(&list->data[0], &token, "value is correct");

  result = tokenListAlloc(1);
  case("with resize");
  assert(result.ok);
  list = result.value;
  for (int i = 0; i < 5; i++) {
    const token_t result_token = tInt(i);
    push_result = tokenListPush(list, &result_token);
    assert(push_result.ok);
  }
  expectEqlSize(list->size, 5, "updates size");
  expectEqlSize(list->capacity, TOKEN_LIST_STRIDE + 1,
                      "updates capacity");
  for (int i = 0; i < 5; i++) {
    expectEqlInt(list->data[i].value.integer, i, "value is correct");
  }
  tokenListDealloc(list);
}

int main(void) {
  suite(equality);
  suite(listEquality);
  suite(listAllocations);
  suite(listPush);
  return report();
}
