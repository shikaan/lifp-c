// Arena (v0.0.1)
// ---
//
// Arena allocator for fast memory allocation and bulk deallocation.
//
// The arena allocator provides a fast allocation strategy where memory is
// allocated from a pre-allocated block in a sequential manner. All allocations
// can be freed at once by resetting the arena, making it ideal for temporary
// allocations during parsing, evaluation, and other operations that need to
// allocate many small objects that have similar lifetimes.
//
// Key features:
// - Fast O(1) allocation with no fragmentation
// - Bulk deallocation by resetting the arena
// - No individual free operations needed
// - Memory-efficient for scenarios with many small allocations
//
// ```c
// result_ref_t result = arenaCreate(1024);
// if (result.ok) {
//     arena_t *arena = result.value;
//
//     // Allocate some memory
//     result_ref_t alloc = arenaAllocate(arena, sizeof(int));
//     if (alloc.ok) {
//         int *ptr = (int*)alloc.value;
//         *ptr = 42;
//     }
//
//     // Reset and reuse
//     arenaReset(arena);
//
//     // Clean up
//     arenaDestroy(arena);
// }
// ```

#pragma once

#include "result.h"
#include <stddef.h>

typedef unsigned char byte_t;

/**
 * The result of an allocation. It returns a void* to be casted by the caller.
 * @name result_ref_t
 */
typedef Result(void *) result_ref_t;

/**
 * Arena allocator structure.
 * @name arena_t
 */
typedef struct {
  size_t size;     // Total size of the arena memory buffer
  size_t offset;   // Current allocation offset within the buffer
  byte_t memory[]; // Flexible array member containing the actual memory buffer
} arena_t;

/**
 * Create a new arena with the specified size.
 * @name arenaCreate
 * @param {size_t} size - The total size in bytes for the arena's memory buffer
 * @returns {result_ref_t} Result containing the arena pointer on success, or
 * an allocation error
 * @example
 *   result_ref_t result = arenaCreate(1024);
 *   if (result.ok) {
 *       arena_t *arena = result.value;
 *       // Use the arena...
 *       arenaDestroy(arena);
 *   }
 */
result_ref_t arenaCreate(size_t size);

/**
 * Allocate memory from the arena.
 * @name arenaAllocate
 * @param {arena_t*} self - Pointer to the arena to allocate from
 * @param {size_t} size - Number of bytes to allocate
 * @returns {result_ref_t} Result containing pointer to allocated memory on
 * success, or allocation error
 * @example
 *   result_ref_t result = arenaAllocate(arena, sizeof(int));
 *   if (result.ok) {
 *       int *ptr = (int*)result.value;
 *       *ptr = 42;
 *   }
 */
result_ref_t arenaAllocate(arena_t *self, size_t size);

/**
 * Destroy the arena and free all its memory.
 * @name arenaDestroy
 * @param {arena_t*} self - Pointer to the arena to destroy
 * @example
 *   arenaDestroy(arena);  // arena pointer becomes invalid
 */
void arenaDestroy(arena_t *self);

/**
 * Reset the arena to empty state.
 * @name arenaReset
 * @param {arena_t*} self - Pointer to the arena to reset
 * @example
 *   arenaAllocate(arena, 100);
 *   arenaAllocate(arena, 200);
 *   arenaReset(arena);  // All memory now available again
 */
void arenaReset(arena_t *self);

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
void bytewiseCopy(void *dest, const void *src, size_t size);
