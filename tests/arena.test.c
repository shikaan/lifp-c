#include "../src/arena.h"
#include "test.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

void basic() {
  result_alloc_t creation = arenaCreate(1024);
  expectTrue(creation.ok, "succeeds arena creation");

  arena_t *arena = creation.value;
  expectNotNull(arena, "arena is not null");

  result_alloc_t allocation = arenaAllocate(arena, 100);
  expectTrue(allocation.ok, "succeeds arena creation");
  expectNotNull(allocation.value, "allocated memory is not be null");

  case("reset");
  arenaReset(arena);
  
  result_alloc_t reallocation = arenaAllocate(arena, 1024);
  expectTrue(reallocation.ok, "can reallocate memory after a reset");
  
  arenaDestroy(arena);
}

void overflow() {
  result_alloc_t creation = arenaCreate(100);
  expectTrue(creation.ok, "succeeds arena creation");
  
  arena_t *arena = creation.value;
  
  result_alloc_t allocation = arenaAllocate(arena, 200);
  expectFalse(allocation.ok, "fails oversized allocation");
  expectEqlUint(allocation.error.kind, ERROR_KIND_ALLOCATION, "throws correct exception");
  
  arenaDestroy(arena);
}

void alignment() {
  result_alloc_t creation = arenaCreate(1024);
  assert(creation.ok);
  arena_t *arena = creation.value;

  for (size_t i = 0; i < 16; i++) {
    result_alloc_t allocation = arenaAllocate(arena, i);
    assert(allocation.ok);
    uintptr_t pointer = (uintptr_t)allocation.value;
    expectEqlUint(pointer % 8, 0, "address is 8-aligned");
  }
}

int main() {
  suite(basic);
  suite(overflow);
  suite(alignment);
  return report();
}
