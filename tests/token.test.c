#include "../src/token.h"
#include "./test.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

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

static void test__expectEqlToken(const token_t *a, const token_t *b,
                                 const char *name) {
  char msg[256];
  char bufa[64];
  char bufb[64];
  tokenToString(a, 64, bufa);
  tokenToString(b, 64, bufb);
  snprintf(msg, 256, "Expected %s to equal '%s'", bufa, bufb);
  test__expect(tokenEql(a, b), name, msg);
}
static void test__expectNeqToken(const token_t *a, const token_t *b,
                                 const char *name) {
  char msg[256];
  char bufa[64];
  char bufb[64];
  tokenToString(a, 64, bufa);
  tokenToString(b, 64, bufb);
  snprintf(msg, 256, "Expected %s not to equal '%s'", bufa, bufb);
  test__expect(!tokenEql(a, b), name, msg);
}

static token_list_t *makeTokenList(const token_t *elements, size_t size) {
  result_alloc_t allocation = tokenListAlloc(size);
  assert(allocation.ok);
  token_list_t *list = allocation.value;
  for (size_t i = 0; i < size; i++) {
    tokenListPush(list, elements[i]);
  }
  return list;
}

void equality() {
  const position_t POSITION = {.column = 1, .line = 1};
  token_t tok1 = {.type = TOKEN_TYPE_INTEGER,
                  .value = {.integer = 42},
                  .position = POSITION};
  token_t tok2 = {.type = TOKEN_TYPE_INTEGER,
                  .value = {.integer = 42},
                  .position = POSITION};
  token_t tok3 = {.type = TOKEN_TYPE_INTEGER,
                  .value = {.integer = 43},
                  .position = POSITION};
  token_t tok4 = {.type = TOKEN_TYPE_SYMBOL,
                  .value = {.symbol = {'a'}},
                  .position = POSITION};
  token_t tok5 = {.type = TOKEN_TYPE_SYMBOL,
                  .value = {.symbol = {'a'}},
                  .position = POSITION};
  token_t tok6 = {.type = TOKEN_TYPE_SYMBOL,
                  .value = {.symbol = {'b'}},
                  .position = POSITION};

  test__expectEqlToken(&tok1, &tok2, "integers");
  test__expectNeqToken(&tok1, &tok3, "different integers");
  test__expectEqlToken(&tok4, &tok5, "symbols");
  test__expectNeqToken(&tok4, &tok6, "different symbols");
}

void listEquality() {
  const position_t POSITION = {.column = 1, .line = 1};
  token_t arr1[2] = {{.type = TOKEN_TYPE_INTEGER,
                      .value = {.integer = 1},
                      .position = POSITION},
                     {.type = TOKEN_TYPE_SYMBOL,
                      .value = {.symbol = {'a'}},
                      .position = POSITION}};
  token_t arr2[2] = {{.type = TOKEN_TYPE_INTEGER,
                      .value = {.integer = 1},
                      .position = POSITION},
                     {.type = TOKEN_TYPE_SYMBOL,
                      .value = {.symbol = {'a'}},
                      .position = POSITION}};
  token_t arr3[2] = {{.type = TOKEN_TYPE_INTEGER,
                      .value = {.integer = 2},
                      .position = POSITION},
                     {.type = TOKEN_TYPE_SYMBOL,
                      .value = {.symbol = {'a'}},
                      .position = POSITION}};
  token_list_t *list1 = makeTokenList(arr1, 2);
  token_list_t *list2 = makeTokenList(arr2, 2);
  token_list_t *list3 = makeTokenList(arr3, 2);

  test__expect(tokenListEql(list1, list2), "lists",
               "Expected lists to be equal");
  test__expect(!tokenListEql(list1, list3), "different lists",
               "Expected lists to be equal");

  tokenListDealloc(list1);
  tokenListDealloc(list2);
  tokenListDealloc(list3);
}

void listAllocations() {
  result_alloc_t result = tokenListAlloc(4);
  assert(result.ok);
  token_list_t *list = result.value;
  test__expectEqlSize(list->capacity, 4, "correct capacity");
  test__expectEqlSize(list->size, 0, "correct size");
  tokenListDealloc(list);
}

void listPush() {
  result_alloc_t result = tokenListAlloc(1);
  assert(result.ok);
  token_list_t *list = result.value;
  token_t token = {.type = TOKEN_TYPE_INTEGER,
                   .value = {.integer = 1},
                   .position = {.column = 1, .line = 1}};
  result_token_list_push_t push_result = tokenListPush(list, token);
  test__expectTrue(push_result.ok, "pushes");
  test__expectEqlSize(list->size, 1, "increases size");
  test__expectEqlSize(list->capacity, 1, "doesn't change capacity");
  test__expectEqlToken(&list->data[0], &token, "value is correct");

  result = tokenListAlloc(1);
  test__case("with resize");
  assert(result.ok);
  list = result.value;
  for (size_t i = 0; i < 5; i++) {
    token.value.integer = (int)i;
    push_result = tokenListPush(list, token);
    assert(push_result.ok);
  }
  test__expectEqlSize(list->size, 5, "updates size");
  test__expectEqlSize(list->capacity, TOKEN_LIST_STRIDE + 1,
                      "updates capacity");
  for (int i = 0; i < 5; i++) {
    test__expectEqlInt(list->data[i].value.integer, i, "value is correct");
  }
  tokenListDealloc(list);
}

int main(void) {
  test__suite(equality);
  test__suite(listEquality);
  test__suite(listAllocations);
  test__suite(listPush);
  return test__report();
}
