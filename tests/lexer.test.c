#include "../src/lexer.h"
#include "./test.h"
#include "./utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

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

void atoms() {
  // Single token cases
  token_t lparen_token = tParen('(');
  token_t rparen_token = tParen(')');
  token_t integer_token = tInt(1);
  // token_t symbol_token = tSym("a");
  token_t symbol_token = {.type = TOKEN_TYPE_SYMBOL,
                          .value.symbol = {'a'},
                          .position.column = 1,
                          .position.line = 1};

  // Multi-token
  token_t single_chars_tokens[2] = {tInt(1),
                                    {.type = TOKEN_TYPE_INTEGER,
                                     .value = {.integer = 2},
                                     .position = {.column = 2, .line = 1}}};

  const struct {
    const char *input;
    token_list_t *expected;
    const char *name;
  } cases[] = {
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

  for (size_t i = 0; i < arraySize(cases); i++) {
    auto list_result = tokenize(cases[i].input);

    case(cases[i].name);
    expectTrue(list_result.ok, "doesn't fail");
    expect(tokenListEql(cases[i].expected, list_result.value),
                 "returns correct list", "Expected token lists to be equal.");

    listDealloc((generic_flat_list_t*)cases[i].expected);
    listDealloc((generic_flat_list_t*)list_result.value);
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
  } cases[] = {
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

  for (size_t i = 0; i < arraySize(cases); i++) {
    auto list_result = tokenize(cases[i].input);

    case(cases[i].name);
    expectTrue(list_result.ok, "doesn't fail");
    expect(tokenListEql(cases[i].expected, list_result.value),
                 "returns correct list", "Expected token lists to be equal.");

    listDealloc((generic_flat_list_t*)cases[i].expected);
    listDealloc((generic_flat_list_t*)list_result.value);
  }
}

void errors() {
  const struct {
    const char *input;
    size_t column;
    size_t line;
    char token;
    const char *name;
  } cases[] = {{"@", 1, 1, '@', "unexpected character"},
               {"a$", 2, 1, '$', "unexpected character with symbol"},
               {"1#", 2, 1, '#', "unexpected character with integer"},
               {"(\x01)", 2, 1, '\x01', "unexpected control char"}};

  for (size_t i = 0; i < arraySize(cases); i++) {
    auto result = tokenize(cases[i].input);
    case(cases[i].name);
    expectFalse(result.ok, "should fail");
    char msg[128];
    snprintf(msg, 128, "Expected UNEXPECTED_TOKEN got %u\n", result.error.kind);
    expect(result.error.kind == EXCEPTION_KIND_UNEXPECTED_TOKEN,
                 "error is ERROR_UNEXPECTED_TOKEN", msg);
    expectEqlSize(result.error.payload.unexpected_token.position.column,
                        cases[i].column, "column matches");
    expectEqlSize(result.error.payload.unexpected_token.position.line,
                        cases[i].line, "line matches");
    snprintf(msg, 128, "Expected '%c' got '%c'\n", cases[i].token,
             result.error.payload.unexpected_token.token);
    expect(result.error.payload.unexpected_token.token == cases[i].token,
                 "token matches", msg);
  }
}

int main(void) {
  suite(atoms);
  suite(whitespaces);
  suite(errors);
  return report();
}
