#include "lexer.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "arena.h"
#include "list.h"
#include "position.h"
#include "result.h"

typedef Result(token_t) result_token_t;

result_token_t parseAtomBuffer(size_t buffer_len,
                               char buffer[static buffer_len],
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

  token_value_t value = {};
  strncpy(value.symbol, buffer, buffer_len);

  const token_t tok = {
      .type = TOKEN_TYPE_SYMBOL,
      .value = value,
      .position = position,
  };
  return ok(result_token_t, tok);
}

result_token_list_t tokenize(arena_t *arena, const char *source) {
  position_t position = {.column = 0, .line = 1};

  result_alloc_t allocation = listCreate(token_t, arena, 32);
  if (!allocation.ok) {
    return error(result_token_list_t, allocation.error);
  }
  token_list_t *tokens = allocation.value;

  constexpr size_t BUFFER_CAPACITY = 64;
  char atom_buffer[BUFFER_CAPACITY] = {0};
  size_t atom_buffer_len = 0;
  result_alloc_t append;

  for (int i = 0; source[i] != '\0'; i++) {
    position.column++;
    const char current_char = source[i];

    if (current_char == LPAREN) {
      token_t tok = {.type = TOKEN_TYPE_LPAREN,
                     .value.lparen = nullptr,
                     .position = position};
      append = listAppend(node_t, tokens, &tok);
    } else if (current_char == RPAREN) {
      if (atom_buffer_len > 0) {
        result_token_t parsing =
            parseAtomBuffer(atom_buffer_len, atom_buffer, position);
        if (!parsing.ok) {
          return error(result_token_list_t, parsing.error);
        }
        append = listAppend(token_t, tokens, &parsing.value);
        // clean buffer
        atom_buffer_len = 0;
      }

      const token_t tok = {.type = TOKEN_TYPE_RPAREN,
                           .value = {.rparen = nullptr},
                           .position = position};
      append = listAppend(node_t, tokens, &tok);
    } else if (isspace(current_char)) {
      if (current_char == '\n') {
        position.line++;
        position.column = 0;
      }

      if (atom_buffer_len == 0)
        continue;

      result_token_t parsing =
          parseAtomBuffer(atom_buffer_len, atom_buffer, position);
      if (!parsing.ok) {
        return error(result_token_list_t, parsing.error);
      }
      append = listAppend(token_t, tokens, &parsing.value);
      // clean buffer
      atom_buffer_len = 0;
    } else if (isprint(current_char)) {
      if (atom_buffer_len > BUFFER_CAPACITY) {
        error_t exception = {
            .kind = ERROR_KIND_UNEXPECTED_TOKEN,
            .payload.unexpected_token.position = position,
            .payload.unexpected_token.token = current_char,
        };
        return error(result_token_list_t, exception);
      }

      atom_buffer[atom_buffer_len++] = current_char;
      continue;
    } else {
      error_t exception = {
          .kind = ERROR_KIND_UNEXPECTED_TOKEN,
          .payload.unexpected_token.position = position,
          .payload.unexpected_token.token = current_char,
      };
      return error(result_token_list_t, exception);
    }

    if (!append.ok) {
      return error(result_token_list_t, allocation.error);
    }
  }

  if (atom_buffer_len > 0) {
    result_token_t parsing =
        parseAtomBuffer(atom_buffer_len, atom_buffer, position);
    if (!parsing.ok) {
      return error(result_token_list_t, parsing.error);
    }
    append = listAppend(token_t, tokens, &parsing.value);
    if (!append.ok) {
      return error(result_token_list_t, allocation.error);
    }
  }

  return ok(result_token_list_t, tokens);
}
