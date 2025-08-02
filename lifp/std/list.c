#include "../../lib/result.h"
#include "../value.h"
#include <stdint.h>

// List count function - counts elements in a list
const char *LIST_COUNT = "list.count";
result_void_t listCount(value_t *result, value_list_t *values) {
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

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = (int32_t)list_value.value.list.count;

  return (result_void_t){.ok = true};
}

// List from function - creates a list from the given arguments
const char *LIST_FROM = "list.from";
result_void_t listFrom(value_t *result, value_list_t *values) {
  result->type = VALUE_TYPE_LIST;

  value_list_t *new_list = nullptr;
  tryAssign(result_void_t, listCreate(value_t, values->arena, values->count),
            new_list);
  result->value.list = *new_list;

  if (values->count > 0) {
    // Copy all values to the new list
    for (size_t i = 0; i < values->count; i++) {
      value_t source = listGet(value_t, values, i);
      try(result_void_t, listAppend(value_t, &result->value.list, &source));
    }
  } else {
    new_list->data = nullptr;
  }

  return (result_void_t){.ok = true};
}

// List nth function - returns the nth element of a list, or nil if out of
// bounds
const char *LIST_NTH = "list.nth";
result_void_t listNth(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t index_value = listGet(value_t, values, 0);
  value_t list_value = listGet(value_t, values, 1);

  if (index_value.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)index_value.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = index_value.position};
    return error(result_void_t, error);
  }

  if (list_value.type != VALUE_TYPE_LIST) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)list_value.type,
                     .payload.unexpected_type.expected = NODE_TYPE_LIST,
                     .position = list_value.position};
    return error(result_void_t, error);
  }

  int32_t index = index_value.value.integer;
  value_list_t *list = &list_value.value.list;

  // Check bounds
  if (index < 0 || (size_t)index >= list->count) {
    result->type = VALUE_TYPE_NIL;
    result->value.nil = nullptr;
  } else {
    *result = listGet(value_t, list, (size_t)index);
  }

  return (result_void_t){.ok = true};
}
