#pragma once

#include "parser.h"
#include <stddef.h>
#include <stdio.h>

constexpr size_t OUTPUT_BUFFER_SIZE = 4096;

void print(const node_t *node, int size, char buffer[static size],
           int *offset) {
  switch (node->type) {
  case NODE_TYPE_BOOLEAN: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "%s",
                        node->value.boolean ? "true" : "false");
    return;
  }
  case NODE_TYPE_NIL: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "nil");
    return;
  }
  case NODE_TYPE_INTEGER: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "%d",
                        node->value.integer);
    return;
  }
  case NODE_TYPE_SYMBOL: {
    *offset += snprintf(buffer + *offset, (size_t)(size - *offset), "%s",
                        node->value.symbol);
    return;
  }
  case NODE_TYPE_LIST: {
    (*offset) += snprintf(buffer + *offset, 2, "(");
    for (size_t i = 0; i < node->value.list.count - 1; i++) {
      print(&node->value.list.offset[i], size, buffer, offset);
      *offset += snprintf(buffer + *offset, 2, " ");
    }

    print(&node->value.list.offset[node->value.list.count - 1], size, buffer,
          offset);
    *offset += snprintf(buffer + *offset, 2, ")");
    return;
  }
  default:
    return;
  }
}