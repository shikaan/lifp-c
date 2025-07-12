#pragma once

#include "result.h"
#include <stddef.h>
#include <stdlib.h>

#define deallocSafe(DoublePointer)                                             \
  {                                                                            \
    if (*(DoublePointer) != nullptr) {                                         \
      free((void *)*(DoublePointer));                                          \
      *(DoublePointer) = nullptr;                                              \
    }                                                                          \
  }

typedef Result(void *) result_alloc_t;

static inline result_alloc_t allocSafe(size_t size) {
  void *ptr = malloc(size);

  if (ptr == nullptr) {
    exception_t exception = {.kind = EXCEPTION_KIND_ALLOCATION};
    return error(result_alloc_t, exception);
  }

  return ok(result_alloc_t, ptr);
}

static inline result_alloc_t reallocSafe(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);

  if (new_ptr == nullptr) {
    exception_t exception = {.kind = EXCEPTION_KIND_ALLOCATION};
    return error(result_alloc_t, exception);
  }

  return ok(result_alloc_t, new_ptr);
}

static inline void bytewiseCopy(void *dest, const void *src, size_t size) {
  unsigned char *dest_bytes = (unsigned char *)dest;
  const unsigned char *src_bytes = (const unsigned char *)src;
  for (size_t i = 0; i < size; i++) {
    dest_bytes[i] = src_bytes[i];
  }
}