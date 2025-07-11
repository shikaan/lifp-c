
#include "src/alloc.h"
#include "src/debug.h"
#include "src/evaluate.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/print.h"
#include <readline/readline.h> //TODO this is not platform dependent
#include <stddef.h>
#include <stdio.h>

constexpr size_t BUFFER_SIZE = 4096;

void printError(const char *msg) {
  printf("!! Error: %s\n", msg); // TODO: create print error function
}

int main(void) {
  bool exit = false;
  char buffer[BUFFER_SIZE];
  while (!exit) {
    char *input = readline("> ");
    result_token_list_t tokens_result = tokenize(input);
    if (!tokens_result.ok) {
      printError("tokenization");
      continue;
    }

    size_t offset = 0;
    size_t depth = 0;
    result_node_t nodes_result = parse(tokens_result.value, &offset, &depth);
    if (!nodes_result.ok) {
      debugu(nodes_result.error.kind);
      printError("parsing");
      continue;
    }

    result_reduce_t reduce_result = reduce(nodes_result.value);
    if (!reduce_result.ok) {
      printError("reduction");
      continue;
    }

    int buffer_offset = 0;
    print(reduce_result.value, BUFFER_SIZE, buffer, &buffer_offset);

    printf("~> %s\n", buffer);
    buffer[0] = 0;
    // TODO: breaks when deallocating an atom
    deallocSafe(reduce_result.value);
    // TODO: breaks when deallocating a list
    listDealloc(nodes_result.value);
    listDealloc(tokens_result.value);
  }
}
