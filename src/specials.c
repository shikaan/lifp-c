#include "environment.h"
#include "evaluate.h"
#include "list.h"
#include "map.h"
#include "node.h"
#include "result.h"
#include "value.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef result_value_ref_t (*special_form_t)(environment_t *,
                                             const node_list_t *);

const char *DEFINE_EXAMPLE = "(def! x (+ 1 2))";
const char *DEFINE = "def!";
result_value_ref_t define(environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // def! is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .position = first.position,
                     .example = DEFINE_EXAMPLE};
    return error(result_value_ref_t, error);
  }

  node_t key = listGet(node_t, nodes, 1);
  if (key.type != NODE_TYPE_SYMBOL) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)key.type,
                     .payload.unexpected_type.expected = NODE_TYPE_SYMBOL,
                     .position = key.position,
                     .example = DEFINE_EXAMPLE};
    return error(result_value_ref_t, error);
  }

  // Perform reduction in the AST memory
  value_t *result = nullptr;
  node_t value = listGet(node_t, nodes, 2);
  tryAssign(result_value_ref_t, evaluate(nodes->arena, &value, env), result);

  // If reduction is successful, we can move the closure to VM memory
  value_t *copy = nullptr;
  tryAssign(result_value_ref_t, valueClone(env->arena, result), copy);

  mapSet(env->values, key.value.symbol, copy);

  value_t nil = {
      .type = VALUE_TYPE_NIL,
      .value.nil = nullptr,
      .position = first.position,
  };
  return ok(result_value_ref_t, &nil);
}

const char *FUNCTION_EXAMPLE = "(fn (a b) (+ a b))";
const char *FUNCTION = "fn";
result_value_ref_t function(environment_t *env, const node_list_t *nodes) {
  (void)env;
  assert(nodes->count > 0); // fn* is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .position = first.position,
                     .example = FUNCTION_EXAMPLE};
    return error(result_value_ref_t, error);
  }

  node_t arguments = listGet(node_t, nodes, 1);
  if (arguments.type != NODE_TYPE_LIST) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)arguments.type,
                     .payload.unexpected_type.expected = NODE_TYPE_LIST,
                     .position = arguments.position,
                     .example = FUNCTION_EXAMPLE};
    return error(result_value_ref_t, error);
  }

  node_t form = listGet(node_t, nodes, 2);

  value_t *closure = nullptr;
  tryAssign(result_value_ref_t, valueCreate(nodes->arena, VALUE_TYPE_CLOSURE),
            closure);

  closure->position.column = first.position.column;
  closure->position.line = first.position.line;

  try(result_value_ref_t, listCopy(node_t, &arguments.value.list,
                                   &closure->value.closure.arguments));
  try(result_value_ref_t, nodeCopy(&form, &closure->value.closure.form));

  return ok(result_value_ref_t, closure);
}

const char *LET_EXAMPLE = "(let ((a 1) (b 2)) (+ a b))";
const char *LET = "let";
result_value_ref_t let(environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // let is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .position = first.position,
                     .example = LET_EXAMPLE};
    return error(result_value_ref_t, error);
  }

  node_t couples = listGet(node_t, nodes, 1);
  if (couples.type != NODE_TYPE_LIST) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)couples.type,
                     .payload.unexpected_type.expected = NODE_TYPE_LIST,
                     .position = couples.position,
                     .example = LET_EXAMPLE};
    return error(result_value_ref_t, error);
  }

  environment_t *local_env = nullptr;
  tryAssign(result_value_ref_t, environmentCreate(env->arena, env), local_env);

  for (size_t i = 0; i < couples.value.list.count; i++) {
    node_t couple = listGet(node_t, &couples.value.list, i);

    if (couple.type != NODE_TYPE_LIST || couple.value.list.count != 2) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)couple.type,
                       .payload.unexpected_type.expected = NODE_TYPE_LIST,
                       .position = couple.position,
                       .example = LET_EXAMPLE};
      return error(result_value_ref_t, error);
    }

    node_t symbol = listGet(node_t, &couple.value.list, 0);
    if (symbol.type != NODE_TYPE_SYMBOL) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)couple.type,
                       .payload.unexpected_type.expected = NODE_TYPE_SYMBOL,
                       .position = symbol.position,
                       .example = LET_EXAMPLE};
      return error(result_value_ref_t, error);
    }

    node_t body = listGet(node_t, &couple.value.list, 1);
    value_t *evaluated = nullptr;
    tryAssign(result_value_ref_t, evaluate(nodes->arena, &body, local_env),
              evaluated);
    try(result_value_ref_t,
        mapSet(local_env->values, symbol.value.symbol, evaluated));
  }

  node_t form = listGet(node_t, nodes, 2);

  value_t *result = nullptr;
  tryAssign(result_value_ref_t, evaluate(nodes->arena, &form, local_env),
            result);

  return ok(result_value_ref_t, result);
}

const char *COND_EXAMPLE = "(cond\n\t((!= x 0) (/ 10 x))\n\t(+ x 10))";
const char *COND = "cond";
result_value_ref_t cond(environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0);
  value_t *result = nullptr;

  for (size_t i = 1; i < nodes->count - 1; i++) {
    node_t node = listGet(node_t, nodes, i);
    if (node.type != NODE_TYPE_LIST || node.value.list.count != 2) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)node.type,
                       .payload.unexpected_type.expected = NODE_TYPE_LIST,
                       .position = node.position,
                       .example = COND_EXAMPLE};
      return error(result_value_ref_t, error);
    }

    node_t condition = listGet(node_t, &node.value.list, 0);
    tryAssign(result_value_ref_t, evaluate(nodes->arena, &condition, env),
              result);

    if (result->type != VALUE_TYPE_BOOLEAN) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)condition.type,
                       .payload.unexpected_type.expected = NODE_TYPE_LIST,
                       .position = condition.position,
                       .example = COND_EXAMPLE};
      return error(result_value_ref_t, error);
    }

    if (result->value.boolean) {
      node_t form = listGet(node_t, &node.value.list, 1);
      tryAssign(result_value_ref_t, evaluate(nodes->arena, &form, env), result);
      return ok(result_value_ref_t, result);
    }
  }

  node_t fallback = listGet(node_t, nodes, nodes->count - 1);
  tryAssign(result_value_ref_t, evaluate(nodes->arena, &fallback, env), result);
  return ok(result_value_ref_t, result);
}
