#ifndef RESULT_H
#define RESULT_H

#include "./position.h"

typedef enum {
  EXCEPTION_KIND_UNEXPECTED_TOKEN,
  EXCEPTION_KIND_ALLOCATION,
} exception_kind_t;

typedef struct {
  char token;
  position_t position;
} unexpected_token_payload_t;

typedef union {
  unexpected_token_payload_t unexpected_token;
  nullptr_t allocation;
} exception_payload_t;

typedef struct {
  exception_kind_t kind;
  exception_payload_t payload;
} exception_t;

#define Result(T)                                                              \
  struct {                                                                     \
    bool ok;                                                                   \
    union {                                                                    \
      T value;                                                                 \
      exception_t error;                                                       \
    };                                                                         \
  }

// Helper functions for creating results
#define ok(T, Value)                                                           \
  (T) { .ok = true, .value = (Value) }
#define err(T, Kind, Payload)                                                  \
  (T) {                                                                        \
    .ok = false, .error = (exception_t) {                                      \
      .kind = (Kind), .payload = (Payload)                                     \
    }                                                                          \
  }

#endif // RESULT_H
