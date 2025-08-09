#include "profile.h"

#ifdef MEMORY_PROFILE

#include "alloc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

span_t *CURRENT_SPAN = nullptr;
span_t *ROOT_SPAN = nullptr;

static unsigned long getCurrentMemory(void) { return allocGetMetrics().bytes; }

static unsigned long hash(const char *key) {
  static constexpr unsigned long prime = 1099511628211U;
  unsigned long hash = 14695981039346656037U;
  const size_t len = strlen(key);

  for (size_t i = 0; i < len; i++) {
    hash ^= (unsigned long)(unsigned char)key[i];
    hash *= prime;
  }

  return hash;
}

static span_t *spanCreate(const char *label) {
  span_t *span = malloc(sizeof(span_t));
  span->hits = 0;
  span->total = 0;
  span->last = 0;
  span->label[0] = 0;
  span->subspans_count = 0;
  span->span_id = hash(label);

  auto label_len = strlen(label);
  label_len = label_len >= SPAN_LABEL_LEN ? SPAN_LABEL_LEN - 1 : label_len;
  strncpy(span->label, label, label_len);
  span->label[label_len] = 0;
  return span;
}

static void spanFree(span_t **span) {
  if (*span == nullptr)
    return;

  for (unsigned long i = 0; i < (*span)->subspans_count; i++) {
    spanFree(&(*span)->subspans[i]);
  }
  deallocSafe(span);
}

void profileInit() {
  metrics.bytes = 0;
  metrics.segments_count = 0;
  CURRENT_SPAN = spanCreate("root");
  ROOT_SPAN = CURRENT_SPAN;
  ROOT_SPAN->hits = 1; // prevent division by zero
}

span_t *spanStart(const char *label) {
  assert(CURRENT_SPAN);
  const unsigned long span_id = hash(label);

  span_t *span = nullptr;
  for (unsigned long i = 0; i < CURRENT_SPAN->subspans_count; i++) {
    if (CURRENT_SPAN->subspans[i]->span_id == span_id) {
      span = CURRENT_SPAN->subspans[i];
    }
  }

  if (!span) {
    span = spanCreate(label);
    if (CURRENT_SPAN->subspans_count < MAX_SUBSPAN) {
      CURRENT_SPAN->subspans[CURRENT_SPAN->subspans_count] = span;
      CURRENT_SPAN->subspans_count++;
    } else {
      printf("cannot allocate new profiling span\n");
      return nullptr;
    }
  }

  span->hits++;
  span->last = getCurrentMemory();
  span->parent = CURRENT_SPAN;
  CURRENT_SPAN = span;
  return span;
}

void spanEnd(span_t **span_double_ref) {
  span_t *span = *span_double_ref;
  unsigned long delta = getCurrentMemory() - span->last;
  span->total += delta;

  if (span->parent) {
    span->parent->total += delta;
  }

  CURRENT_SPAN = span->parent;
}

void printSpan(const span_t *span, int indentation) {
  char prefix[32];
  snprintf(prefix, 32, "%*c", indentation * 2, ' ');

  printf("%s %s[%lu]: %lu bytes (%0.2f bytes/hit)\n", prefix, span->label,
         span->hits, span->total, (double)span->total / (double)span->hits);

  for (unsigned long i = 0; i < span->subspans_count; i++) {
    printSpan(span->subspans[i], indentation + 1);
  }
}

void profileReport(void) {
  assert(CURRENT_SPAN);

  printf(" === Memory Metrics: Leaked safeAlloc ===\n");
  printSpan(ROOT_SPAN, 0);
}

void profileEnd(void) {
  spanFree(&ROOT_SPAN);
  CURRENT_SPAN = nullptr;
}
#endif
