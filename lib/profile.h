#pragma once

#ifdef MEMORY_PROFILE
#include "result.h"

constexpr size_t MAX_SUBSPAN = 16;
constexpr size_t SPAN_LABEL_LEN = 16;

typedef struct span_t {
  char label[SPAN_LABEL_LEN];
  unsigned long hits;
  unsigned long total;
  unsigned long last;
  unsigned long subspans_count;
  unsigned long span_id;
  struct span_t *parent;
  struct span_t *subspans[MAX_SUBSPAN];
} span_t;

void profileInit(void);
void profileReport(void);
void profileEnd(void);

span_t *spanStart(const char *label);
void spanEnd(span_t **span_double_ref);

#define profileAllocations()                                                   \
  span_t *_concat(metrics_, __LINE__) __attribute__((cleanup(spanEnd))) =      \
      spanStart(__FUNCTION__);

#else

#define profileAllocations()
#define profileReport()
#define profileInit(Arena)
#define profileEnd()

#endif
