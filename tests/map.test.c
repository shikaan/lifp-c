#include "../src/map.h"
#include "../src/arena.h"
#include "test.h"
#include <assert.h>
#include <string.h>

typedef void (*fun_t)(void);
void testFunction(void) {}

static arena_t *test_arena;

void create() {
  result_ref_t creation = mapCreate(int, test_arena, 8);
  expectTrue(creation.ok, "map allocation succeeds");
  generic_map_t *map = creation.value;

  expectEqlSize(map->count, 0, "initial count is 0");
  expectEqlSize(map->capacity, 8, "capacity is set correctly");
  expectEqlSize(map->item_size, sizeof(int), "item size is set correctly");
  arenaReset(test_arena);
}

void getSet() {
  result_ref_t map_creation = mapCreate(int, test_arena, 8);
  assert(map_creation.ok);
  generic_map_t *map = map_creation.value;

  case("empty map");
  expectNull(mapGet(int, map, "key"), "retrieves null from empty map");
    
  case("new item");
  int value = 42;
  result_ref_t setting = mapSet(map, "test", &value);
  expectTrue(setting.ok, "sets successfully");
  expectEqlSize(map->count, 1, "count increases");
  int* item = mapGet(int, map, "test");
  expectEqlInt(*item, value, "value is correct");
  arenaReset(test_arena);
  
  case("update");
  map_creation = mapCreate(int, test_arena, 8);
  assert(map_creation.ok);
  map = map_creation.value;

  int value2 = 43;

  setting = mapSet(map, "test", &value);
  assert(setting.ok);
  expectEqlSize(map->count, 1, "count is 1 after first set");
  item = mapGet(int, map, "test");
  expectEqlInt(*item, value, "value is correct");
  
  setting = mapSet(map, "test", &value2);
  expectTrue(setting.ok, "updates the record");
  expectEqlSize(map->count, 1, "count remains 1 after update");
  item = mapGet(int, map, "test");
  expectEqlInt(*item, value2, "retrieves the updated element");
  arenaReset(test_arena);

  case("collision");
  map_creation = mapCreate(int, test_arena, 8);
  assert(map_creation.ok);
  map = map_creation.value;

  // These two strings are known to clash in FNV-1
  const char* key1 = "liquid";
  const char* key2 = "costarring";

  setting = mapSet(map, key1, &value);
  assert(setting.ok);

  setting = mapSet(map, key2, &value2);
  assert(setting.ok);

  expectEqlSize(map->count, 2, "increases count");
  item = mapGet(int, map, key1);
  int* another_item = mapGet(int, map, key2);
  expectNeqInt(*item, *another_item, "values don't overlap");
  expectEqlInt(*item, value, "retrieves correct value");
  expectEqlInt(*another_item, value2, "retrieves correct other value");

  map_creation = mapCreate(int, test_arena, 1);
  assert(map_creation.ok);
  map = map_creation.value;

  // setting with key1 and trying to retrieve with colliding key2
  setting = mapSet(map, key1, &value);
  assert(setting.ok);
  expectNull(mapGet(int, map, key2), "doesn't return item from colliding key");
  
  arenaReset(test_arena);
}

void allocations() {
  result_ref_t allocation = mapCreate(int, test_arena, 1);
  assert(allocation.ok);
  Map(int) *map = allocation.value;

  int value = 1;
  allocation = mapSet(map, "key", &value);
  assert(allocation.ok);
  
  allocation = mapSet(map, "another", &value);
  expectFalse(allocation.ok, "cannot allocate over capacity");
  expectEqlUint(allocation.error.kind, ERROR_KIND_ALLOCATION, "returns correct error");
  arenaReset(test_arena);
}

int main() {
  result_ref_t creation = arenaCreate(1024);
  assert(creation.ok);
  test_arena = creation.value;

  suite(create);
  suite(getSet);
  suite(allocations);

  arenaDestroy(test_arena);
  return report();
}
