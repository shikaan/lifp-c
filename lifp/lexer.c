#include "lexer.h"
#include "error.h"
#include "position.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef Result(token_t, position_t) result_token_t;
result_token_t bufferToToken(size_t buffer_len, char buffer[static buffer_len],
                             position_t position) {
  buffer[buffer_len] = 0;
  char *remainder;
  // FIXME: this is silently overflowing
  int32_t number = (int32_t)strtol(buffer, &remainder, 10);

  // This condition is met when all the chars of the token represent an integer
  // This includes also leading +/-
  const bool is_number = ((!remainder) || (strlen(remainder) == 0)) != 0;
  if (is_number) {
    const token_t tok = {.type = TOKEN_TYPE_INTEGER,
                         .value.integer = number,
                         .position = position};
    return ok(result_token_t, tok);
  }

  if (buffer_len >= SYMBOL_SIZE) {
    throwMeta(result_token_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN, position,
              "Token too long. Expected length <= %lu, got %lu", SYMBOL_SIZE,
              buffer_len);
  }

  // else, it's just a symbol
  token_t tok = {.type = TOKEN_TYPE_SYMBOL, .position = position};
  memcpy(&tok.value.symbol, buffer, buffer_len);
  tok.value.symbol[buffer_len] = 0;
  return ok(result_token_t, tok);
}

result_token_list_ref_t tokenize(arena_t *arena, const char *source) {
  position_t cursor = {.column = 0, .line = 1};

  token_list_t *tokens = nullptr;
  tryAssign(result_token_list_ref_t, listCreate(token_t, arena, 32), tokens,
            cursor);

  constexpr size_t BUFFER_CAPACITY = 64;
  char buffer[BUFFER_CAPACITY] = {0};
  size_t buffer_len = 0;
  token_t token;
  position_t position = {};

  for (int i = 0; source[i] != '\0'; i++) {
    cursor.column++;
    const char current_char = source[i];

    if (current_char == LPAREN) {
      token.type = TOKEN_TYPE_LPAREN;
      token.value.lparen = nullptr;
      token.position = cursor;
      try(result_token_list_ref_t, listAppend(token_t, tokens, &token),
          position);
    } else if (current_char == RPAREN) {
      if (buffer_len > 0) {
        tryAssign(result_token_list_ref_t,
                  bufferToToken(buffer_len, buffer, position), token, position);
        try(result_token_list_ref_t, listAppend(token_t, tokens, &token),
            position);
        // clean buffer
        buffer_len = 0;
      }

      token.type = TOKEN_TYPE_RPAREN;
      token.value.rparen = nullptr;
      token.position = cursor;
      try(result_token_list_ref_t, listAppend(token_t, tokens, &token),
          position);
    } else if (isspace(current_char)) {
      if (current_char == '\n') {
        cursor.line++;
        cursor.column = 0;
      }

      if (buffer_len == 0)
        continue;

      tryAssign(result_token_list_ref_t,
                bufferToToken(buffer_len, buffer, position), token, position);
      try(result_token_list_ref_t, listAppend(token_t, tokens, &token),
          position);
      // clean buffer
      buffer_len = 0;
    } else if (isprint(current_char)) {
      if (buffer_len == 0) {
        position.line = cursor.line;
        position.column = cursor.column;
      }

      if (buffer_len >= BUFFER_CAPACITY) {
        throwMeta(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                  position, "Token too long. Expected length <= %lu, got %lu",
                  SYMBOL_SIZE, buffer_len);
      }

      buffer[buffer_len++] = current_char;
      continue;
    } else {
      throwMeta(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                cursor, "Unexpected token '%c'", current_char);
    }
  }

  if (buffer_len > 0) {
    if (buffer_len >= BUFFER_CAPACITY) {
      throwMeta(result_token_list_ref_t, ERROR_CODE_SYNTAX_UNEXPECTED_TOKEN,
                position, "Token too long. Expected length <= %lu, got %lu",
                SYMBOL_SIZE, buffer_len);
    }
    tryAssign(result_token_list_ref_t,
              bufferToToken(buffer_len, buffer, position), token, position);
    try(result_token_list_ref_t, listAppend(token_t, tokens, &token), position);
  }

  return ok(result_token_list_ref_t, tokens);
}
