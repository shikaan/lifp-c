#include "parser.h"
#include "alloc.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

// Make clang see unreachable
// TODO: under which conditions can this be removed?
#define __need_unreachable 1

result_node_t parseAtom(token_t token) {
  assert(token.type == TOKEN_TYPE_INTEGER || token.type == TOKEN_TYPE_SYMBOL);
  result_alloc_t allocation = nodeAlloc(
      NODE_TYPE_INTEGER); // FIXME: this sucks, as we don't need integers here

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
    nodeDealloc(&node);
    unreachable();
  }
}

result_node_t parseList(const token_list_t *tokens, size_t *offset,
                        size_t *depth) {
  result_alloc_t node_result = nodeAlloc(NODE_TYPE_LIST);
  if (!node_result.ok) {
    return error(result_node_t, node_result.error);
  }

  node_t *node = node_result.value;
  node->position = tokens->data[*offset].position;
  node->type = NODE_TYPE_LIST;
  (*depth)++;
  (*offset)++;

  for (; *offset < tokens->capacity; (*offset)++) {
    const token_t tok = tokens->data[*offset];
    if (tok.type == TOKEN_TYPE_RPAREN) {
      (*depth)--;
      break;
    }

    result_node_t sub_node_result = parse(tokens, offset, depth);
    if (!sub_node_result.ok) {
      nodeDealloc(&node);
      return sub_node_result;
    }

    node_t *sub_node = sub_node_result.value;

    result_alloc_t appending = listAppend(&node->value.list, sub_node);
    // nodeDealloc(&sub_node); this cannot be freed because it would yield a use
    // after free, since listAppend copies the values, but doesn't deep copy in
    // the case of lists
    if (!appending.ok) {
      nodeDealloc(&node);
      return error(result_node_t, appending.error);
    }
  }

  return ok(result_node_t, node);
}

result_node_t parse(const token_list_t *tokens, size_t *offset, size_t *depth) {
  if (tokens->capacity == 0) {
    exception_t exception = {.kind = EXCEPTION_INVALID_EXPRESSION};
    return error(result_node_t, exception);
  }

  const token_t first_token = tokens->data[*offset];
  size_t initial_depth = *depth;

  if (first_token.type == TOKEN_TYPE_LPAREN) {
    result_node_t parse_list_result = parseList(tokens, offset, depth);

    // There are left parens that don't match right parens
    if (*depth != initial_depth) {
      exception_t exception = {.kind = EXCEPTION_UNBALANCED_PARENTHESES};
      return error(result_node_t, exception);
    }

    // There are dangling chars after top level list
    if (initial_depth == 0 && *offset != (tokens->count - 1)) {
      exception_t exception = {.kind = EXCEPTION_INVALID_EXPRESSION};
      return error(result_node_t, exception);
    }

    return parse_list_result;
  }

  return parseAtom(first_token);
}
