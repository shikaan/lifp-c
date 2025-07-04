#include "./src/lexer.h"
#include <stdio.h>

int main(void) {
  Token token = {
    .type = TOKEN_TYPE_lparen,
    .value = {
      .lparen = nullptr,
    }
  };

  printf("Token type: %u\n", token.type);
 return 0;
}
