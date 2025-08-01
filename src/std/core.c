#include "../result.h"
#include "../value.h"
#include <stdint.h>

const char *SUM = "+";
result_void_t sum(value_t *result, value_list_t *values) {
  int32_t sum = 0;
  for (size_t i = 0; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    if (current.type != VALUE_TYPE_INTEGER) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)current.type,
                       .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                       .position = current.position};
      return error(result_void_t, error);
    }

    sum += current.value.integer;
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = sum;

  return (result_void_t){.ok = true};
}

const char *SUB = "-";
result_void_t subtract(value_t *result, value_list_t *values) {
  if (values->count == 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 1,
                     .payload.unexpected_arity.actual = 0,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  if (first.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)first.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.position};
    return error(result_void_t, error);
  }

  int32_t result_value = first.value.integer;

  for (size_t i = 1; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    if (current.type != VALUE_TYPE_INTEGER) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)current.type,
                       .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                       .position = current.position};
      return error(result_void_t, error);
    }

    result_value -= current.value.integer;
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = result_value;

  return (result_void_t){.ok = true};
}

const char *MUL = "*";
result_void_t multiply(value_t *result, value_list_t *values) {
  int32_t product = 1;
  for (size_t i = 0; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    if (current.type != VALUE_TYPE_INTEGER) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)current.type,
                       .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                       .position = current.position};
      return error(result_void_t, error);
    }

    product *= current.value.integer;
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = product;

  return (result_void_t){.ok = true};
}

const char *DIV = "/";
result_void_t divide(value_t *result, value_list_t *values) {
  if (values->count == 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 1,
                     .payload.unexpected_arity.actual = 0,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  if (first.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)first.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.position};
    return error(result_void_t, error);
  }

  int32_t result_value = first.value.integer;

  for (size_t i = 1; i < values->count; i++) {
    value_t current = listGet(value_t, values, i);
    if (current.type != VALUE_TYPE_INTEGER) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                       .payload.unexpected_type.actual = (int)current.type,
                       .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                       .position = current.position};
      return error(result_void_t, error);
    }

    if (current.value.integer == 0) {
      error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                       .payload.unexpected_arity.expected = 1,
                       .payload.unexpected_arity.actual = 0,
                       .position = current.position};
      return error(result_void_t, error);
    }

    result_value /= current.value.integer;
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = result_value;

  return (result_void_t){.ok = true};
}

const char *MOD = "%";
result_void_t modulo(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)first.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.position};
    return error(result_void_t, error);
  }

  if (second.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = second.position};
    return error(result_void_t, error);
  }

  if (second.value.integer == 0) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 1,
                     .payload.unexpected_arity.actual = 0,
                     .position = second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_INTEGER;
  result->value.integer = first.value.integer % second.value.integer;

  return (result_void_t){.ok = true};
}

const char *EQUAL = "=";
result_void_t equal(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  bool are_equal = false;
  if (first.type == second.type) {
    switch (first.type) {
    case VALUE_TYPE_INTEGER:
      are_equal = first.value.integer == second.value.integer;
      break;
    case VALUE_TYPE_BOOLEAN:
      are_equal = first.value.boolean == second.value.boolean;
      break;
    case VALUE_TYPE_NIL:
      are_equal = true;
      break;
    case VALUE_TYPE_BUILTIN:
    case VALUE_TYPE_CLOSURE:
    case VALUE_TYPE_LIST:
    default:
      are_equal = false;
      break;
    }
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = are_equal;

  return (result_void_t){.ok = true};
}

const char *LESS_THAN = "<";
result_void_t lessThan(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_INTEGER || second.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual =
                         first.type != VALUE_TYPE_INTEGER ? (int)first.type
                                                          : (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.type != VALUE_TYPE_INTEGER
                                     ? first.position
                                     : second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.integer < second.value.integer;

  return (result_void_t){.ok = true};
}

const char *GREATER_THAN = ">";
result_void_t greaterThan(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_INTEGER || second.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual =
                         first.type != VALUE_TYPE_INTEGER ? (int)first.type
                                                          : (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.type != VALUE_TYPE_INTEGER
                                     ? first.position
                                     : second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.integer > second.value.integer;

  return (result_void_t){.ok = true};
}

const char *NEQ = "!=";
result_void_t notEqual(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  bool are_equal = false;
  if (first.type == second.type) {
    switch (first.type) {
    case VALUE_TYPE_INTEGER:
      are_equal = first.value.integer == second.value.integer;
      break;
    case VALUE_TYPE_BOOLEAN:
      are_equal = first.value.boolean == second.value.boolean;
      break;
    case VALUE_TYPE_NIL:
      are_equal = true;
      break;
    case VALUE_TYPE_BUILTIN:
    case VALUE_TYPE_CLOSURE:
    case VALUE_TYPE_LIST:
    default:
      are_equal = false;
      break;
    }
  }

  result->type = VALUE_TYPE_BOOLEAN;
  // Logical NOT operation on are_equal - flip the bit
  result->value.boolean = (bool)(!are_equal);

  return (result_void_t){.ok = true};
}

const char *LEQ = "<=";
result_void_t lessEqual(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_INTEGER || second.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual =
                         first.type != VALUE_TYPE_INTEGER ? (int)first.type
                                                          : (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.type != VALUE_TYPE_INTEGER
                                     ? first.position
                                     : second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.integer <= second.value.integer;

  return (result_void_t){.ok = true};
}

const char *GEQ = ">=";
result_void_t greaterEqual(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_INTEGER || second.type != VALUE_TYPE_INTEGER) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual =
                         first.type != VALUE_TYPE_INTEGER ? (int)first.type
                                                          : (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_INTEGER,
                     .position = first.type != VALUE_TYPE_INTEGER
                                     ? first.position
                                     : second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  result->value.boolean = first.value.integer >= second.value.integer;

  return (result_void_t){.ok = true};
}

const char *LOGICAL_AND = "and";
result_void_t logicalAnd(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_BOOLEAN) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)first.type,
                     .payload.unexpected_type.expected = NODE_TYPE_BOOLEAN,
                     .position = first.position};
    return error(result_void_t, error);
  }

  if (second.type != VALUE_TYPE_BOOLEAN) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_BOOLEAN,
                     .position = second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  bool both_true = first.value.boolean;
  if (both_true) {
    both_true = second.value.boolean;
  }
  result->value.boolean = both_true;

  return (result_void_t){.ok = true};
}

const char *LOGICAL_OR = "or";
result_void_t logicalOr(value_t *result, value_list_t *values) {
  if (values->count != 2) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_ARITY,
                     .payload.unexpected_arity.expected = 2,
                     .payload.unexpected_arity.actual = values->count,
                     .position = {0}};
    return error(result_void_t, error);
  }

  value_t first = listGet(value_t, values, 0);
  value_t second = listGet(value_t, values, 1);

  if (first.type != VALUE_TYPE_BOOLEAN) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)first.type,
                     .payload.unexpected_type.expected = NODE_TYPE_BOOLEAN,
                     .position = first.position};
    return error(result_void_t, error);
  }

  if (second.type != VALUE_TYPE_BOOLEAN) {
    error_t error = {.kind = ERROR_KIND_UNEXPECTED_TYPE,
                     .payload.unexpected_type.actual = (int)second.type,
                     .payload.unexpected_type.expected = NODE_TYPE_BOOLEAN,
                     .position = second.position};
    return error(result_void_t, error);
  }

  result->type = VALUE_TYPE_BOOLEAN;
  bool either_true = first.value.boolean;
  if (!either_true) {
    either_true = second.value.boolean;
  }
  result->value.boolean = either_true;

  return (result_void_t){.ok = true};
}
