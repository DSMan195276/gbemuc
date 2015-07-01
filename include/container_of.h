#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include <stddef.h>

/* Inspired via the Linux-kernel macro 'container_of' */
#define container_of(ptr, type, member) \
    ((type *) ((char*)(ptr) - offsetof(type, member)))

#endif
