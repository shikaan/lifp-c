#include "../src/list.h"
#include "./test.h"
#include <assert.h>
#include <stddef.h>

typedef List(int) int_list_t;
static arena_t *test_arena;

int main(void) {
  result_alloc_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.ok);
  test_arena = allocation.value;

  allocation = listAlloc(int, test_arena, 1);
  assert(allocation.ok);
  int_list_t *list = allocation.value;

  case("empty");
  expectEqlSize(list->count, 0, "is empty");
  expectEqlSize(list->capacity, 1, "has correct capacity");
  
  case("after push");
  int item = 12;
  allocation = listAppend(int, list, &item);
  assert(allocation.ok);
  expectEqlSize(list->count, 1, "is not empty");
  expectEqlSize(list->capacity, 1, "has same capacity");
  expectEqlInt(listGet(int, list, list->count - 1), item, "has correct item");
  
  case("with resize");
  int item_2 = 2;
  allocation = listAppend(int, list, &item_2);
  assert(allocation.ok);
  expectEqlSize(list->count, 2, "has correct count");
  expectEqlSize(list->capacity, 2, "has updated capacity");
  
  case("with another resize");
  for (size_t i = 0; i < 4; i++) {
    allocation = listAppend(int, list, &item_2);
    assert(allocation.ok);
  }
  expectEqlSize(list->capacity, 8, "has updated capacity again");

  arenaDestroy(test_arena);
  return report();
}
