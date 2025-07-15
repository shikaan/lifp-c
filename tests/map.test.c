#include "../src/map.h"
#include "../src/arena.h"
#include "test.h"
#include <assert.h>
#include <string.h>

void create() {
  result_alloc_t arena_result = arenaCreate(1024);
  expectTrue(arena_result.ok, "arena creation succeeds");
  arena_t *arena = arena_result.value;

  result_alloc_t map_result = mapCreate(int, arena, 8);
  expectTrue(map_result.ok, "map allocation succeeds");

  generic_map_t *map = map_result.value;
  expectEqlSize(map->count, 0, "initial count is 0");
  expectEqlSize(map->capacity, 8, "capacity is set correctly");
  expectEqlSize(map->item_size, sizeof(int), "item size is set correctly");

  arenaDestroy(arena);
}

void getSet() {
  case("new item");
  result_alloc_t arena_creation = arenaCreate(1024);
  assert(arena_creation.ok);
  arena_t *arena = arena_creation.value;

  result_alloc_t map_creation = mapCreate(int, arena, 8);
  assert(map_creation.ok);
  generic_map_t *map = map_creation.value;

  int value = 42;
  result_alloc_t setting = mapSet(map, "test", &value);
  expectTrue(setting.ok, "sets successfully");
  expectEqlSize(map->count, 1, "count increases");
  int* item = mapGet(map, "test");
  expectEqlInt(*item, value, "value is correct");
  arenaReset(arena);
  
  case("update");
  map_creation = mapCreate(int, arena, 8);
  assert(map_creation.ok);
  map = map_creation.value;

  int value2 = 43;
  
  setting = mapSet(map, "test", &value);
  assert(setting.ok);
  expectEqlSize(map->count, 1, "count is 1 after first set");
  item = mapGet(map, "test");
  expectEqlInt(*item, value, "value is correct");
  
  setting = mapSet(map, "test", &value2);
  expectTrue(setting.ok, "updates the record");
  expectEqlSize(map->count, 1, "count remains 1 after update");
  item = mapGet(map, "test");
  expectEqlInt(*item, value2, "retrieves the updated element");
  arenaReset(arena);

  case("collision");
  map_creation = mapCreate(int, arena, 8);
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
  item = mapGet(map, key1);
  int* another_item = mapGet(map, key2);
  expectNeqInt(*item, *another_item, "values don't overlap");
  expectEqlInt(*another_item, value2, "retrieves correct first value");
  expectEqlInt(*item, value, "retrieves correct other value");

  arenaDestroy(arena);
}

int main() {
  suite(create);
  suite(getSet);

  return report();
}
