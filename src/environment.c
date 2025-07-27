#include "environment.h"
#include "arena.h"
#include "map.h"
#include "value.h"

result_builtin_t sum(value_t *result, value_list_t *values) {
  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = 0;

  for (size_t i = 0; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    // TODO: this should raise an error instead of silently skipping non-numbers
    if (current.type == VALUE_TYPE_INTEGER) {
      result->value.integer += current.value.integer;
    }
  }

  return (result_builtin_t){.ok = true};
}

result_alloc_t environmentCreate(arena_t *arena, environment_t *parent) {
  environment_t *environment = nullptr;
  try(result_alloc_t, arenaAllocate(arena, sizeof(environment_t)), environment);
  environment->parent = parent;
  environment->arena = arena;

  // TODO: the number of values per environnment is very arbitrary because it's
  //   fixed for now. The hashmap should grow instead
  try(result_alloc_t, mapCreate(value_t, arena, 32), environment->values);

#define setBuiltin(Label, Builtin)                                             \
  {                                                                            \
    try(result_alloc_t, arenaAllocate(arena, sizeof(value_t)), builtin);       \
    builtin->type = VALUE_TYPE_BUILTIN;                                        \
    builtin->value.builtin = Builtin;                                          \
    mapSet(environment->values, (Label), builtin);                             \
  }

  value_t *builtin = nullptr;
  setBuiltin("+", sum);
#undef setBuiltin

  return ok(result_alloc_t, environment);
}

value_t *environmentResolveSymbol(environment_t *self, const char *symbol) {
  const auto result = mapGet(value_t, self->values, symbol);
  if (!result && self->parent) {
    return environmentResolveSymbol(self->parent, symbol);
  }
  return result;
}
