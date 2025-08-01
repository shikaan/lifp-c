#include "../result.h"
#include "../value.h"
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

const char *FLOW_SLEEP = "flow.sleep";

result_void_t flowSleep(value_t *result, value_list_t *values) {
  if (values->count != 1) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 1,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t ms_value = listGet(value_t, values, 0);
  if (ms_value.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)ms_value.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = ms_value.position};
    return error(result_void_t, error);
  }

  int32_t milliseconds = ms_value.value.integer;
  if (milliseconds < 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_VALUE,
                     .payload.unexpected_value.expected = nullptr,
                     .payload.unexpected_value.actual = nullptr,
                     .position = ms_value.position};
    return error(result_void_t, error);
  }

  // Perform the sleep operation
#ifdef _WIN32
  Sleep((DWORD)milliseconds);
#else
  // Convert milliseconds to nanoseconds for nanosleep
  struct timespec timespec_val;
  timespec_val.tv_sec = milliseconds / 1000;
  timespec_val.tv_nsec = (milliseconds % 1000) * 1000000L;
  nanosleep(&timespec_val, nullptr);
#endif

  // Return nil value
  result->type = VALUE_TYPE_NIL;
  result->value.nil = nullptr;

  return (result_void_t){.ok = true};
}
