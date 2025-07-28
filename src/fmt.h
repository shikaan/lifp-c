#pragma once

#include "value.h"

void formatError(const error_t *error, const char *input_buffer,
                 const char *file_name, int size,
                 char output_buffer[static size], int *offset);

void formatValue(const value_t *value, int size,
                 char output_buffer[static size], int *offset);
