#include "parser.h"
#include "arena.h"
#include "lexer.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

// Make clang see unreachable
// TODO: under which conditions can this be removed?
#define __need_unreachable 1

result_node_t parseAtom(arena_t *arena, token_t token) {
  assert(token.type == TOKEN_TYPE_INTEGER || token.type == TOKEN_TYPE_SYMBOL);
  result_alloc_t allocation = nodeAlloc(arena, NODE_TYPE_INTEGER);

  if (!allocation.ok) {
    return error(result_node_t, allocation.error);
  }

  node_t *node = allocation.value;
  node->position = token.position;

  switch (token.type) {
  case TOKEN_TYPE_INTEGER:
    node->type = NODE_TYPE_INTEGER;
    node->value.integer = token.value.integer;
    return ok(result_node_t, node);
  case TOKEN_TYPE_SYMBOL:
    if (strncmp(token.value.symbol, TRUE, 4) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = true;
      return ok(result_node_t, node);
    }

    if (strncmp(token.value.symbol, FALSE, 5) == 0) {
      node->type = NODE_TYPE_BOOLEAN;
      node->value.boolean = false;
      return ok(result_node_t, node);
    }

    if (strncmp(token.value.symbol, NIL, 3) == 0) {
      node->type = NODE_TYPE_NIL;
      node->value.nil = nullptr;
      return ok(result_node_t, node);
    }

    node->type = NODE_TYPE_SYMBOL;
    bytewiseCopy(node->value.symbol, token.value.symbol, SYMBOL_SIZE);
    return ok(result_node_t, node);
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
  default:
    unreachable();
  }
}

result_node_t parseList(arena_t *arena, const token_list_t *tokens,
                        size_t *offset, size_t *depth) {
  result_alloc_t node_result = nodeAlloc(arena, NODE_TYPE_LIST);
  if (!node_result.ok) {
    return error(result_node_t, node_result.error);
  }

  node_t *node = node_result.value;
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

    result_node_t sub_node_result = parse(arena, tokens, offset, depth);
    if (!sub_node_result.ok) {
      return sub_node_result;
    }

    node_t *sub_node = sub_node_result.value;

    result_alloc_t appending = listAppend(node_t, &node->value.list, sub_node);
    if (!appending.ok) {
      return error(result_node_t, appending.error);
    }
  }

  return ok(result_node_t, node);
}

result_node_t parse(arena_t *arena, const token_list_t *tokens, size_t *offset,
                    size_t *depth) {
  if (tokens->count == 0) {
    error_t exception = {.kind = ERROR_KIND_INVALID_EXPRESSION};
    return error(result_node_t, exception);
  }

  const token_t first_token = listGet(token_t, tokens, *offset);
  size_t initial_depth = *depth;

  if (first_token.type == TOKEN_TYPE_LPAREN) {
    result_node_t parse_list_result = parseList(arena, tokens, offset, depth);

    // There are left parens that don't match right parens
    if (*depth != initial_depth) {
      error_t exception = {.kind = ERROR_KIND_UNBALANCED_PARENTHESES};
      return error(result_node_t, exception);
    }

    // There are dangling chars after top level list
    if (initial_depth == 0 && *offset != (tokens->count - 1)) {
      error_t exception = {.kind = ERROR_KIND_INVALID_EXPRESSION};
      return error(result_node_t, exception);
    }

    return parse_list_result;
  }

  return parseAtom(arena, first_token);
}
