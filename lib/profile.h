#pragma once

#ifdef MEMORY_PROFILE
#include "arena.h"
#include "result.h"

constexpr size_t MAX_SUBSPAN = 16;
constexpr size_t SPAN_LABEL_LEN = 16;

typedef struct span_t {
  char label[SPAN_LABEL_LEN];
  unsigned long hits;
  unsigned long last;
  unsigned long total;
  unsigned long subspans_count;
  unsigned long span_id;
  struct span_t *parent;
  struct span_t *subspans[MAX_SUBSPAN];
  void *payload;
} span_t;

void profileInit(void);
void profileReport(void);
void profileEnd(void);

span_t *safeAllocSpanStart(const char *label);
void safeAllocSpanEnd(span_t **span_double_ref);
span_t *arenaSpanStart(arena_t *arena, const char *label);
void arenaSpanEnd(span_t **span_double_ref);

#define profileSafeAlloc()                                                     \
  span_t *_concat(metrics_, __LINE__)                                          \
      __attribute__((cleanup(safeAllocSpanEnd))) =                             \
          safeAllocSpanStart(__FUNCTION__);

#define profileArena(Arena)                                                    \
  span_t *_concat(metrics_, __LINE__) __attribute__((cleanup(arenaSpanEnd))) = \
      arenaSpanStart(Arena, __FUNCTION__);

#else

#define profileReport()
#define profileInit()
#define profileEnd()

#define safeAllocSpanStart(Label)
#define safeAllocSpanEnd(Span)
#define arenaSpanStart(Arena, Label)
#define arenaSpanEnd(Span)

#define profileSafeAlloc()
#define profileArena(Arena)

#endif
