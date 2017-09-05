#ifndef INCLUDE_OBJECT_POOL_H
#define INCLUDE_OBJECT_POOL_H

#include <stdlib.h>

struct object_block;

struct object_pool {
    size_t object_size;
    size_t pool_size;
    struct object_block *head;
};

void object_pool_init(struct object_pool *, size_t object_size, size_t pool_size);
void object_pool_clear(struct object_pool *);

void *object_pool_get(struct object_pool *);

#endif
