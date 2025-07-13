#include "../src/arena.h"
#include "test.h"

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
  expectEqlUint(allocation.error.kind, EXCEPTION_KIND_ALLOCATION, "throws correct exception");
  
  arenaDestroy(arena);
}

int main() {
    suite(basic);
    suite(overflow);
    return report();
}
