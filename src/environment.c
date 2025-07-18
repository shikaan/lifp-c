#include "environment.h"

constexpr size_t BUILTINS_COUNT = 1;
result_builtin_t sum(node_t *result, node_list_t *nodes) {
  result->type = NODE_TYPE_INTEGER;
  result->value.integer = 0;

  for (size_t i = 0; i < nodes->count; i++) {
    node_t current = listGet(node_t, nodes, i);
    // TODO: this should raise an error instead of silently skipping non-numbers
    if (current.type == NODE_TYPE_INTEGER) {
      result->value.integer += current.value.integer;
    }
  }

  return (result_builtin_t){.ok = true};
}

result_alloc_t environmentCreate(arena_t *arena, environment_t *parent) {
  auto allocation = arenaAllocate(arena, sizeof(environment_t));
  if (!allocation.ok)
    return allocation;
  environment_t *environment = allocation.value;
  environment->parent = parent;

  allocation = mapCreate(builtin_t, arena, BUILTINS_COUNT);
  if (!allocation.ok)
    return allocation;

  environment->builtins = allocation.value;

// Utility inline macro to not forget to put the function pointer into a stack
// variable so that we can extract the pointer to the function pointer before
// storing it on the map
#define setBuiltin(Label, Builtin)                                             \
  {                                                                            \
    builtin = Builtin;                                                         \
    mapSet(environment->builtins, Label, (void *)&builtin);                    \
  }

  builtin_t builtin;
  setBuiltin("+", sum);
#undef setBuiltin

  return ok(result_alloc_t, environment);
}
