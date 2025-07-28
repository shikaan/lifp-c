#include "list.h"
#include "node.h"
#include "result.h"
#include "value.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define append(Size, Buffer, Offset, ...)                                      \
  *Offset += snprintf(Buffer + *Offset, (size_t)(Size - *Offset), __VA_ARGS__);

static void formatCurrentLine(const error_t *error, const char *input_buffer,
                              int size, char output_buffer[static size],
                              int *offset) {
  const char *sepatator = "\n";
  char *brkt = nullptr;
  char *line = nullptr;
  char *copy = strdup(input_buffer);
  size_t current_line = 1;

  for (line = strtok_r(copy, sepatator, &brkt); line;
       line = strtok_r(nullptr, sepatator, &brkt)) {
    if (error->position.line == current_line)
      break;
  }

  const char *indent = "  ";
  append(size, output_buffer, offset, "\n\n%s%s", indent, line);
  append(size, output_buffer, offset, "\n%s%*c^\n", indent,
         (int)error->position.column - 1, ' ');
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

void formatError(const error_t *error, const char *input_buffer,
                 const char *file_name, int size,
                 char output_buffer[static size], int *offset) {

  append(size, output_buffer, offset, "Error: ");

  switch (error->kind) {
  case ERROR_KIND_ALLOCATION:
    append(size, output_buffer, offset, "allocation error");
    return; // this does not have a position
  case ERROR_KIND_INVALID_EXPRESSION:
    append(size, output_buffer, offset, "invalid expression");
    break;
  case ERROR_KIND_UNEXPECTED_TYPE:
    append(size, output_buffer, offset, "invalid type");
    break;
  case ERROR_KIND_KEY_TOO_LONG:
    append(size, output_buffer, offset, "invalid key size");
    break;
  case ERROR_KIND_UNEXPECTED_TOKEN:
    append(size, output_buffer, offset, "unexpected token '%c'",
           error->payload.unexpected_token);
    break;
  case ERROR_KIND_SYMBOL_NOT_FOUND:
    append(size, output_buffer, offset, "symbol '%s' not found",
           error->payload.symbol_not_found);
    break;
  case ERROR_KIND_UNBALANCED_PARENTHESES:
    append(size, output_buffer, offset, "unbalanced parentheses");
    break;
  case ERROR_KIND_UNEXPECTED_ARITY:
    append(size, output_buffer, offset,
           "unexpected arity. Expected %lu arguments, got %lu.",
           error->payload.unexpected_arity.expected,
           error->payload.unexpected_arity.actual);
    break;
  case ERROR_KIND_UNEXPECTED_VALUE:
    append(size, output_buffer, offset, "unexpected value");
    break;
  default:
    append(size, output_buffer, offset, "unknown error at %s:%lu:%lu",
           file_name, error->position.line, error->position.column);
    break;
  }

  formatCurrentLine(error, input_buffer, size, output_buffer, offset);
  append(size, output_buffer, offset, "  at %s:%lu:%lu", file_name,
         error->position.line, error->position.column);
}

void formatValue(const value_t *node, int size, char buffer[static size],
                 int *offset) {
  switch (node->type) {
  case VALUE_TYPE_BOOLEAN: {
    append(size, buffer, offset, "%s", node->value.boolean ? "true" : "false");
    return;
  }
  case VALUE_TYPE_NIL: {
    append(size, buffer, offset, "nil");
    return;
  }
  case VALUE_TYPE_INTEGER: {
    append(size, buffer, offset, "%d", node->value.integer);
    return;
  }
  case VALUE_TYPE_BUILTIN: {
    append(size, buffer, offset, "#<builtin>");
    return;
  }
  case VALUE_TYPE_LIST: {
    append(size, buffer, offset, "(");
    value_list_t list = node->value.list;

    if (list.count > 0) {
      for (size_t i = 0; i < list.count - 1; i++) {
        value_t sub_value = listGet(value_t, &list, i);
        formatValue(&sub_value, size, buffer, offset);
        append(size, buffer, offset, " ");
      }

      value_t sub_value = listGet(value_t, &list, list.count - 1);
      formatValue(&sub_value, size, buffer, offset);
    }
    append(size, buffer, offset, ")");
    return;
  }
  case VALUE_TYPE_CLOSURE:
    append(size, buffer, offset, "(fn* (");
    node_list_t arguments = node->value.closure.arguments;

    if (arguments.count > 0) {
      for (size_t i = 0; i < arguments.count - 1; i++) {
        node_t sub_node = listGet(node_t, &arguments, i);
        formatNode(&sub_node, size, buffer, offset);
        append(size, buffer, offset, " ");
      }

      node_t sub_node = listGet(node_t, &arguments, arguments.count - 1);
      formatNode(&sub_node, size, buffer, offset);
    }
    append(size, buffer, offset, ") ");

    formatNode(&node->value.closure.form, size, buffer, offset);
    append(size, buffer, offset, ")");
    return;
  default:
    return;
  }
}

#undef append
