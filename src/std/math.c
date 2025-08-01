#include "../result.h"
#include "../value.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// Math max function - returns the maximum value in a list of numbers
const char *MATH_MAX = "math.max";
result_void_t mathMax(value_t *result, value_list_t *values) {
  if (values->count != 1) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 1,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t list_value = listGet(value_t, values, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)list_value.type,
                     .payload.unexpected_type.expected = NODE_TYPE_LIST,
                     .position = list_value.position};
    return error(result_void_t, error);
  }

  value_list_t *list = &list_value.value.list;
  if (list->count == 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_VALUE,
                     .payload.unexpected_value.expected = nullptr,
                     .payload.unexpected_value.actual = nullptr,
                     .position = list_value.position};
    return error(result_void_t, error);
  }

  // Find the maximum value
  int32_t max_value = INT32_MIN;
  for (size_t i = 0; i < list->count; i++) {
    value_t current = listGet(value_t, list, i);
    if (current.type != VALUE_TYPE_INTEGER) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)current.type,
                       .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                       .position = current.position};
      return error(result_void_t, error);
    }

    if (current.value.integer > max_value) {
      max_value = current.value.integer;
    }
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = max_value;

  return (result_void_t){.ok = true};
}

// Math min function - returns the minimum value in a list of numbers
const char *MATH_MIN = "math.min";
result_void_t mathMin(value_t *result, value_list_t *values) {
  if (values->count != 1) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 1,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t list_value = listGet(value_t, values, 0);
  if (list_value.type != VALUE_TYPE_LIST) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)list_value.type,
                     .payload.unexpected_type.expected = NODE_TYPE_LIST,
                     .position = list_value.position};
    return error(result_void_t, error);
  }

  value_list_t *list = &list_value.value.list;
  if (list->count == 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_VALUE,
                     .payload.unexpected_value.expected = nullptr,
                     .payload.unexpected_value.actual = nullptr,
                     .position = list_value.position};
    return error(result_void_t, error);
  }

  // Find the minimum value
  int32_t min_value = INT32_MAX;
  for (size_t i = 0; i < list->count; i++) {
    value_t current = listGet(value_t, list, i);
    if (current.type != VALUE_TYPE_INTEGER) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)current.type,
                       .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                       .position = current.position};
      return error(result_void_t, error);
    }

    if (current.value.integer < min_value) {
      min_value = current.value.integer;
    }
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = min_value;

  return (result_void_t){.ok = true};
}

// Math random function - returns a random integer between 0 and RAND_MAX
const char *MATH_RANDOM = "math.random";
result_void_t mathRandom(value_t *result, value_list_t *values) {
  if (values->count != 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 0,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  // Initialize random number generator on first call
  static bool initialized = false;
  if (!initialized) {
    srand((unsigned int)time(nullptr));
    initialized = true;
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = rand();

  return (result_void_t){.ok = true};
}
