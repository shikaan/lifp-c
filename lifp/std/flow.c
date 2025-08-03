#include "../../lib/result.h"
#include "../error.h"
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

result_void_position_t flowSleep(value_t *result, value_list_t *values) {
  if (values->count != 1) {
    throwMeta(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
              result->position, "%s requires exactly 1 argument. Got %zu",
              FLOW_SLEEP, values->count);
  }

  value_t ms_value = listGet(value_t, values, 0);
  if (ms_value.type != VALUE_TYPE_INTEGER) {
    throwMeta(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
              ms_value.position, "%s requires an integer. Got type %u",
              FLOW_SLEEP, ms_value.type);
  }

  int32_t milliseconds = ms_value.value.integer;
  if (milliseconds < 0) {
    throwMeta(result_void_position_t, ERROR_CODE_RUNTIME_ERROR,
              ms_value.position, "%s requires a non-negative integer",
              FLOW_SLEEP);
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

  return ok(result_void_position_t);
}
