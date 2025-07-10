#include "parser.h"
#include "alloc.h"
#include "result.h"
#include "token.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

// Make clang see unreachable
// TODO: under which conditions can this be removed?
#define __need_unreachable 1

static const size_t NODE_LIST_STRIDE = 16;
static result_alloc_t nodeListAlloc() {
  result_alloc_t node_allocation = allocSafe(sizeof(node_t));
  if (!node_allocation.ok) {
    return node_allocation;
  }

  node_t *node = node_allocation.value;
  result_alloc_t list_allocation = allocSafe(sizeof(node_t) * NODE_LIST_STRIDE);

  if (!list_allocation.ok) {
    deallocSafe(node);
    return list_allocation;
  }

  node_t *list = list_allocation.value;
  node->value.list.count = 0;
  node->value.list.items = list;

  return node_allocation;
}

static result_alloc_t nodeAtomAlloc() { return allocSafe(sizeof(node_t)); }
static void nodeDealloc(node_t *ptr) { deallocSafe(ptr); }

result_node_t parseAtom(token_t token) {
  assert(token.type == TOKEN_TYPE_INTEGER || token.type == TOKEN_TYPE_SYMBOL);
  result_alloc_t node_result = nodeAtomAlloc();

  if (!node_result.ok) {
    return error(result_node_t, node_result.error.kind,
                 node_result.error.payload);
  }

  node_t *node = node_result.value;
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
    memcpy(node->value.symbol, token.value.symbol, SYMBOL_SIZE);
    return ok(result_node_t, node);
  case TOKEN_TYPE_LPAREN:
  case TOKEN_TYPE_RPAREN:
  default:
    nodeDealloc(node);
    unreachable();
  }
}

result_node_t parseList(token_list_t *tokens, size_t *offset, size_t *depth) {
  result_alloc_t node_result = nodeListAlloc();
  if (!node_result.ok) {
    return error(result_node_t, node_result.error.kind,
                 node_result.error.payload);
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

    // Expand the array by NODE_LIST_STRIDE spots if needed
    if (node->value.list.count % NODE_LIST_STRIDE == 0) {
      auto realloc_result = reallocSafe(
          node->value.list.items,
          (node->value.list.count + NODE_LIST_STRIDE) * (sizeof(node_t)));
      if (!realloc_result.ok) {
        nodeDealloc(node);
        return error(result_node_t, realloc_result.error.kind,
                     realloc_result.error.payload);
      }
      node->value.list.items = realloc_result.value;
    }

    result_node_t sub_node_result = parse(tokens, offset, depth);
    if (!sub_node_result.ok) {
      nodeDealloc(node);
      return sub_node_result;
    }

    memcpy(&node->value.list.items[node->value.list.count],
           sub_node_result.value, sizeof(node_t));
    node->value.list.count++;
  }

  return ok(result_node_t, node);
}

result_node_t parse(token_list_t *tokens, size_t *offset, size_t *depth) {
  if (tokens->capacity == 0) {
    exception_payload_t payload = {.invalid_expression = nullptr};
    return error(result_node_t, EXCEPTION_INVALID_EXPRESSION, payload);
  }

  const token_t first_token = tokens->data[*offset];
  size_t initial_depth = *depth;

  if (first_token.type == TOKEN_TYPE_LPAREN) {
    result_node_t parse_list_result = parseList(tokens, offset, depth);

    // There are left parens that don't match right parens
    if (*depth != initial_depth) {
      exception_payload_t payload = {.unbalanced_parentheses = nullptr};
      return error(result_node_t, EXCEPTION_UNBALANCED_PARENTHESES, payload);
    }

    // There are dangling chars after top level list
    if (initial_depth == 0 && *offset != (tokens->capacity - 1)) {
      exception_payload_t payload = {.invalid_expression = nullptr};
      return error(result_node_t, EXCEPTION_INVALID_EXPRESSION, payload);
    }

    return parse_list_result;
  }

  return parseAtom(first_token);
}
