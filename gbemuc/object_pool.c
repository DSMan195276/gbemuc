
#include "common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object_pool.h"

/* If you're using C11, then we use alignof with max_align_t from <stdalign.h>
 * to determine the alignment for our objects */

#if (__STDC_VERSION >= 20112L)
# include <stdalign.h>
# define MAX_ALIGNMENT alignof(max_align_t)
#else
# define MAX_ALIGNMENT 16
#endif

struct empty_object {
    struct empty_object *next;
};

struct object_block {
    struct object_block *next;
    size_t free_objects;
    struct empty_object *empty_head; /* Linked-list of empty objects */
    char *memory;
};

static size_t align_to_power_of_two(size_t size, size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

void object_pool_init(struct object_pool *pool, size_t object_size, size_t pool_size)
{
    memset(pool, 0, sizeof(*pool));

    pool->object_size = align_to_power_of_two(object_size, MAX_ALIGNMENT);
    pool->pool_size = pool_size;
}

void *object_pool_get(struct object_pool *pool)
{
    struct object_block *cur_pool = pool->head;

    while (cur_pool && cur_pool->free_objects == 0)
        cur_pool = cur_pool->next;

    if (!cur_pool) {
        struct object_block *new_block;
        new_block = malloc(sizeof(*new_block));
        memset(new_block, 0, sizeof(*new_block));

        new_block->next = pool->head;
        pool->head = new_block;

        new_block->free_objects = pool->pool_size;
        new_block->memory = malloc(pool->object_size * pool->pool_size);

        struct empty_object **prev_object = &new_block->empty_head;
        char *mem = new_block->memory;
        size_t i;

        for (i = 0; i < pool->pool_size; i++) {
            struct empty_object *cur = (struct empty_object *)mem;
            mem += pool->object_size;

            (*prev_object) = cur;
            prev_object = &cur->next;
        }

        *prev_object = NULL;
        cur_pool = new_block;
    }

    struct empty_object *obj = cur_pool->empty_head;
    cur_pool->empty_head = obj->next;
    cur_pool->free_objects--;

    return obj;
}

void object_pool_clear(struct object_pool *pool)
{
    struct object_block *block, *next_block;

    for (block = pool->head; block; block = next_block) {
        next_block = block->next;

        free(block->memory);
        free(block);
    }
}

