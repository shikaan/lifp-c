#include "profile.h"
#include "arena.h"
#include "memory.h"
#include "result.h"
#include <assert.h>
#include <mach/mach.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>

// Global span variables
span_t *CURRENT_SPAN = nullptr;
span_t *ROOT_SPAN = nullptr;

static long getCurrentMemory(void) {
  struct mach_task_basic_info info;
  mach_msg_type_number_t info_count = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info,
                &info_count) != KERN_SUCCESS)
    return 0;
  return (long)info.resident_size; // in bytes
}

static unsigned long hash(const char *key) {
  unsigned long hash = 14695981039346656037U;
  const unsigned long prime = 1099511628211U;
  const size_t len = strlen(key);

  for (size_t i = 0; i < len; i++) {
    hash ^= (unsigned long)(unsigned char)key[i];
    hash *= prime;
  }

  return hash;
}

static result_ref_t spanCreate(arena_t *arena, const char *label) {
  span_t *span;
  try(result_ref_t, arenaAllocate(arena, sizeof(span_t)), span);
  span->hits = 0;
  span->total = 0;
  span->last = 0;
  span->arena = arena;
  span->label[0] = 0;
  span->subspans_count = 0;
  span->span_id = hash(label);
  strncpy(span->label, label, strlen(label));
  return ok(result_ref_t, span);
}

result_void_t metricsInit(arena_t *arena) {
  try(result_void_t, spanCreate(arena, "root"), CURRENT_SPAN);
  ROOT_SPAN = CURRENT_SPAN;
  ROOT_SPAN->hits = 1; // prevent division by zero
  return ok(result_void_t);
}

result_ref_t metricsStart(const char *label) {
  assert(CURRENT_SPAN);
  unsigned long id = hash(label);

  span_t *span = nullptr;
  for (long i = 0; i < CURRENT_SPAN->subspans_count; i++) {
    if (CURRENT_SPAN->subspans[i]->span_id == id) {
      span = CURRENT_SPAN->subspans[i];
    }
  }

  if (!span) {
    try(result_ref_t, spanCreate(CURRENT_SPAN->arena, label), span);
    CURRENT_SPAN->subspans[CURRENT_SPAN->subspans_count] = span;
    CURRENT_SPAN->subspans_count++;
  }

  span->hits++;
  span->last = getCurrentMemory();
  span->parent = CURRENT_SPAN;
  CURRENT_SPAN = span;

  return ok(result_ref_t, span);
}

result_void_t metricsEnd(result_ref_t *span_ref) {
  span_t *span = span_ref->value;
  span->total += getCurrentMemory() - span->last;

  if (span->parent) {
    span->parent->total += span->total;
  }

  CURRENT_SPAN = span->parent;
  return ok(result_void_t);
}

void printSpan(const span_t *span, int indentation) {
  char prefix[32];
  snprintf(prefix, 32, "%*c", indentation * 2, ' ');

  printf("%s %s[%lu]: %lu bytes (%0.2f bytes)\n", prefix, span->label,
         span->hits, span->total, (double)span->total / (double)span->hits);

  for (long i = 0; i < span->subspans_count; i++) {
    printSpan(span->subspans[i], indentation + 1);
  }
}

void metricsReport(void) {
  assert(CURRENT_SPAN);

  printf(" === Memory Metrics ===\n");
  printSpan(ROOT_SPAN, 0);
}
