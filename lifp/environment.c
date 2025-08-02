#include "environment.h"

// NOLINTBEGIN
#include "std/core.c"
#include "std/flow.c"
#include "std/list.c"
#include "std/math.c"
// NOLINTEND

#include "value.h"

result_ref_t environmentCreate(arena_t *arena, environment_t *parent) {
  environment_t *environment = nullptr;
  tryAssign(result_ref_t, arenaAllocate(arena, sizeof(environment_t)),
            environment);
  environment->parent = parent;
  environment->arena = arena;

  tryAssign(result_ref_t, mapCreate(value_t, arena, 32), environment->values);

#define setBuiltin(Label, Builtin)                                             \
  {                                                                            \
    builtin.type = VALUE_TYPE_BUILTIN;                                         \
    builtin.value.builtin = (Builtin);                                         \
    try(result_ref_t, mapSet(environment->values, (Label), &builtin));         \
  }

  value_t builtin;
  setBuiltin(SUM, sum);
  setBuiltin(SUB, subtract);
  setBuiltin(MUL, multiply);
  setBuiltin(DIV, divide);
  setBuiltin(MOD, modulo);
  setBuiltin(EQUAL, equal);
  setBuiltin(LESS_THAN, lessThan);
  setBuiltin(GREATER_THAN, greaterThan);
  setBuiltin(NEQ, notEqual);
  setBuiltin(LEQ, lessEqual);
  setBuiltin(GEQ, greaterEqual);
  setBuiltin(LOGICAL_AND, logicalAnd);
  setBuiltin(LOGICAL_OR, logicalOr);
  setBuiltin(FLOW_SLEEP, flowSleep);
  setBuiltin(LIST_COUNT, listCount);
  setBuiltin(LIST_FROM, listFrom);
  setBuiltin(LIST_NTH, listNth);
  setBuiltin(MATH_MAX, mathMax);
  setBuiltin(MATH_MIN, mathMin);
  setBuiltin(MATH_RANDOM, mathRandom);
#undef setBuiltin

  return ok(result_ref_t, environment);
}

value_t *environmentResolveSymbol(environment_t *self, const char *symbol) {
  const auto result = mapGet(value_t, self->values, symbol);
  if (!result && self->parent) {
    return environmentResolveSymbol(self->parent, symbol);
  }
  return result;
}
