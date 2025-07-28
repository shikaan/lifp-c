#include "../src/arena.h"
#include "../src/environment.h"
#include "../src/evaluate.h"
#include "../src/fmt.h"
#include "../src/lexer.h"
#include "../src/node.h"
#include "../src/parser.h"
#include <readline/history.h>  //TODO this is platform dependent
#include <readline/readline.h> //TODO this is platform dependent
#include <stddef.h>
#include <stdio.h>
#include <string.h>

constexpr size_t BUFFER_SIZE = 4096;
constexpr size_t AST_MEMORY = (size_t)(1024 * 1024);
constexpr size_t VM_MEMORY = (size_t)(1024 * 1024);

void printError(const error_t *error, const char *input_buffer, int size,
                char output_buffer[static size]) {
  int offset = 0;
  formatError(error, input_buffer, "repl", size, output_buffer, &offset);
  fprintf(stdout, "!! %s\n", output_buffer);
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

  using_history();

  while (true) {
    arenaReset(ast_arena);
    char *input = readline("> ");
    result_token_list_t tokenization = tokenize(ast_arena, input);
    if (!tokenization.ok) {
      printError(&tokenization.error, input, BUFFER_SIZE, buffer);
      continue;
    }
    token_list_t *tokens = tokenization.value;
    add_history(input);

    size_t offset = 0;
    size_t depth = 0;
    result_node_t parsing = parse(ast_arena, tokens, &offset, &depth);
    if (!parsing.ok) {
      printError(&parsing.error, input, BUFFER_SIZE, buffer);
      continue;
    }
    node_t *syntax_tree = parsing.value;

    result_reduce_t reduction = reduce(ast_arena, syntax_tree, environment);
    if (!reduction.ok) {
      printError(&reduction.error, input, BUFFER_SIZE, buffer);
      continue;
    }
    value_t *reduced = reduction.value;

    int buffer_offset = 0;
    formatValue(reduced, BUFFER_SIZE, buffer, &buffer_offset);
    printf("~> %s\n", buffer);

    memset(buffer, 0, BUFFER_SIZE);
  }
  arenaDestroy(ast_arena);
  arenaDestroy(vm_arena);
  return 0;
}
