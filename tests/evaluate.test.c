#include "../src/evaluate.h"
#include "../src/arena.h"
#include "../src/environment.h"
#include "../src/list.h"
#include "../src/node.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;
static environment_t *environment;

#define tryAssert(Action, Destination)                                         \
  {                                                                            \
    auto result__LINE__ = (Action);                                            \
    assert(result__LINE__.ok);                                                 \
    (Destination) = (result__LINE__.value);                                    \
  }
#define tryAssertVoid(Action)                                                  \
  {                                                                            \
    auto result__LINE__ = (Action);                                            \
    assert(result__LINE__.ok);                                                 \
  }

void expectEqlValueType(value_type_t actual, value_type_t expected,
                        const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected node type %d to equal %d", (int)actual,
           (int)expected);
  expect(actual == expected, name, msg);
}

void atoms() {
  value_t *result = nullptr;

  case("integer");
  node_t integer_node = nInt(42);
  tryAssert(evaluate(test_arena, &integer_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_INTEGER,
                     "has correct type");
  expectEqlInt(result->value.integer, 42, "has correct value");

  case("boolean");
  node_t bool_node = nBool(true);
  tryAssert(evaluate(test_arena, &bool_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_BOOLEAN,
                     "has correct type");
  expectTrue(result->value.boolean, "has correct value");

  case("nil");
  node_t nil_node = nNil();
  tryAssert(evaluate(test_arena, &nil_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_NIL,
                     "has correct type");

  case("symbol");
  value_t *symbol = nullptr;
  tryAssert(valueCreate(test_arena, VALUE_TYPE_INTEGER), symbol);
  mapSet(environment->values, "value", symbol);

  node_t symbol_node = nSym("value");
  tryAssert(evaluate(test_arena, &symbol_node, environment), result);
  expectEqlValueType(result->type, VALUE_TYPE_INTEGER,
                     "has correct type");
}

void listOfElements() {
  result_ref_t allocation = listCreate(node_t, test_arena, 4);
  assert(allocation.ok);
  node_list_t *expected = allocation.value;

  node_t first = nInt(42);
  node_t second = nInt(123);
  allocation = listAppend(node_t, expected, &first);
  assert(allocation.ok);
  allocation = listAppend(node_t, expected, &second);
  assert(allocation.ok);

  node_t list_node = nList(2, expected->data);

  result_value_ref_t reduction = evaluate(test_arena, &list_node, environment);
  assert(reduction.ok);
  expectEqlValueType(reduction.value->type, VALUE_TYPE_LIST,
                     "has correct type");
  value_list_t reduced_list = reduction.value->value.list;
  expectEqlSize(reduced_list.count, 2, "has correct count");
  for (size_t i = 0; i < reduced_list.count; i++) {
    value_t node = listGet(value_t, &reduced_list, i);
    node_t expected_node = listGet(node_t, expected, i);
    expectEqlValueType(node.type, VALUE_TYPE_INTEGER, "has correct type");
    expectEqlInt(node.value.integer, expected_node.value.integer,
                 "has correct value");
  }
}

void functionCall() {
  result_ref_t allocation = listCreate(node_t, test_arena, 4);
  expectTrue(allocation.ok, "list allocation succeeds");
  node_list_t *list = allocation.value;

  node_t symbol = nSym("+");
  node_t num1 = nInt(1);
  node_t num2 = nInt(2);
  node_t num3 = nInt(3);
  allocation = listAppend(node_t, list, &symbol);
  assert(allocation.ok);
  allocation = listAppend(node_t, list, &num1);
  assert(allocation.ok);
  allocation = listAppend(node_t, list, &num2);
  assert(allocation.ok);
  allocation = listAppend(node_t, list, &num3);
  assert(allocation.ok);

  node_t form_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;

  value_t *result = nullptr;
  tryAssert(evaluate(test_arena, &form_node, environment), result);
  expectNotNull(result, "reduced result is not null");
  expectEqlInt(result->value.integer, 6, "has correct result");
  
  value_t val = { .type = VALUE_TYPE_INTEGER, .value.integer = 1};
  mapSet(environment->values, "lol", &val);
  node_t lol_symbol = nSym("lol");
  
  tryAssert(listCreate(node_t, test_arena, 4), list);
  tryAssertVoid(listAppend(node_t, list, &lol_symbol))
  tryAssertVoid(listAppend(node_t, list, &num1))
  node_t list_node = nList(4, list->data);
  form_node.value.list.capacity = list->capacity;
  tryAssert(evaluate(test_arena, &list_node, environment), result);
  expectEqlUint(result->type, VALUE_TYPE_LIST, "does't invoke if symbol is not lambda");
}

void nested() {
  result_ref_t allocation = listCreate(node_t, test_arena, 4);
  assert(allocation.ok);
  node_list_t *inner_list = allocation.value;

  node_t inner1 = nInt(1);
  node_t inner2 = nInt(2);
  allocation = listAppend(node_t, inner_list, &inner1);
  assert(allocation.ok);
  allocation = listAppend(node_t, inner_list, &inner2);
  assert(allocation.ok);

  node_t inner_list_node = nList(2, inner_list->data);
  inner_list_node.value.list.capacity = inner_list->capacity;

  // Create outer list: (3 (1 2))
  allocation = listCreate(node_t, test_arena, 4);
  assert(allocation.ok);
  node_list_t *outer_list = allocation.value;

  node_t outer1 = nInt(3);
  allocation = listAppend(node_t, outer_list, &outer1);
  assert(allocation.ok);
  allocation = listAppend(node_t, outer_list, &inner_list_node);
  assert(allocation.ok);

  node_t outer_list_node = nList(2, outer_list->data);
  outer_list_node.value.list.capacity = outer_list->capacity;

  result_value_ref_t reduction =
      evaluate(test_arena, &outer_list_node, environment);
  assert(reduction.ok);
  expectEqlValueType(reduction.value->type, VALUE_TYPE_LIST,
                     "has correct type");
  expectEqlSize(reduction.value->value.list.count, 2, "has correct count");
  value_t first = listGet(value_t, &reduction.value->value.list, 0);
  value_t second = listGet(value_t, &reduction.value->value.list, 1);
  expectEqlValueType(first.type, VALUE_TYPE_INTEGER, "has correct type");
  expectEqlValueType(second.type, VALUE_TYPE_LIST, "has correct type");
}

void emptyList() {
  result_ref_t allocation = listCreate(node_t, test_arena, 4); // capacity > 0
  assert(allocation.ok);
  node_list_t *empty_list = allocation.value;

  node_t empty_list_node = nList(0, empty_list->data);
  empty_list_node.value.list.capacity = empty_list->capacity;

  result_value_ref_t result = evaluate(test_arena, &empty_list_node, environment);
  assert(result.ok);
  expectEqlValueType(result.value->type, VALUE_TYPE_LIST, "has correct type");
  expectEqlSize(result.value->value.list.count, 0, "has correct count");
}

void allocations() {
  result_ref_t arena_result = arenaCreate(32);
  assert(arena_result.ok);
  arena_t *small_arena = arena_result.value;

  arenaAllocate(small_arena, 31); // Use up most space
  node_t large_node = nInt(123);
  result_value_ref_t reduction = evaluate(small_arena, &large_node, environment);
  assert(!reduction.ok);
  expectEqlUint(reduction.error.kind, ERROR_KIND_ALLOCATION,
                "returns allocation error");

  arenaDestroy(small_arena);
}

void errors() {
  result_ref_t allocation = listCreate(node_t, test_arena, 1);
  assert(allocation.ok);
  node_list_t *list = allocation.value;

  case("non-existing symbol");
  node_t sym = nSym("not-existent");
  result_ref_t appending = listAppend(node_t, list, &sym);
  assert(appending.ok);

  result_value_ref_t reduction = evaluate(test_arena, &sym, environment);
  expectFalse(reduction.ok, "fails reduction");
  expectEqlUint(reduction.error.kind, ERROR_KIND_SYMBOL_NOT_FOUND, "with correct symbol");
}

void specialForms() { 
  result_ref_t allocation = listCreate(node_t, test_arena, 1);
  assert(allocation.ok);
  node_list_t *list = allocation.value;

  node_t special = nSym("def!");
  node_t var = nSym("foo");
  node_t value = nInt(1);

  result_ref_t appending = listAppend(node_t, list, &special);
  assert(appending.ok);
  appending = listAppend(node_t, list, &var);
  assert(appending.ok);
  appending = listAppend(node_t, list, &value);
  assert(appending.ok);

  node_t list_node = nList(3, list->data);
  list_node.value.list.arena = test_arena;

  result_value_ref_t reduction = evaluate(test_arena, &list_node, environment);
  assert(reduction.ok); 
  value_t* val = mapGet(value_t, environment->values, "foo");

  expectNotNull(val, "environment is updated");
  expectEqlInt(val->value.integer, 1, "with correct value");
}

int main(void) {
  result_ref_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.ok);
  test_arena = allocation.value;

  allocation = environmentCreate(test_arena, nullptr);
  assert(allocation.ok);
  environment = allocation.value;

  suite(atoms);
  suite(listOfElements);
  suite(functionCall);
  suite(nested);
  suite(emptyList);
  suite(allocations);
  suite(errors);
  suite(specialForms);

  arenaDestroy(test_arena);
  return report();
}
