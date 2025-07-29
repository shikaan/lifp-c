#include "../src/fmt.h"
#include "../src/arena.h"
#include "../src/list.h"
#include "../src/node.h"
#include "test.h"
#include <assert.h>
#include <stddef.h>

static arena_t *test_arena;

void values() {
  const int size = 128;
  char buffer[size];
  int offset = 0;

  const value_t integer = {.type = VALUE_TYPE_INTEGER, .value.integer = 123};
  formatValue(&integer, size, buffer, &offset);
  expectEqlString(buffer, "123", 3, "formats integers");

  offset = 0;
  const value_t nil = {.type = VALUE_TYPE_NIL};
  formatValue(&nil, size, buffer, &offset);
  expectEqlString(buffer, "nil", 3, "formats nil");

  offset = 0;
  const value_t v_true = {.type = VALUE_TYPE_BOOLEAN, .value.boolean = true};
  formatValue(&v_true, size, buffer, &offset);
  expectEqlString(buffer, "true", 4, "formats true");

  offset = 0;
  const value_t v_false = {.type = VALUE_TYPE_BOOLEAN, .value.boolean = false};
  formatValue(&v_false, size, buffer, &offset);
  expectEqlString(buffer, "false", 5, "formats false");

  offset = 0;
  const value_t builtin = {.type = VALUE_TYPE_BUILTIN};
  formatValue(&builtin, size, buffer, &offset);
  expectEqlString(buffer, "#<builtin>", 11, "formats builtin");

  offset = 0;
  arenaReset(test_arena);
  result_ref_t allocation = valueCreate(test_arena, VALUE_TYPE_LIST);
  assert(allocation.ok);
  value_t *list = allocation.value;

  allocation = listAppend(value_t, &list->value.list, &integer);
  assert(allocation.ok);
  allocation = listAppend(value_t, &list->value.list, &nil);
  assert(allocation.ok);

  formatValue(list, size, buffer, &offset);
  expectEqlString(buffer, "(123 nil)", 10, "formats lists");

  offset = 0;
  arenaReset(test_arena);
  allocation = valueCreate(test_arena, VALUE_TYPE_CLOSURE);
  assert(allocation.ok);
  value_t *closure = allocation.value;

  allocation = nodeCreate(test_arena, NODE_TYPE_SYMBOL);
  assert(allocation.ok);
  node_t *symbol = allocation.value;
  symbol->value.symbol[0] = 'a';
  symbol->value.symbol[1] = 0;

  allocation = listAppend(node_t, &closure->value.closure.arguments, symbol);
  assert(allocation.ok);

  closure->value.closure.form.type = NODE_TYPE_LIST;

  allocation =
      listAppend(node_t, &closure->value.closure.form.value.list, symbol);
  assert(allocation.ok);

  formatValue(list, size, buffer, &offset);
  expectEqlString(buffer, "(fn* (a) (a))", 10, "formats lambdas");
}

void errors() {
  const int size = 128;
  char buffer[size];
  int offset = 0;

  char symbol[10] = "not-found";
  error_t error = {
      .kind = ERROR_KIND_SYMBOL_NOT_FOUND,
      .payload.symbol_not_found = symbol,
      .position.line = 1,
      .position.column = 10,
  };

  const char list_buffer[23] = "(1 2 3 4 not-found 10)";
  formatError(&error, list_buffer, "file.lifp", size, buffer, &offset);
  expectEqlString(buffer,
                  "Error: symbol 'not-found' not found\n"
                  "\n"
                  "  (1 2 3 4 not-found 10)\n"
                  "           ^\n"
                  "  at file.lifp:1:10",
                  95, "puts caret in the right place (list)");

  offset = 0;
  error.position.column = 1;
  const char atom_buffer[10] = "not-found";
  formatError(&error, atom_buffer, "file.lifp", size, buffer, &offset);
  expectEqlString(buffer,
                  "Error: symbol 'not-found' not found\n"
                  "\n"
                  "  not-found\n"
                  "  ^\n"
                  "  at file.lifp:1:1",
                  72, "puts caret in the right place (atom)");
  offset = 0;
  error.position.column = 2;
  const char init_list_buffer[12] = "(not-found)";
  formatError(&error, init_list_buffer, "file.lifp", size, buffer, &offset);
  expectEqlString(buffer,
                  "Error: symbol 'not-found' not found\n"
                  "\n"
                  "  (not-found)\n"
                  "   ^\n"
                  "  at file.lifp:1:2",
                  72, "puts caret in the right place (init list)");
}

int main() {
  result_ref_t allocation = arenaCreate((size_t)(1024 * 1024));
  assert(allocation.ok);
  test_arena = allocation.value;

  suite(values);
  suite(errors);
  return report();
}
