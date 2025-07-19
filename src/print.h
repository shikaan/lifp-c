#pragma once

#include "list.h"
#include "value.h"
#include <stddef.h>
#include <stdio.h>

constexpr size_t OUTPUT_BUFFER_SIZE = 4096;

void print(const value_t *node, int size, char buffer[static size],
           int *offset) {
  switch (node->type) {
  case VALUE_TYPE_BOOLEAN: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "%s",
                        node->value.boolean ? "true" : "false");
    return;
  }
  case VALUE_TYPE_NIL: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "nil");
    return;
  }
  case VALUE_TYPE_INTEGER: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "%d",
                        node->value.integer);
    return;
  }
  case VALUE_TYPE_FUNCTION: {
    *offset +=
        snprintf(buffer + *offset, (size_t)(size - *offset), "#<function>");
    return;
  }
  case VALUE_TYPE_LIST: {
    (*offset) += snprintf(buffer + *offset, 2, "(");
    for (size_t i = 0; i < node->value.list.count - 1; i++) {
      value_t sub_node = listGet(value_t, &node->value.list, i);
      print(&sub_node, size, buffer, offset);
      *offset += snprintf(buffer + *offset, 2, " ");
    }

    value_t sub_node =
        listGet(value_t, &node->value.list, node->value.list.count - 1);
    print(&sub_node, size, buffer, offset);
    *offset += snprintf(buffer + *offset, 2, ")");
    return;
  }
  default:
    return;
  }
}