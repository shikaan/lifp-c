#pragma once

#include "list.h"
#include "result.h"
#include "value.h"
#include <stddef.h>
#include <stdio.h>

constexpr size_t OUTPUT_BUFFER_SIZE = 4096;

void formatCurrentLine(const error_t error, const char *input_buffer,
                       size_t size, char output_buffer[static size],
                       int *offset) {
  // TODO: use strtok
  if (input_buffer) {
    size_t current_line = 1;
    const char *line_start = input_buffer;
    const char *line_end = input_buffer;

    // Find the start of the error line
    while (current_line < error.position.line && *line_end != '\0') {
      if (*line_end == '\n') {
        current_line++;
        line_start = line_end + 1;
      }
      line_end++;
    }

    // Find the end of the error line
    while (*line_end != '\0' && *line_end != '\n') {
      line_end++;
    }

    // Print the line
    *offset += snprintf(output_buffer + *offset, size, "\n\n");
    int line_length = (int)(line_end - line_start);
    *offset += snprintf(output_buffer + *offset, size, "  %.*s\n", line_length,
                        line_start);

    // Print the caret pointer
    *offset += snprintf(output_buffer + *offset, size, "  ");
    for (size_t i = 1;
         i < error.position.column && (size - (size_t)*offset) > 1; i++) {
      *offset += snprintf(output_buffer + *offset, size, " ");
    }
    *offset += snprintf(output_buffer + *offset, size, "^\n");
  }
}

void formatError(const error_t error, const char *input_buffer,
                 const char *file_name, size_t size,
                 char output_buffer[static size]) {

  int offset = snprintf(output_buffer, size, "Error: ");

  switch (error.kind) {
  case ERROR_KIND_ALLOCATION:
    snprintf(output_buffer + offset, size, "allocation error.");
    return; // this does not have a position
  case ERROR_KIND_INVALID_EXPRESSION:
    offset += snprintf(output_buffer + offset, size, "invalid expression");
    break;
  case ERROR_KIND_UNEXPECTED_TYPE:
    offset += snprintf(output_buffer + offset, size, "unexpected type");
    break;
  case ERROR_KIND_KEY_TOO_LONG:
    offset += snprintf(output_buffer + offset, size, "key too long");
    break;
  case ERROR_KIND_UNEXPECTED_TOKEN:
    offset += snprintf(output_buffer + offset, size, "unexpected token '%c'",
                       error.payload.unexpected_token);
    break;
  case ERROR_KIND_SYMBOL_NOT_FOUND:
    offset += snprintf(output_buffer + offset, size, "symbol '%s' not found",
                       error.payload.symbol_not_found);
    break;
  case ERROR_KIND_UNBALANCED_PARENTHESES:
    offset += snprintf(output_buffer + offset, size, "unbalanced parentheses");
    break;
  case ERROR_KIND_UNEXPECTED_ARITY:
    offset += snprintf(output_buffer + offset, size,
                       "unexpected arity. Expected %lu arguments, got %lu.",
                       error.payload.unexpected_arity.expected,
                       error.payload.unexpected_arity.actual);
    break;
  case ERROR_KIND_UNEXPECTED_VALUE:
    offset += snprintf(output_buffer + offset, size, "unexpected value");
    break;
  default:
    offset +=
        snprintf(output_buffer + offset, size, "unknown error at %s:%lu:%lu",
                 file_name, error.position.line, error.position.column);
    break;
  }

  formatCurrentLine(error, input_buffer, size, output_buffer, &offset);

  snprintf(output_buffer + offset, size, "  at %s:%lu:%lu", file_name,
           error.position.line, error.position.column);
}

void formatOutput(const value_t *node, int size, char buffer[static size],
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
  case VALUE_TYPE_BUILTIN: {
    *offset +=
        snprintf(buffer + *offset, (size_t)(size - *offset), "#<builtin>");
    return;
  }
  case VALUE_TYPE_LIST: {
    (*offset) += snprintf(buffer + *offset, 2, "(");

    if (node->value.list.count > 0) {
      for (size_t i = 0; i < node->value.list.count - 1; i++) {
        value_t sub_node = listGet(value_t, &node->value.list, i);
        formatOutput(&sub_node, size, buffer, offset);
        *offset += snprintf(buffer + *offset, 2, " ");
      }

      value_t sub_node =
          listGet(value_t, &node->value.list, node->value.list.count - 1);
      formatOutput(&sub_node, size, buffer, offset);
    }
    *offset += snprintf(buffer + *offset, 2, ")");
    return;
  }
  case VALUE_TYPE_CLOSURE:
    // TODO: pretty print closures
    *offset +=
        snprintf(buffer + *offset, (size_t)(size - *offset), "#<closure>");
    return;
  default:
    return;
  }
}