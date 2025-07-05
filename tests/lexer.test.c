#include "../src/lexer.h"
#include "./test.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  const char *input;
  token_list_t *expected;
  const char *name;
} test_case_t;

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

static token_list_t *makeTokenList(const token_t *elements, size_t size) {
  token_list_t *list = malloc(sizeof(token_list_t));
  list->size = size;
  list->capacity = size;
  list->data = malloc(size * sizeof(token_t));
  memcpy(list->data, elements, size * sizeof(token_t));
  return list;
}

void atoms() {
  const position_t POSITION = {.column = 1, .line = 1};

  // Single token cases
  token_t lparen_token = {.type = TOKEN_TYPE_LPAREN,
                          .value = {.lparen = nullptr},
                          .position = POSITION};
  token_t rparen_token = {.type = TOKEN_TYPE_RPAREN,
                          .value = {.rparen = nullptr},
                          .position = POSITION};
  token_t integer_token = {.type = TOKEN_TYPE_INTEGER,
                           .value = {.integer = 1},
                           .position = POSITION};
  token_t symbol_token = {.type = TOKEN_TYPE_SYMBOL,
                          .value = {.symbol = {'a'}},
                          .position = POSITION};

  // Multi-token
  token_t single_chars_tokens[2] = {{.type = TOKEN_TYPE_INTEGER,
                                     .value = {.integer = 1},
                                     .position = POSITION},
                                    {.type = TOKEN_TYPE_INTEGER,
                                     .value = {.integer = 2},
                                     .position = {.column = 2, .line = 1}}};

  const test_case_t CASES[] = {
      {.input = "(",
       .expected = makeTokenList(&lparen_token, 1),
       .name = "lparen"},
      {.input = ")",
       .expected = makeTokenList(&rparen_token, 1),
       .name = "rparen"},
      {.input = "1",
       .expected = makeTokenList(&integer_token, 1),
       .name = "integer"},
      {.input = "a",
       .expected = makeTokenList(&symbol_token, 1),
       .name = "symbol"},
      {.input = "12",
       .expected = makeTokenList(single_chars_tokens, 2),
       .name = "only single chars"},
  };

  for (size_t i = 0; i < array_len(CASES); i++) {
    const test_case_t TEST_CASE = CASES[i];
    auto list_result = tokenize(TEST_CASE.input);

    test__case(TEST_CASE.name);
    test__expectTrue(list_result.ok, "doesn't fail");
    test__expect(tokenListEql(TEST_CASE.expected, list_result.value),
                 "returns correct list", "Expected token lists to be equal.");

    tokenListFree(TEST_CASE.expected);
    tokenListFree(list_result.value);
  }
}

int main(void) {
  test__suite(atoms);
  return test__report();
}
