#include "../src/alloc.h"
#include "../src/arena.h"
#include "../src/environment.h"
#include "../src/evaluate.h"
#include "../src/lexer.h"
#include "../src/node.h"
#include "../src/parser.h"
#include "../src/print.h"
#include <readline/readline.h> //TODO this is platform dependent
#include <stddef.h>
#include <stdio.h>
#include <string.h>

constexpr size_t BUFFER_SIZE = 4096;
constexpr size_t AST_MEMORY = (size_t)(1024 * 1024);
constexpr size_t VM_MEMORY = (size_t)(1024 * 1024);

void printError(const char *msg) {
  printf("!! Error: %s\n", msg); // TODO: create print error function
}

int main(void) {
  char buffer[BUFFER_SIZE];
  result_alloc_t creation = arenaCreate(AST_MEMORY);
  if (!creation.ok) {
    fprintf(stderr, "unable to allocate memory");
    return 1;
  }
  arena_t *ast_arena = creation.value;

  creation = arenaCreate(VM_MEMORY);
  if (!creation.ok) {
    fprintf(stderr, "unable to allocate memory");
    return 1;
  }
  arena_t *vm_arena = creation.value;

  creation = environmentCreate(vm_arena, nullptr);
  if (!creation.ok) {
    fprintf(stderr, "unable to allocate memory");
    return 1;
  }
  environment_t *environment = creation.value;

  while (true) {
    arenaReset(ast_arena);
    char *input = readline("> ");
    result_token_list_t tokenization = tokenize(ast_arena, input);
    if (!tokenization.ok) {
      printError("tokenization");
      continue;
    }
    token_list_t *tokens = tokenization.value;

    size_t offset = 0;
    size_t depth = 0;
    result_node_t parsing = parse(ast_arena, tokens, &offset, &depth);
    if (!parsing.ok) {
      printError("parsing");
      continue;
    }
    node_t *syntax_tree = parsing.value;

    result_reduce_t reduction = reduce(ast_arena, syntax_tree, environment);
    if (!reduction.ok) {
      printError("reduction");
      continue;
    }
    node_t *reduced = reduction.value;

    int buffer_offset = 0;
    print(reduced, BUFFER_SIZE, buffer, &buffer_offset);

    printf("~> %s\n", buffer);

    memset(buffer, 0, BUFFER_SIZE);
  }
  arenaDestroy(ast_arena);
  arenaDestroy(vm_arena);
  return 0;
}
