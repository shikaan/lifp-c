#include "value.h"

result_ref_t valueCreate(arena_t *arena, value_type_t type) {
  value_t *value = nullptr;
  try(result_ref_t, arenaAllocate(arena, sizeof(value_t)), value);
  value->type = type;

  if (value->type == VALUE_TYPE_LIST) {
    value_list_t *list = nullptr;
    try(result_ref_t, listCreate(value_t, arena, VALUE_LIST_INITIAL_SIZE),
        list);
    bytewiseCopy(&value->value.list, list, sizeof(value_list_t));
  }

  if (value->type == VALUE_TYPE_CLOSURE) {
    value_list_t *list = nullptr;
    try(result_ref_t, listCreate(node_t, arena, VALUE_LIST_INITIAL_SIZE), list);
    bytewiseCopy(&value->value.closure.arguments, list, sizeof(value_list_t));

    // TODO: this is pessimistic, forms can be also non-lists and therefore
    // require less memory. Can we clean this up?
    node_t *form = nullptr;
    try(result_ref_t, nodeCreate(arena, NODE_TYPE_LIST), form);
    bytewiseCopy(&value->value.closure.form, form, sizeof(node_t));
  }

  return ok(result_ref_t, value);
}

result_value_ref_t valueClone(arena_t *arena, const value_t *source) {
  value_t *destination = nullptr;
  tryWithMeta(result_value_ref_t, valueCreate(arena, source->type),
              source->position, destination);
  destination->position.line = source->position.line;
  destination->position.column = source->position.column;

  switch (source->type) {
  case VALUE_TYPE_BOOLEAN:
    destination->value.boolean = source->value.boolean;
    break;
  case VALUE_TYPE_INTEGER:
    destination->value.integer = source->value.integer;
    break;
  case VALUE_TYPE_BUILTIN:
    destination->value.builtin = source->value.builtin;
    break;
  case VALUE_TYPE_NIL:
    destination->value.nil = source->value.nil;
    break;
  case VALUE_TYPE_CLOSURE:
    nodeCopy(&source->value.closure.form, &destination->value.closure.form);

    tryWithMeta(result_value_ref_t,
                listCopy(value_t, &source->value.closure.arguments,
                         &destination->value.closure.arguments),
                source->position);
    break;
  case VALUE_TYPE_LIST:
    tryWithMeta(
        result_value_ref_t,
        listCopy(value_t, &source->value.list, &destination->value.list),
        source->position);
    break;
  default:
    unreachable();
  }

  return ok(result_value_ref_t, destination);
}