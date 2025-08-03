#include "evaluate.h"
#include "environment.h"
#include "error.h"
#include "node.h"
#include "value.h"

// NOLINTNEXTLINE
#include "specials.c"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static bool isSpecialFormNode(const node_t FIRST_NODE) {
  if (FIRST_NODE.type != NODE_TYPE_SYMBOL)
    return false;

  size_t len = strlen(FIRST_NODE.value.symbol);
  return ((strncmp(FIRST_NODE.value.symbol, DEFINE, 4) == 0 && len == 4) ||
          (strncmp(FIRST_NODE.value.symbol, FUNCTION, 2) == 0 && len == 2) ||
          (strncmp(FIRST_NODE.value.symbol, LET, 3) == 0 && len == 3) != 0 ||
          (strncmp(FIRST_NODE.value.symbol, COND, 4) == 0 && len == 4)) != 0;
}

static result_value_ref_t invokeBuiltin(value_t *result, value_t builtin_value,
                                        arena_t *arena) {
  assert(builtin_value.type == VALUE_TYPE_BUILTIN);
  builtin_t builtin = builtin_value.value.builtin;

  // Prepare argument list with all values except for the symbol
  value_list_t *arguments = nullptr;
  tryAssign(result_value_ref_t,
            listCreate(value_t, arena, result->value.list.count - 1), arguments,
            builtin_value.position);

  for (size_t i = 1; i < result->value.list.count; i++) {
    value_t argument = listGet(value_t, &result->value.list, i);
    try(result_value_ref_t, listAppend(value_t, arguments, &argument));
  }

  try(result_value_ref_t, builtin(result, arguments), builtin_value.position);
  return ok(result_value_ref_t, result);
}

static result_value_ref_t invokeClosure(value_t *result, value_t closure_value,
                                        arena_t *arena,
                                        environment_t *parent_environment) {
  assert(closure_value.type == VALUE_TYPE_CLOSURE);
  closure_t closure = closure_value.value.closure;

  if (result->value.list.count - 1 != closure.arguments.count) {
    throwMeta(result_value_ref_t, ERROR_CODE_TYPE_UNEXPECTED_ARITY,
              closure_value.position,
              "Unexpected arity. Expected %lu arguments, got %lu.",
              result->value.list.count - 1, closure.arguments.count);
  }

  environment_t *environment = nullptr;
  tryAssign(result_value_ref_t, environmentCreate(parent_environment),
            environment, closure_value.position);

  // Populate the closure with the values, skipping the closure symbol
  for (size_t i = 1; i < result->value.list.count; i++) {
    auto argument = listGet(node_t, &closure.arguments, i - 1);
    auto value = listGet(value_t, &result->value.list, i);
    mapSet(environment->values, argument.value.symbol, &value);
  }

  value_t *reduced = nullptr;
  tryAssign(result_value_ref_t, evaluate(arena, &closure.form, environment),
            reduced, closure_value.position);
  environmentDestroy(&environment);
  return ok(result_value_ref_t, reduced);
}

static result_value_ref_t invokeSpecialForm(value_t *result, node_t form_node,
                                            const node_list_t *nodes,
                                            environment_t *environment) {
  if (form_node.value.symbol[0] == 'd') {
    tryAssign(result_value_ref_t, define(environment, nodes), result,
              form_node.position);
  } else if (form_node.value.symbol[0] == 'f') {
    tryAssign(result_value_ref_t, function(environment, nodes), result,
              form_node.position);
  } else if (form_node.value.symbol[0] == 'l') {
    tryAssign(result_value_ref_t, let(environment, nodes), result,
              form_node.position);
  } else {
    tryAssign(result_value_ref_t, cond(environment, nodes), result,
              form_node.position);
  }

  return ok(result_value_ref_t, result);
}

result_value_ref_t evaluateList(arena_t *arena, node_t *syntax_tree,
                                environment_t *environment) {
  const auto list = syntax_tree->value.list;

  value_t *result = nullptr;
  tryAssign(result_value_ref_t, valueCreate(arena, VALUE_TYPE_LIST), result,
            syntax_tree->position);
  result->position.column = syntax_tree->position.column;
  result->position.line = syntax_tree->position.line;

  if (list.count == 0) {
    return ok(result_value_ref_t, result);
  }

  auto first_node = listGet(node_t, &list, 0);
  if (isSpecialFormNode(first_node)) {
    return invokeSpecialForm(result, first_node, &list, environment);
  }

  for (size_t i = 0; i < list.count; i++) {
    auto node = listGet(node_t, &list, i);
    value_t *reduced = nullptr;
    tryAssign(result_value_ref_t, evaluate(arena, &node, environment), reduced,
              node.position);
    try(result_value_ref_t, listAppend(value_t, &result->value.list, reduced),
        syntax_tree->position);
  }

  value_t first_value = listGet(value_t, &result->value.list, 0);

  if (first_value.type == VALUE_TYPE_BUILTIN) {
    return invokeBuiltin(result, first_value, arena);
  }

  if (first_value.type == VALUE_TYPE_CLOSURE) {
    return invokeClosure(result, first_value, arena, environment);
  }

  return ok(result_value_ref_t, result);
}

result_value_ref_t evaluate(arena_t *arena, node_t *syntax_tree,
                            environment_t *environment) {
  value_t *value = nullptr;
  tryAssign(result_value_ref_t, valueCreate(arena, VALUE_TYPE_INTEGER), value,
            syntax_tree->position);
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
      throwMeta(result_value_ref_t, ERROR_CODE_REFERENCE_SYMBOL_NOT_FOUND,
                syntax_tree->position,
                "Symbol '%s' cannot be found in the current environment",
                syntax_tree->value.symbol);
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
  return ok(result_value_ref_t, value);
}
