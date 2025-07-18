#include "../src/environment.h"
#include "../src/arena.h"
#include "../src/evaluate.h"
#include "../src/list.h"
#include "../src/node.h"
#include "test.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;
static environment_t *environment;

void expectEqlNodeType(node_type_t actual, node_type_t expected,
                       const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected node type %d to equal %d", (int)actual,
           (int)expected);
  expect(actual == expected, name, msg);
}

void atoms() {
  node_t integer_node = nInt(42);
  result_reduce_t reduction = reduce(test_arena, &integer_node, environment);
  assert(reduction.ok);
  expectEqlNodeType(reduction.value->type, NODE_TYPE_INTEGER,
                    "reduced integer has correct type");
  expectEqlInt(reduction.value->value.integer, 42,
               "reduced integer has correct value");

  node_t bool_node = nBool(true);
  reduction = reduce(test_arena, &bool_node, environment);
  assert(reduction.ok);
  expectEqlNodeType(reduction.value->type, NODE_TYPE_BOOLEAN,
                    "reduced boolean has correct type");
  expectTrue(reduction.value->value.boolean,
             "reduced boolean has correct value");

  node_t nil_node = nNil();
  reduction = reduce(test_arena, &nil_node, environment);
  assert(reduction.ok);
  expectEqlNodeType(reduction.value->type, NODE_TYPE_NIL,
                    "reduced nil has correct type");

  node_t symbol_node = nSym("test");
  reduction = reduce(test_arena, &symbol_node, environment);
  assert(reduction.ok);
  expectEqlNodeType(reduction.value->type, NODE_TYPE_SYMBOL,
                    "reduced symbol has correct type");
  expectEqlString(reduction.value->value.symbol, "test", SYMBOL_SIZE,
                  "reduced symbol has correct value");
}

void listOfElements() {
  result_alloc_t allocation = listCreate(node_t, test_arena, 4);
  assert(allocation.ok);
  node_list_t *expected = allocation.value;

  node_t first = nInt(42);
  node_t second = nInt(123);
  allocation = listAppend(node_t, expected, &first);
  assert(allocation.ok);
  allocation = listAppend(node_t, expected, &second);
  assert(allocation.ok);

  node_t list_node = nList(2, expected->data);

  result_reduce_t reduction = reduce(test_arena, &list_node, environment);
  assert(reduction.ok);
  expectEqlNodeType(reduction.value->type, NODE_TYPE_LIST, "has correct type");
  node_list_t reduced_list = reduction.value->value.list;
  expectEqlSize(reduced_list.count, 2, "has correct count");
  for (size_t i = 0; i < reduced_list.count; i++) {
    node_t node = listGet(node_t, &reduced_list, i);
    node_t expected_node = listGet(node_t, expected, i);
    expectEqlNodeType(node.type, NODE_TYPE_INTEGER, "has correct type");
    expectEqlInt(node.value.integer, expected_node.value.integer,
                 "has correct value");
  }
}

void functionCall() {
  result_alloc_t allocation = listCreate(node_t, test_arena, 4);
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

  node_t list_node = nList(4, list->data);
  list_node.value.list.capacity = list->capacity;

  result_reduce_t reduction = reduce(test_arena, &list_node, environment);
  assert(reduction.ok);
  expectNotNull(reduction.value, "reduced result is not null");
  expectEqlInt(reduction.value->value.integer, 6, "has correct result");
}

void nested() {
  result_alloc_t allocation = listCreate(node_t, test_arena, 4);
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

  result_reduce_t reduction = reduce(test_arena, &outer_list_node, environment);
  assert(reduction.ok);
  expectEqlNodeType(reduction.value->type, NODE_TYPE_LIST, "has correct type");
  expectEqlSize(reduction.value->value.list.count, 2, "has correct count");
  node_t first = listGet(node_t, &reduction.value->value.list, 0);
  node_t second = listGet(node_t, &reduction.value->value.list, 1);
  expectEqlNodeType(first.type, NODE_TYPE_INTEGER, "has correct type");
  expectEqlNodeType(second.type, NODE_TYPE_LIST, "has correct type");
}

void emptyList() {
  result_alloc_t allocation = listCreate(node_t, test_arena, 4); // capacity > 0
  assert(allocation.ok);
  node_list_t *empty_list = allocation.value;

  node_t empty_list_node = nList(0, empty_list->data);
  empty_list_node.value.list.capacity = empty_list->capacity;

  result_reduce_t result = reduce(test_arena, &empty_list_node, environment);
  assert(result.ok);
  expectEqlNodeType(result.value->type, NODE_TYPE_LIST, "has correct type");
  expectEqlSize(result.value->value.list.count, 0, "has correct count");
}

void allocations() {
  result_alloc_t arena_result = arenaCreate(32);
  assert(arena_result.ok);
  arena_t *small_arena = arena_result.value;

  arenaAllocate(small_arena, 31); // Use up most space
  node_t large_node = nInt(123);
  result_reduce_t reduction = reduce(small_arena, &large_node, environment);
  assert(!reduction.ok);
  expectEqlUint(reduction.error.kind, EXCEPTION_KIND_ALLOCATION,
                "returns allocation error");

  arenaDestroy(small_arena);
}

int main(void) {
  result_alloc_t allocation = arenaCreate((size_t)(1024 * 1024));
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

  arenaDestroy(test_arena);
  return report();
}
