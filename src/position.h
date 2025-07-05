#ifndef POSITION_H
#define POSITION_H

#include <stddef.h>

typedef struct {
    size_t line;
    size_t column;
} position_t;

bool positionEql(position_t first, position_t second);

#endif //POSITION_H
