#ifndef ALLOC_H
#define ALLOC_H

#include "result.h"
#include <stddef.h>
#include <stdlib.h>

#define deallocSafe(ptr)                                                       \
  {                                                                            \
    free(ptr);                                                                 \
    ptr = nullptr;                                                             \
  }

typedef Result(void *) result_alloc_t;

static inline result_alloc_t allocSafe(size_t size) {
  void *ptr = malloc(size);

  if (ptr == nullptr) {
    exception_payload_t payload = {.allocation = nullptr};
    return err(result_alloc_t, EXCEPTION_KIND_ALLOCATION, payload);
  }

  return ok(result_alloc_t, ptr);
}

static inline result_alloc_t reallocSafe(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);

  if (new_ptr == nullptr) {
    exception_payload_t payload = {.allocation = nullptr};
    return err(result_alloc_t, EXCEPTION_KIND_ALLOCATION, payload);
  }

  return ok(result_alloc_t, new_ptr);
}

#endif // ALLOC_H
