
#include "src/alloc.h"
#include "src/evaluate.h"
#include "src/lexer.h"
#include "src/node.h"
#include "src/parser.h"
#include "src/print.h"
#include <readline/readline.h> //TODO this is not platform dependent
#include <stddef.h>
#include <stdio.h>
#include <string.h>

constexpr size_t BUFFER_SIZE = 4096;

void printError(const char *msg) {
  printf("!! Error: %s\n", msg); // TODO: create print error function
}

int main(void) {
  bool exit = false;
  char buffer[BUFFER_SIZE];
  while (!exit) {
    char *input = readline("> ");
    result_token_list_t tokenization = tokenize(input);
    if (!tokenization.ok) {
      printError("tokenization");
      continue;
    }
    token_list_t *tokens = tokenization.value;

    size_t offset = 0;
    size_t depth = 0;
    result_node_t parsing = parse(tokens, &offset, &depth);
    if (!parsing.ok) {
      printError("parsing");
      continue;
    }
    node_t *syntax_tree = parsing.value;

    result_reduce_t reduction = reduce(syntax_tree);
    if (!reduction.ok) {
      printError("reduction");
      continue;
    }
    node_t *reduced = reduction.value;

    int buffer_offset = 0;
    print(reduced, BUFFER_SIZE, buffer, &buffer_offset);

    printf("~> %s\n", buffer);

    memset(buffer, 0, BUFFER_SIZE);
    nodeDealloc(&reduced);
    nodeDealloc(&syntax_tree);
    listDealloc(&tokens);
  }
}
