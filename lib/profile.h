#pragma once

#include "arena.h"
#include "result.h"
#include <stddef.h>

constexpr size_t MAX_SUBSPAN = 16;

typedef struct span_t {
  char label[16];
  long hits;
  long total;
  long last;
  long subspans_count;
  unsigned long span_id;
  arena_t *arena;
  struct span_t *parent;
  struct span_t *subspans[MAX_SUBSPAN];
} span_t;

result_void_t metricsInit(arena_t *arena);
result_ref_t metricsStart(const char *label);
result_void_t metricsEnd(result_ref_t *span_ref);
void metricsReport(void);

#define profile                                                                \
  result_ref_t _concat(metrics_, __LINE__)                                     \
      __attribute__((cleanup(metricsEnd))) = metricsStart(__FUNCTION__);
