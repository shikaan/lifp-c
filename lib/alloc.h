#pragma once

#include "./result.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef ALLOC_PROFILE
#include <string.h>
constexpr long MAX_SEGMENTS = 1024;

extern unsigned long allocated;
extern long segments_count;
extern void *pointers[MAX_SEGMENTS];
extern size_t sizes[MAX_SEGMENTS];
extern char labels[16][MAX_SEGMENTS];

static inline void _allocationProfileReport(void) {
  size_t frees = 0;
  for (long i = 0; i < segments_count; i++) {
    if (sizes[i] == 0)
      frees++;
  }

  printf(" === Memory Profiler: Malloc ===\n");
  printf("    # malloc: %lu\n", segments_count);
  printf("      # free: %lu\n", frees);
  printf("   allocated: %lu bytes\n", allocated);
}

#define allocationProfileInit()                                                \
  unsigned long allocated = 0;                                                 \
  long segments_count = 0;                                                     \
  void *pointers[MAX_SEGMENTS] = {};                                           \
  size_t sizes[MAX_SEGMENTS] = {};                                             \
  char labels[16][MAX_SEGMENTS] = {};

#define allocationProfileStart(Pointer, Size, Label)                           \
  {                                                                            \
    if (segments_count < MAX_SEGMENTS) {                                       \
      pointers[segments_count] = Pointer;                                      \
      sizes[segments_count] = Size;                                            \
      memcpy(labels[segments_count], Label, strlen(Label));                    \
      segments_count++;                                                        \
      allocated += (Size);                                                     \
    }                                                                          \
  }

#define allocationProfileEnd(DoublePointer)                                    \
  {                                                                            \
    for (long i = 0; i < segments_count; i++) {                                \
      if (*(DoublePointer) == pointers[i]) {                                   \
        allocated -= sizes[i];                                                 \
        sizes[i] = 0;                                                          \
        pointers[i] = nullptr;                                                 \
      }                                                                        \
    }                                                                          \
  }

#define allocationProfileReport() _allocationProfileReport()

#else

#define allocationProfileInit()
#define allocationProfileStart(Pointer, Size, Label)
#define allocationProfileEnd(DoublePointer)
#define allocationProfileReport()

#endif

typedef enum {
  ALLOC_ERROR_MALLOC_ERROR = 1,
} alloc_error_t;

/**
 * The result of an allocation. It returns a void* to be casted by the caller.
 * @name result_ref_t
 */
typedef Result(void *) result_ref_t;

/**
 * Copy bytes from source to destination memory locations.
 * @name bytewiseCopy
 * @param {void*} dest - Pointer to the destination memory location
 * @param {const void*} src - Pointer to the source memory location to copy from
 * @param {size_t} size - Number of bytes to copy
 * @example
 *   char source[] = "Hello";
 *   char dest[6];
 *   bytewiseCopy(dest, source, 6);  // dest now contains "Hello"
 */
static inline void bytewiseCopy(void *dest, const void *src, size_t size) {
  const auto dest_bytes = (unsigned char *)dest;
  const auto src_bytes = (const unsigned char *)src;
  for (size_t i = 0; i < size; i++) {
    dest_bytes[i] = src_bytes[i];
  }
}

/**
 * Safely allocate memory with error handling.
 * @name allocSafe
 * @param {size_t} size - Number of bytes to allocate
 * @returns {result_ref_t} Result containing allocated memory pointer on
 * success, or error on failure
 * @example
 *   result_ref_t result = allocSafe(100);
 *   if (isOk(result)) {
 *     void *ptr = unwrap(result);
 *     // Use allocated memory
 *   }
 */
#define allocSafe(Size) _allocSafe(Size, __FUNCTION__)
static inline result_ref_t _allocSafe(size_t size,
                                      [[maybe_unused]] const char *label) {
  void *ptr = malloc(size);

  if (ptr == nullptr) {
    throw(result_ref_t, ALLOC_ERROR_MALLOC_ERROR, nullptr,
          "Unable to allocate size: %lu", size);
  }

  allocationProfileStart(ptr, size, label);

  return ok(result_ref_t, ptr);
}

/**
 * Safely deallocate memory and set pointer to nullptr.
 * @name deallocSafe
 * @param {void**} DoublePointer - Pointer to the pointer that should be freed
 * @example
 *   char *ptr = malloc(100);
 *   deallocSafe(&ptr);  // ptr is now nullptr
 */
#define deallocSafe(DoublePointer)                                             \
  {                                                                            \
    if (*(DoublePointer) != nullptr) {                                         \
      allocationProfileEnd(DoublePointer);                                     \
      free((void *)*(DoublePointer));                                          \
      *(DoublePointer) = nullptr;                                              \
    }                                                                          \
  }
