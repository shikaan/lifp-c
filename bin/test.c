#include "../lib/arena.h"
#include "../lifp/environment.h"
#include <stdio.h>

int main(void) {
  result_ref_t allocation = arenaCreate((size_t)1024 * 16);
  if (!allocation.ok) {
    printf("arena allocation error\n");
    return 1;
  }
  arena_t *arena = allocation.value;

  allocation = environmentCreate(arena, nullptr);
  if (!allocation.ok) {
    printf("environment error\n");
    printf("%u\n", allocation.error.kind);

    arenaDestroy(arena);
    return 1;
  }
  environment_t *env = allocation.value;

  printf("enviroment size: %lu\n", sizeof(environment_t));
  printf("map size: %lu\n", mapSize(env->values));
  printf("arena used: %lu\n", arena->offset);

  return 0;
}
