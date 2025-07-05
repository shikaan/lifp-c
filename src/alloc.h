#ifndef ALLOC_H_H
#define ALLOC_H_H

#define freeNull(ptr) \
    { \
        free(ptr); \
        ptr = nullptr; \
     }

#endif //ALLOC_H_H
