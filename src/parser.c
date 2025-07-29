#include "parser.h"
#include "arena.h"
#include "lexer.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

result_node_ref_t parseAtom(arena_t *arena, token_t token) {
  assert(token.type == TOKEN_TYPE_INTEGER || token.type == TOKEN_TYPE_SYMBOL);

  node_t *node = nullptr;
  tryAssign(result_node_ref_t, nodeCreate(arena, NODE_TYPE_INTEGER), node);

  node->position = token.position;

  switch (token.type) {
  case TOKEN_TYPE_INTEGER:
    node->type = NODE_TYPE_INTEGER;
    node->value.integer = token.value.integer;
    return ok(result_node_ref_t, node);
  case TOKEN_TYPE_SYMBOL:
    if (strncmp(token.value.symbol, TRUE, 4) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = true;
      return ok(result_node_ref_t, node);
    }

    if (strncmp(token.value.symbol, FALSE, 5) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = false;
      return ok(result_node_ref_t, node);
    }

    if (strncmp(token.value.symbol, NIL, 3) == 0) {
      node->type = NODE_TYPE_NIL;
      node->value.nil = nullptr;
      return ok(result_node_ref_t, node);
    }

    node->type = NODE_TYPE_SYMBOL;
    bytewiseCopy(node->value.symbol, token.value.symbol, SYMBOL_SIZE);
    return ok(result_node_ref_t, node);
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
  default:
    unreachable();
  }
}

result_node_ref_t parseList(arena_t *arena, const token_list_t *tokens,
                            size_t *offset, size_t *depth) {
  node_t *node = nullptr;
  tryAssign(result_node_ref_t, nodeCreate(arena, NODE_TYPE_LIST), node);

  token_t token = listGet(token_t, tokens, *offset);
  node->position = token.position;
  node->type = NODE_TYPE_LIST;
  (*depth)++;
  (*offset)++;

  for (; *offset < tokens->capacity; (*offset)++) {
    const token_t tok = listGet(token_t, tokens, *offset);
    if (tok.type == TOKEN_TYPE_RPAREN) {
      (*depth)--;
      break;
    }

    node_t *sub_node = nullptr;
    tryAssign(result_node_ref_t, parse(arena, tokens, offset, depth), sub_node);

    tryVoid(result_node_ref_t, listAppend(node_t, &node->value.list, sub_node));
  }

  return ok(result_node_ref_t, node);
}

result_node_ref_t parse(arena_t *arena, const token_list_t *tokens,
                        size_t *offset, size_t *depth) {
  if (tokens->count == 0) {
    error_t exception = {.kind = ERROR_KIND_INVALID_EXPRESSION};
    return error(result_node_ref_t, exception);
  }

  const token_t first_token = listGet(token_t, tokens, *offset);
  size_t initial_depth = *depth;

  if (first_token.type == TOKEN_TYPE_LPAREN) {
    result_node_ref_t parse_list_result =
        parseList(arena, tokens, offset, depth);

    // There are left parens that don't match right parens
    if (*depth != initial_depth) {
      error_t exception = {.kind = ERROR_KIND_UNBALANCED_PARENTHESES};
      return error(result_node_ref_t, exception);
    }

    // There are dangling chars after top level list
    if (initial_depth == 0 && *offset != (tokens->count - 1)) {
      error_t exception = {.kind = ERROR_KIND_INVALID_EXPRESSION};
      return error(result_node_ref_t, exception);
    }

    return parse_list_result;
  }

  return parseAtom(arena, first_token);
}
