#include "test.h"

#include "../src/arena.h"
#include "../src/evaluate.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;

result_reduce_t execute(const char *input) {
  result_token_list_t tokenization = tokenize(test_arena, input);
  assert(tokenization.ok);
  token_list_t *tokens = tokenization.value;

  size_t offset = 0;
  size_t depth = 0;
  result_node_t parsing = parse(test_arena, tokens, &offset, &depth);
  assert(parsing.ok);
  node_t *syntax_tree = parsing.value;

  result_alloc_t allocation = environmentCreate(test_arena, nullptr);
  assert(allocation.ok);
  environment_t *env = allocation.value;

  result_reduce_t reduction = reduce(test_arena, syntax_tree, env);
  assert(reduction.ok);
  return reduction;
}

int main() {
  result_alloc_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.ok);
  test_arena = allocation.value;

  case("number");
  result_reduce_t reduction = execute("1");
  expectEqlInt(reduction.value->value.integer, 1, "returns correct value");

  case("symbol");
  reduction = execute("+");
  expectNotNull(reduction.value->value.function, "returns correct value");

  case("list");
  reduction = execute("(1 2)");
  expectEqlUint((unsigned int)reduction.value->value.list.count, 2, "returns a list");
  value_t first = listGet(value_t, &reduction.value->value.list, 0);
  expectEqlInt(first.value.integer, 1, "correct first item");
  value_t second = listGet(value_t, &reduction.value->value.list, 1);
  expectEqlInt(second.value.integer, 2, "correct second item");
  
  case("simple form");
  reduction = execute("(+ 1 2)");
  expectEqlInt(reduction.value->value.integer, 3, "returns correct value");

  case("nested form");
  reduction = execute("(+ 1 (+ 2 4))");
  expectEqlInt(reduction.value->value.integer, 7, "returns correct value");

  return report();
}
