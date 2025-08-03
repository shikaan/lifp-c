#include "../lib/arena.h"
#include "../lifp/environment.h"
#include "../lifp/evaluate.h"
#include "../lifp/fmt.h"
#include "../lifp/lexer.h"
#include "../lifp/node.h"
#include "../lifp/parser.h"
#include <readline/history.h>  //TODO this is platform dependent
#include <readline/readline.h> //TODO this is platform dependent
#include <stddef.h>
#include <stdio.h>
#include <string.h>

constexpr size_t BUFFER_SIZE = 4096;
// TODO: AST memory currently also includes transient values
constexpr size_t AST_MEMORY = (size_t)(1024 * 64);
constexpr size_t VM_MEMORY = (size_t)(1024 * 64);

#define printError(Result, InputBuffer, Size, OutputBuffer)                    \
  int _concat(offset_, __LINE__) = 0;                                          \
  formatErrorMessage((Result)->message, (Result)->meta, "repl", InputBuffer,   \
                     Size, OutputBuffer, &_concat(offset_, __LINE__));         \
  fprintf(stdout, "%s\n", OutputBuffer);

int main(void) {
  char buffer[BUFFER_SIZE];
  result_ref_t creation = arenaCreate(AST_MEMORY);
  if (creation.code != RESULT_OK) {
    fprintf(stderr, "unable to allocate memory");
    return 1;
  }
  arena_t *ast_arena = creation.value;

  creation = arenaCreate(VM_MEMORY);
  if (creation.code != RESULT_OK) {
    fprintf(stderr, "unable to allocate memory");
    return 1;
  }
  arena_t *vm_arena = creation.value;

  creation = environmentCreate(vm_arena, nullptr);
  if (creation.code != RESULT_OK) {
    fprintf(stderr, "unable to allocate memory");
    return 1;
  }
  environment_t *environment = creation.value;

  using_history();

  while (true) {
    arenaReset(ast_arena);
    char *input = readline("> ");
    result_token_list_ref_t tokenization = tokenize(ast_arena, input);
    if (tokenization.code != RESULT_OK) {
      printError(&tokenization, input, BUFFER_SIZE, buffer);
      continue;
    }
    token_list_t *tokens = tokenization.value;
    add_history(input);

    size_t offset = 0;
    size_t depth = 0;
    result_node_ref_t parsing = parse(ast_arena, tokens, &offset, &depth);
    if (parsing.code != RESULT_OK) {
      printError(&parsing, input, BUFFER_SIZE, buffer);
      continue;
    }
    node_t *syntax_tree = parsing.value;

    result_value_ref_t reduction =
        evaluate(ast_arena, syntax_tree, environment);
    if (reduction.code != RESULT_OK) {
      printError(&reduction, input, BUFFER_SIZE, buffer);
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

#undef printError