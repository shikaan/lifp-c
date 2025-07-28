#include "environment.h"
#include "evaluate.h"
#include "list.h"
#include "node.h"
#include "result.h"
#include "value.h"
#include <assert.h>
#include <string.h>

typedef Result(value_t *) result_special_t;
typedef result_special_t (*special_form_t)(environment_t *,
                                           const node_list_t *);

const char *DEFINE_EXAMPLE = "(def! x (+ 1 2))";
result_special_t define(environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // def! is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .position = first.position,
                     .example = DEFINE_EXAMPLE};
    return error(result_special_t, error);
  }

  node_t key = listGet(node_t, nodes, 1);
  if (key.type != NODE_TYPE_SYMBOL) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)key.type,
                     .payload.unexpected_type.expected = NODE_TYPE_SYMBOL,
                     .position = key.position,
                     .example = DEFINE_EXAMPLE};
    return error(result_special_t, error);
  }

  // Perform reduction in the AST memory
  value_t *result = nullptr;
  node_t value = listGet(node_t, nodes, 2);
  try(result_special_t, evaluate(nodes->arena, &value, env), result);

  // If reduction is successful, we can move the closure to VM memory
  value_t *copy = nullptr;
  try(result_special_t, valueClone(env->arena, result), copy);

  mapSet(env->values, key.value.symbol, copy);

  value_t nil = {
      .type = VALUE_TYPE_NIL,
      .value.nil = nullptr,
      .position = first.position,
  };
  return ok(result_special_t, &nil);
}

const char *FUNCTION_EXAMPLE = "(fn* (a b) (+ a b))";
result_special_t function(environment_t *env, const node_list_t *nodes) {
  assert(nodes->count > 0); // fn* is always there
  node_t first = listGet(node_t, nodes, 0);
  if (nodes->count != 3) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .position = first.position,
                     .example = FUNCTION_EXAMPLE};
    return error(result_special_t, error);
  }

  node_t arguments = listGet(node_t, nodes, 1);
  if (arguments.type != NODE_TYPE_LIST) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)arguments.type,
                     .payload.unexpected_type.expected = NODE_TYPE_LIST,
                     .position = arguments.position,
                     .example = FUNCTION_EXAMPLE};
    return error(result_special_t, error);
  }

  node_t form = listGet(node_t, nodes, 2);

  environment_t *child_environment = nullptr;
  try(result_special_t, environmentCreate(env->arena, env), child_environment);

  value_t *closure = nullptr;
  try(result_special_t, valueCreate(nodes->arena, VALUE_TYPE_CLOSURE), closure);

  closure->position.column = first.position.column;
  closure->position.line = first.position.line;

  tryVoid(result_special_t, listCopy(node_t, &arguments.value.list,
                                     &closure->value.closure.arguments));
  tryVoid(result_special_t, nodeCopy(&form, &closure->value.closure.form));

  return ok(result_special_t, closure);
}
