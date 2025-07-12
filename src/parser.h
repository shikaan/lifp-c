#pragma once

#include "lexer.h"
#include "node.h"
#include "result.h"
#include <stddef.h>
#include <stdint.h>

constexpr char TRUE[] = "true";
constexpr char FALSE[] = "false";
constexpr char NIL[] = "nil";

typedef Result(node_t *) result_node_t;
result_node_t parse(const token_list_t *tokens, size_t *offset, size_t *depth);
