#include "../src/stack.h"
#include "./test.h"
#include <assert.h>
#include <stddef.h>

typedef Stack(int) int_stack_t;

int main(void) {
  auto allocation = stackAlloc(int, 1);
  assert(allocation.ok);
  int_stack_t *stack = allocation.value;

  case("empty");
  expectEqlSize(stack->count, 0, "is empty");
  expectEqlSize(stack->capacity, 1, "has correct capacity");

  case("after push");
  int item = 12;
  stackPush(stack, &item);
  expectEqlSize(stack->count, 1, "is not empty");
  expectEqlSize(stack->capacity, 1, "has same capacity");
  expectEqlInt(stack->data[stack->capacity -1], item, "has correct item");

  case("with resize");
  int item_2 = 2;
  stackPush(stack, &item_2);
  expectEqlSize(stack->count, 2, "has correct count");
  expectEqlSize(stack->capacity, STACK_STRIDE + 1, "has updated capacity");

  case("with another resize");
  for (size_t i = 0; i < STACK_STRIDE; i++) {
    stackPush(stack, &item_2);
  }
  
  expectEqlSize(stack->capacity, (STACK_STRIDE*2) + 1, "has updated capacity again");

  stackDealloc(stack);

  return report();
}
