#include "evaluate.h"
#include "arena.h"
#include "environment.h"
#include "list.h"
#include "map.h"
#include "node.h"
#include "result.h"
#include "value.h"

// NOLINTNEXTLINE
#include "specials.c"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Make clang see unreachable
// TODO: under which conditions can this be removed?
#define __need_unreachable 1

static bool isSpecialFormNode(const node_t FIRST_NODE) {
  if (FIRST_NODE.type != NODE_TYPE_SYMBOL)
    return false;

  return ((strncmp(FIRST_NODE.value.symbol, "def!", 4) == 0) ||
          (strncmp(FIRST_NODE.value.symbol, "fn*", 3) == 0)) != 0;
}

static result_valuep_t invokeBuiltin(value_t *result, value_t builtin_value,
                                     arena_t *arena) {
  assert(builtin_value.type == VALUE_TYPE_BUILTIN);
  builtin_t builtin = builtin_value.value.builtin;

  // Prepare argument list with all values except for the symbol
  value_list_t *arguments = nullptr;
  try(result_valuep_t, listCreate(value_t, arena, result->value.list.count - 1),
      arguments);

  for (size_t i = 1; i < result->value.list.count; i++) {
    value_t argument = listGet(value_t, &result->value.list, i);
    tryVoid(result_valuep_t, listAppend(value_t, arguments, &argument));
  }

  tryVoid(result_valuep_t, builtin(result, arguments));
  return ok(result_valuep_t, result);
}

static result_valuep_t invokeClosure(value_t *result, value_t closure_value,
                                     arena_t *arena,
                                     environment_t *parent_environment) {
  assert(closure_value.type == VALUE_TYPE_CLOSURE);
  closure_t closure = closure_value.value.closure;

  if (result->value.list.count - 1 != closure.arguments.count) {
    error_t error = {
        .kind = ERROR_KIND_UNEXPECTED_ARITY,
        .payload.unexpected_arity.actual = result->value.list.count - 1,
        .payload.unexpected_arity.expected = closure.arguments.count,
        .position = closure_value.position,
        .example = nullptr};
    return error(result_valuep_t, error);
  }

  environment_t *environment = nullptr;
  try(result_valuep_t,
      environmentCreate(parent_environment->arena, parent_environment),
      environment);

  // Populate the closure with the values, skipping the closure symbol
  for (size_t i = 1; i < result->value.list.count; i++) {
    auto argument = listGet(node_t, &closure.arguments, i - 1);
    auto value = listGet(value_t, &result->value.list, i);
    mapSet(environment->values, argument.value.symbol, &value);
  }

  value_t *reduced = nullptr;
  try(result_valuep_t, evaluate(arena, &closure.form, environment), reduced);
  return ok(result_valuep_t, reduced);
}

result_valuep_t evaluateList(arena_t *arena, node_t *syntax_tree,
                             environment_t *environment) {
  const auto list = syntax_tree->value.list;

  value_t *result = nullptr;
  try(result_valuep_t, valueCreate(arena, VALUE_TYPE_LIST), result);
  result->position.column = syntax_tree->position.column;
  result->position.line = syntax_tree->position.line;

  if (list.count == 0) {
    return ok(result_valuep_t, result);
  }

  auto first_node = listGet(node_t, &list, 0);
  if (isSpecialFormNode(first_node)) {
    if (first_node.value.symbol[0] == 'd') {
      try(result_valuep_t, define(environment, &list), result);
    } else {
      try(result_valuep_t, function(environment, &list), result);
    }

    return ok(result_valuep_t, result);
  }

  for (size_t i = 0; i < list.count; i++) {
    auto node = listGet(node_t, &list, i);
    value_t *reduced = nullptr;
    try(result_valuep_t, evaluate(arena, &node, environment), reduced);
    tryVoid(result_valuep_t, listAppend(value_t, &result->value.list, reduced));
  }

  value_t first_value = listGet(value_t, &result->value.list, 0);

  if (first_value.type == VALUE_TYPE_BUILTIN) {
    return invokeBuiltin(result, first_value, arena);
  }

  if (first_value.type == VALUE_TYPE_CLOSURE) {
    return invokeClosure(result, first_value, arena, environment);
  }

  return ok(result_valuep_t, result);
}

result_valuep_t evaluate(arena_t *arena, node_t *syntax_tree,
                         environment_t *environment) {
  value_t *value = nullptr;
  try(result_valuep_t, valueCreate(arena, VALUE_TYPE_INTEGER), value);
  value->position.column = syntax_tree->position.column;
  value->position.line = syntax_tree->position.line;

  switch (syntax_tree->type) {
  case NODE_TYPE_BOOLEAN: {
    value->type = VALUE_TYPE_BOOLEAN;
    value->value.boolean = syntax_tree->value.boolean;
    break;
  }
  case NODE_TYPE_NIL: {
    value->type = VALUE_TYPE_NIL;
    value->value.nil = nullptr;
    break;
  }
  case NODE_TYPE_INTEGER: {
    value->type = VALUE_TYPE_INTEGER;
    value->value.integer = syntax_tree->value.integer;
    break;
  }
  case NODE_TYPE_SYMBOL: {
    value_t *resolved_value =
        environmentResolveSymbol(environment, syntax_tree->value.symbol);

    if (!resolved_value) {
      error_t error = {.kind = ERROR_KIND_SYMBOL_NOT_FOUND,
                       .position = syntax_tree->position,
                       .payload.symbol_not_found = syntax_tree->value.symbol};
      return error(result_valuep_t, error);
    }

    value->type = resolved_value->type;
    value->value = resolved_value->value;
    break;
  }
  case NODE_TYPE_LIST:
    return evaluateList(arena, syntax_tree, environment);
  default:
    unreachable();
  }
  return ok(result_valuep_t, value);
}
