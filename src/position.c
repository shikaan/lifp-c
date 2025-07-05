#include "./position.h"

bool positionEql(position_t first, position_t second) {
  return (bool)((first.column == second.column) && (first.line == second.line));
}
