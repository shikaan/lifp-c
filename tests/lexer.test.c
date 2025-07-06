#include "../src/lexer.h"
#include "./test.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

static token_list_t *makeTokenList(const token_t *elements, size_t size) {
  result_alloc_t allocation = tokenListAlloc(size);
  assert(allocation.ok);
  token_list_t *list = allocation.value;
  for (size_t i = 0; i < size; i++) {
    tokenListPush(list, elements[i]);
  }
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

  const struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } CASES[] = {
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
    auto list_result = tokenize(CASES[i].input);

    test__case(CASES[i].name);
    test__expectTrue(list_result.ok, "doesn't fail");
    test__expect(tokenListEql(CASES[i].expected, list_result.value),
                 "returns correct list", "Expected token lists to be equal.");

    tokenListDealloc(CASES[i].expected);
    tokenListDealloc(list_result.value);
  }
}

void whitespaces() {
  token_t token = {.type = TOKEN_TYPE_SYMBOL,
                   .value = {.symbol = {'a'}},
                   .position = {.column = 2, .line = 1}};
  token_t other_token = {.type = TOKEN_TYPE_SYMBOL,
                         .value = {.symbol = {'b'}},
                         .position = {.column = 1, .line = 2}};

  const struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } CASES[] = {
      {.input = " a",
       .expected = makeTokenList(&token, 1),
       .name = "blank space"},
      {.input = "\ta",
       .expected = makeTokenList(&token, 1),
       .name = "tab space"},
      {.input = "\ra",
       .expected = makeTokenList(&token, 1),
       .name = "carriage return"},
      {.input = "\nb\r",
       .expected = makeTokenList(&other_token, 1),
       .name = "new line"},
  };

  for (size_t i = 0; i < array_len(CASES); i++) {
    auto list_result = tokenize(CASES[i].input);

    test__case(CASES[i].name);
    test__expectTrue(list_result.ok, "doesn't fail");
    test__expect(tokenListEql(CASES[i].expected, list_result.value),
                 "returns correct list", "Expected token lists to be equal.");

    tokenListDealloc(CASES[i].expected);
    tokenListDealloc(list_result.value);
  }
}

void error() {
  const struct {
    const char *input;
    size_t column;
    size_t line;
    char token;
    const char *name;
  } CASES[] = {{"@", 1, 1, '@', "unexpected character"},
               {"a$", 2, 1, '$', "unexpected character with symbol"},
               {"1#", 2, 1, '#', "unexpected character with integer"},
               {"(\x01)", 2, 1, '\x01', "unexpected control char"}};

  for (size_t i = 0; i < array_len(CASES); i++) {
    auto result = tokenize(CASES[i].input);
    test__case(CASES[i].name);
    test__expectFalse(result.ok, "should fail");
    char msg[128];
    snprintf(msg, 128, "Expected UNEXPECTED_TOKEN got %u\n", result.error.kind);
    test__expect(result.error.kind == EXCEPTION_KIND_UNEXPECTED_TOKEN,
                 "error is ERROR_UNEXPECTED_TOKEN", msg);
    test__expectEqlSize(result.error.payload.unexpected_token.position.column,
                        CASES[i].column, "column matches");
    test__expectEqlSize(result.error.payload.unexpected_token.position.line,
                        CASES[i].line, "line matches");
    snprintf(msg, 128, "Expected '%c' got '%c'\n", CASES[i].token,
             result.error.payload.unexpected_token.token);
    test__expect(result.error.payload.unexpected_token.token == CASES[i].token,
                 "token matches", msg);
  }
}

int main(void) {
  test__suite(atoms);
  test__suite(whitespaces);
  test__suite(error);
  return test__report();
}
