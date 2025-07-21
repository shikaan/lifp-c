#include "environment.h"
#include "arena.h"
#include "map.h"
#include "value.h"

result_lambda_t sum(value_t *result, value_list_t *values) {
  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = 0;

  for (size_t i = 0; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    // TODO: this should raise an error instead of silently skipping non-numbers
    if (current.type == VALUE_TYPE_INTEGER) {
      result->value.integer += current.value.integer;
    }
  }

  return (result_lambda_t){.ok = true};
}

result_alloc_t environmentCreate(arena_t *arena, environment_t *parent) {
  auto allocation = arenaAllocate(arena, sizeof(environment_t));
  if (!allocation.ok)
    return allocation;
  environment_t *environment = allocation.value;
  environment->parent = parent;

  // TODO: the number of values per environnment is very arbitrary because it's
  // fixed for now. The hashmap should grow instead
  allocation = mapCreate(value_t, arena, 32);
  if (!allocation.ok)
    return allocation;
  environment->values = allocation.value;

  // TODO: as arbitrarily as above, I am putting a stub value until we develop
  // special forms
  allocation = arenaAllocate(arena, sizeof(value_t));
  if (!allocation.ok)
    return allocation;
  value_t *test_value = allocation.value;
  test_value->value.integer = 42;
  test_value->type = VALUE_TYPE_INTEGER;
  mapSet(environment->values, "VERSION", (void *)test_value);

#define setBuiltin(Label, Builtin)                                             \
  {                                                                            \
    allocation = arenaAllocate(arena, sizeof(value_t));                        \
    if (!allocation.ok)                                                        \
      return allocation;                                                       \
    builtin = allocation.value;                                                \
    builtin->type = VALUE_TYPE_FUNCTION;                                       \
    builtin->value.function = Builtin;                                         \
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
