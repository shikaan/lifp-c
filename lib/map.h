// Map (v0.0.1)
// ---
//
// Generic hash map implementation with arena allocation.
//
// The map provides a type-safe hash table implementation using C macros for
// generic programming. It uses linear probing for collision resolution and
// stores all data in an arena allocator for efficient bulk deallocation.
// Keys are limited to 32 characters and stored as fixed-length strings.
//
// Key features:
// - Type-safe generic interface using C macros
// - Arena-based memory allocation for cache locality
// - Linear probing hash table with open addressing
// - Fixed maximum key length for predictable memory usage
// - O(1) average case insertion and lookup
//
// ```c
// result_ref_t arena_result = arenaCreate(1024);
// if (arena_result.ok) {
//     arena_t *arena = arena_result.value;
//
//     // Create a map for integers
//     result_ref_t map_result = mapCreate(int, arena, 16);
//     if (map_result.ok) {
//         Map(int) *map = map_result.value;
//
//         // Set values
//         int value = 42;
//         mapSet(map, "answer", &value);
//
//         // Get values
//         int *retrieved = mapGet(int, map, "answer");
//         if (retrieved) {
//             printf("Value: %d\n", *retrieved); // Prints: Value: 42
//         }
//     }
//
//     arenaDestroy(arena);
// }
// ```

#pragma once

#include "arena.h"
#include "result.h"
#include <stddef.h>
#include <stdint.h>

/**
 * Maximum length for map keys including null terminator.
 * @name MAX_KEY_LENGTH
 */
constexpr size_t MAX_KEY_LENGTH = 32;

typedef enum {
  MAP_ERROR_ALLOCATION = ARENA_ERROR_OUT_OF_SPACE,
  MAP_ERROR_INVALID_KEY,
} map_error_t;

/**
 * Generic map structure macro.
 * Creates a type-safe map structure for the specified value type.
 * @name Map
 * @param {Type} Type - The value type to store in the map
 * @example
 *   Map(int) *int_map;
 *   Map(char*) *string_map;
 */
#define Map(Type)                                                              \
  struct {                                                                     \
    size_t count;                                                              \
    size_t capacity;                                                           \
    size_t item_size;                                                          \
    bool *used;                                                                \
    char (*keys)[MAX_KEY_LENGTH];                                              \
    Type *values;                                                              \
    arena_t *arena;                                                            \
  }

typedef Map(void) generic_map_t;

/**
 * Create a new map with the specified type and capacity.
 * @name mapCreate
 * @param {Type} ItemType - The type of values to store in the map
 * @param {arena_t*} Arena - Arena allocator to use for memory allocation
 * @param {size_t} Capacity - Initial capacity of the map (should be power of 2)
 * @returns {result_ref_t} Result containing the map pointer on success, or
 * allocation error
 * @example
 *   result_ref_t result = mapCreate(int, arena, 16);
 *   if (result.ok) {
 *       Map(int) *map = result.value;
 *       // Use the map...
 *   }
 */
#define mapCreate(ItemType, Arena, Capacity)                                   \
  genericMapCreate(Arena, Capacity, sizeof(ItemType))

/**
 * Set a key-value pair in the map.
 * @name mapSet
 * @param {Map(Type)*} Map - Pointer to the map to modify
 * @param {const char*} Key - Key string (max 31 characters + null terminator)
 * @param {Type*} Value - Pointer to the value to store
 * @returns {result_ref_t} Result indicating success or allocation error
 * @example
 *   int value = 42;
 *   result_ref_t result = mapSet(map, "key", &value);
 *   if (!result.ok) {
 *       // Handle error
 *   }
 */
#define mapSet(Map, Key, Value)                                                \
  genericMapSet((generic_map_t *)(Map), Key, Value)

/**
 * Get a value from the map by key.
 * @name mapGet
 * @param {Type} ItemType - The type of values stored in the map
 * @param {Map(Type)*} Map - Pointer to the map to search
 * @param {const char*} Key - Key string to search for
 * @returns {Type*} Pointer to the value if found, NULL if not found
 * @example
 *   int *value = mapGet(int, map, "key");
 *   if (value) {
 *       printf("Found: %d\n", *value);
 *   } else {
 *       printf("Key not found\n");
 *   }
 */
#define mapGet(ItemType, Map, Key)                                             \
  (ItemType *)genericMapGet((generic_map_t *)(Map), Key)

/**
 * Create a new generic map with the specified capacity and item size.
 * @name genericMapCreate
 * @param {arena_t*} arena - Arena allocator to use for memory allocation
 * @param {size_t} capacity - Initial capacity of the map
 * @param {size_t} item_size - Size in bytes of each value item
 * @returns {result_ref_t} Result containing the map pointer on success, or
 * allocation error
 */
result_ref_t genericMapCreate(arena_t *arena, size_t capacity,
                              size_t item_size);

/**
 * Set a key-value pair in a generic map.
 * @name genericMapSet
 * @param {generic_map_t*} self - Pointer to the map to modify
 * @param {const char*} key - Key string (max 31 characters + null terminator)
 * @param {void*} value - Pointer to the value to store
 * @returns {result_ref_t} Result indicating success or allocation error
 */
result_void_t genericMapSet(generic_map_t *self, const char *key, void *value);

/**
 * Get a value from a generic map by key.
 * @name genericMapGet
 * @param {generic_map_t*} self - Pointer to the map to search
 * @param {const char*} key - Key string to search for
 * @returns {void*} Pointer to the value if found, NULL if not found
 */
void *genericMapGet(generic_map_t *self, const char *key);

/**
 * Get the total memory size of a map including all allocated arrays.
 * @name mapSize
 * @param {Map(Type)*} Map - Pointer to the map
 * @returns {size_t} Total size in bytes of the map structure and its data
 * @example
 *   size_t total_size = mapSize(map);
 *   printf("Map uses %zu bytes\n", total_size);
 */
#define mapSize(Map) genericMapSize((generic_map_t *)(Map))

/**
 * Get the total memory size of a generic map including all allocated arrays.
 * @name genericMapSize
 * @param {generic_map_t*} self - Pointer to the map
 * @returns {size_t} Total size in bytes of the map structure and its data
 */
size_t genericMapSize(generic_map_t *self);
