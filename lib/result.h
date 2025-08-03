#pragma once

#include <stdio.h>
typedef char message_t[64];

constexpr int RESULT_OK = 0;

#define Result(ValueType, ...)                                                 \
  struct {                                                                     \
    int code;                                                                  \
    __VA_OPT__(__VA_ARGS__ meta);                                              \
    union {                                                                    \
      ValueType value;                                                         \
      message_t message;                                                       \
    };                                                                         \
  }

#define ResultVoid(...)                                                        \
  struct {                                                                     \
    int code;                                                                  \
    __VA_OPT__(__VA_ARGS__ meta);                                              \
    message_t message;                                                         \
  }

typedef ResultVoid() result_void_t;

#define _concat_detail(x, y) x##y
#define _concat(x, y) _concat_detail(x, y)
#define _result(ResultType) _concat(ResultType, __LINE__)
#define _error(ResultType) _concat(error_, _concat(ResultType, __LINE__))

#define ok(T, ...)                                                             \
  (T) { .code = 0 __VA_OPT__(, .value =) __VA_ARGS__ }

#define tryAssign(ResultType, Action, Destination, ...)                        \
  auto _result(ResultType) = Action;                                           \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    _concat(throw, __VA_OPT__(Meta))(ResultType, _result(ResultType).code,     \
                                     __VA_OPT__(__VA_ARGS__, ) "%s",           \
                                     _result(ResultType).message);             \
  }                                                                            \
  (Destination) = (_result(ResultType).value);

#define try(ResultType, Action, ...)                                           \
  auto _result(ResultType) = Action;                                           \
  if (_result(ResultType).code != RESULT_OK) {                                 \
    _concat(throw, __VA_OPT__(Meta))(ResultType, _result(ResultType).code,     \
                                     __VA_OPT__(__VA_ARGS__, ) "%s",           \
                                     _result(ResultType).message);             \
  }

#define throw(ResultType, Code, Fmt, ...)                                      \
  {                                                                            \
    ResultType _error(ResultType);                                             \
    _error(ResultType).code = Code;                                            \
    snprintf(_error(ResultType).message, sizeof(message_t),                    \
             Fmt __VA_OPT__(, ) __VA_ARGS__);                                  \
    return _error(ResultType);                                                 \
  }

#define throwMeta(ResultType, Code, Meta, Fmt, ...)                            \
  {                                                                            \
    ResultType _error(ResultType);                                             \
    _error(ResultType).code = Code;                                            \
    _error(ResultType).meta = Meta;                                            \
    snprintf(_error(ResultType).message, sizeof(message_t),                    \
             Fmt __VA_OPT__(, ) __VA_ARGS__);                                  \
    return _error(ResultType);                                                 \
  }
