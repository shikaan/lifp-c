#include "../src/list.h"
#include "./test.h"
#include <assert.h>
#include <stddef.h>

typedef List(int) int_list_t;

int main(void) {
  auto allocation = listAlloc(int, 1);
  assert(allocation.ok);
  int_list_t *list = allocation.value;

  case("empty");
  expectEqlSize(list->count, 0, "is empty");
  expectEqlSize(list->capacity, 1, "has correct capacity");

  case("after push");
  int item = 12;
  listAppend(list, &item);
  expectEqlSize(list->count, 1, "is not empty");
  expectEqlSize(list->capacity, 1, "has same capacity");
  expectEqlInt(list->data[list->count - 1], item, "has correct item");

  case("with resize");
  int item_2 = 2;
  listAppend(list, &item_2);
  expectEqlSize(list->count, 2, "has correct count");
  expectEqlSize(list->capacity, LIST_STRIDE + 1, "has updated capacity");

  case("with another resize");
  for (size_t i = 0; i < LIST_STRIDE; i++) {
    listAppend(list, &item_2);
  }
  expectEqlSize(list->capacity, (LIST_STRIDE*2) + 1, "has updated capacity again");

  listDealloc(&list);

  return report();
}
