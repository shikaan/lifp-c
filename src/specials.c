#include "environment.h"
#include "evaluate.h"
#include "list.h"
#include "node.h"
#include "result.h"

typedef ResultVoid() result_special_t;

/**
  (def! x (+ 2 3))
*/
result_special_t define(environment_t *env, const node_list_t *nodes) {
  if (nodes->count != 3) {
    error_t error = {.kind = ERROR_KIND_ALLOCATION}; // TODO: fix type
    return error(result_special_t, error);
  }

  node_t key = listGet(node_t, nodes, 1);
  if (key.type != NODE_TYPE_SYMBOL) {
    error_t error = {.kind = ERROR_KIND_ALLOCATION}; // TODO: fix type
    return error(result_special_t, error);
  }

  node_t value = listGet(node_t, nodes, 2);
  result_reduce_t reduction = reduce(env->arena, &value, env);
  if (!reduction.ok) {
    return error(result_special_t, reduction.error);
  }

  mapSet(env->values, key.value.symbol, reduction.value);

  return (result_special_t){.ok = true};
}
