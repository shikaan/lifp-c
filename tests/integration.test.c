#include "test.h"

#include "../src/arena.h"
#include "../src/evaluate.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_ast_arena;
static arena_t *test_vm_arena;

result_valuep_t execute(const char *input) {
  char input_copy[1024];
  strcpy(input_copy, input);

  char *line = strtok(input_copy, "\n");
  result_valuep_t last_result;

  result_alloc_t allocation = environmentCreate(test_vm_arena, nullptr);
  assert(allocation.ok);
  environment_t *env = allocation.value;

  while (line != NULL) {
    result_token_list_t tokenization = tokenize(test_ast_arena, line);
    assert(tokenization.ok);
    token_list_t *tokens = tokenization.value;

    size_t offset = 0;
    size_t depth = 0;
    result_node_t parsing = parse(test_ast_arena, tokens, &offset, &depth);
    assert(parsing.ok);
    node_t *syntax_tree = parsing.value;

    result_valuep_t reduction = evaluate(test_ast_arena, syntax_tree, env);
    assert(reduction.ok);
    last_result = reduction;

    line = strtok(nullptr, "\n");
    arenaReset(test_ast_arena);
  }

  arenaReset(test_vm_arena);
  return last_result;
}

int main() {
  result_alloc_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.ok);
  test_ast_arena = allocation.value;

  allocation = arenaCreate((size_t)(1048 * 1024));
  assert(allocation.ok);
  test_vm_arena = allocation.value;

  case("number");
  result_valuep_t reduction = execute("1");
  expectEqlInt(reduction.value->value.integer, 1, "returns correct value");

  case("symbol");
  reduction = execute("+");
  expectNotNull(reduction.value->value.builtin, "returns correct value");

  case("list");
  reduction = execute("(1 2)");
  expectEqlUint((unsigned int)reduction.value->value.list.count, 2, "returns a list"); 
  value_t first = listGet(value_t, &reduction.value->value.list,  0); 
  expectEqlInt(first.value.integer, 1, "correct first item"); 
  value_t second = listGet(value_t, &reduction.value->value.list, 1);
  expectEqlInt(second.value.integer, 2, "correct second item");

  case("simple form");
  reduction = execute("(+ 1 2)");
  expectEqlInt(reduction.value->value.integer, 3, "returns correct value");

  case("nested form");
  reduction = execute("(+ 1 (+ 2 4))");
  expectEqlInt(reduction.value->value.integer, 7, "returns correct value");

  case("declare function");
  reduction = execute("(def! sum (fn* (a b) (+ a b)))\n(sum 1 2)");
  expectEqlInt(reduction.value->value.integer, 3, "returns correct value");

  return report();
}
