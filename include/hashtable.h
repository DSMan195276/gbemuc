#ifndef INCLUDE_HASHTABLE_H
#define INCLUDE_HASHTABLE_H

#include "hlist.h"

#define HASH_TABLE_SIZE 4096

struct hashtable {
    hlist_head_t table[HASH_TABLE_SIZE];
};

#endif
