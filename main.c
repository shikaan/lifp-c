#include "./src/lexer.h"
#include <stdio.h>

int main(void) {
  const ResultTokenList result = tokenize("hello world");
	printf("%c\n", result.error.payload.unexpected_token);
 	return 1;
}
