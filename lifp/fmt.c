#include "fmt.h"
#include "../lib/list.h"
#include "node.h"
#include "position.h"
#include "value.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define append(Size, Buffer, Offset, ...)                                      \
  *Offset += snprintf(Buffer + *Offset, (size_t)(Size - *Offset), __VA_ARGS__);

static void formatCurrentLine(position_t caret, const char *input_buffer,
                              int size, char output_buffer[static size],
                              int *offset) {
  const char *sepatator = "\n";
  char *brkt = nullptr;
  char *line = nullptr;
  char *copy = strdup(input_buffer);
  size_t current_line = 1;

  for (line = strtok_r(copy, sepatator, &brkt); line;
       line = strtok_r(nullptr, sepatator, &brkt)) {
    if (caret.line == current_line)
      break;
  }

  const char *indent = "  ";
  append(size, output_buffer, offset, "\n\n%s%s", indent, line);
  append(size, output_buffer, offset, "\n%*c^\n",
         (int)caret.column - 1 + (int)strlen(indent), ' ');
}

static void formatNode(const node_t *node, int size, char buffer[static size],
                       int *offset) {
  switch (node->type) {
  case NODE_TYPE_BOOLEAN: {
    append(size, buffer, offset, "%s", node->value.boolean ? "true" : "false");
    return;
  }
  case NODE_TYPE_NIL: {
    append(size, buffer, offset, "nil");
    return;
  }
  case NODE_TYPE_INTEGER: {
    append(size, buffer, offset, "%d", node->value.integer);
    return;
  }
  case NODE_TYPE_SYMBOL: {
    append(size, buffer, offset, "%s", node->value.symbol);
    return;
  }
  case NODE_TYPE_LIST: {
    append(size, buffer, offset, "(");
    node_list_t list = node->value.list;

    if (list.count > 0) {
      for (size_t i = 0; i < list.count - 1; i++) {
        node_t sub_node = listGet(node_t, &list, i);
        formatNode(&sub_node, size, buffer, offset);
        append(size, buffer, offset, " ");
      }

      node_t sub_node = listGet(node_t, &list, list.count - 1);
      formatNode(&sub_node, size, buffer, offset);
    }
    append(size, buffer, offset, ")");
    return;
  }
  default:
    return;
  }
}

void formatErrorMessage(message_t message, position_t position,
                        const char *file_name, const char *input_buffer,
                        int size, char output_buffer[static size],
                        int *offset) {
  append(size, output_buffer, offset, "Error: %s", message);

  formatCurrentLine(position, input_buffer, size, output_buffer, offset);
  append(size, output_buffer, offset, "  at %s:%lu:%lu", file_name,
         position.line, position.column);
}

void formatValue(const value_t *value, int size,
                 char output_buffer[static size], int *offset) {
  switch (value->type) {
  case VALUE_TYPE_BOOLEAN: {
    append(size, output_buffer, offset, "%s",
           value->value.boolean ? "true" : "false");
    return;
  }
  case VALUE_TYPE_NIL: {
    append(size, output_buffer, offset, "nil");
    return;
  }
  case VALUE_TYPE_INTEGER: {
    append(size, output_buffer, offset, "%d", value->value.integer);
    return;
  }
  case VALUE_TYPE_BUILTIN: {
    append(size, output_buffer, offset, "#<builtin>");
    return;
  }
  case VALUE_TYPE_LIST: {
    append(size, output_buffer, offset, "(");
    value_list_t list = value->value.list;

    if (list.count > 0) {
      for (size_t i = 0; i < list.count - 1; i++) {
        value_t sub_value = listGet(value_t, &list, i);
        formatValue(&sub_value, size, output_buffer, offset);
        append(size, output_buffer, offset, " ");
      }

      value_t sub_value = listGet(value_t, &list, list.count - 1);
      formatValue(&sub_value, size, output_buffer, offset);
    }
    append(size, output_buffer, offset, ")");
    return;
  }
  case VALUE_TYPE_CLOSURE:
    append(size, output_buffer, offset, "(fn (");
    node_list_t arguments = value->value.closure.arguments;

    if (arguments.count > 0) {
      for (size_t i = 0; i < arguments.count - 1; i++) {
        node_t sub_node = listGet(node_t, &arguments, i);
        formatNode(&sub_node, size, output_buffer, offset);
        append(size, output_buffer, offset, " ");
      }

      node_t sub_node = listGet(node_t, &arguments, arguments.count - 1);
      formatNode(&sub_node, size, output_buffer, offset);
    }
    append(size, output_buffer, offset, ") ");

    formatNode(&value->value.closure.form, size, output_buffer, offset);
    append(size, output_buffer, offset, ")");
    return;
  default:
    return;
  }
}

#undef append
