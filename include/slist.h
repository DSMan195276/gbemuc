#ifndef INCLUDE_SLIST_H
#define INCLUDE_SLIST_H

#include <stdlib.h>

#include "container_of.h"

/* Like the normal 'list', but singly-linked.
 *
 * This list is NULL terminated. */

struct slist_node {
    struct slist_node *next;
};

struct slist_head {
    struct slist_node n;
};

#define SLIST_HEAD_INIT() { { NULL } }

#define SLIST_HEAD(name) \
    struct slist_head name = SLIST_HEAD_INIT()

static inline void INIT_SLIST_HEAD(struct slist_head *list)
{
    list->n.next = NULL;
}

static inline void __slist_add(struct slist_node *new,
                               struct slist_node *prev)
{
    new->next = prev->next;
    prev->next = new;
}

static inline void slist_add_after(struct slist_node *node, struct slist_node *new)
{
    __slist_add(new, node);
}

static inline void slist_add(struct slist_head *head, struct slist_node *new)
{
    slist_add_after(&head->n, new);
}

static inline void __slist_del(struct slist_node *prev, struct slist_node *next)
{
    prev->next = next->next;
}

static inline void slist_del(struct slist_node *prev, struct slist_node *entry)
{
    __slist_del(prev, entry);
}

static inline int slist_empty(const struct slist_head *head)
{
    return head->n.next == NULL;
}

static inline struct slist_node *__slist_first(struct slist_head *head)
{
    return head->n.next;
}

static inline struct slist_node *__slist_take_first(struct slist_head *head)
{
    struct slist_node *node = __slist_first(head);
    slist_del(&head->n, node);
    return node;
}

#define slist_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define slist_first_entry(head, type, member) \
    slist_entry(__slist_first(head), type, member)

#define slist_take_first_entry(head, type, member) \
    slist_entry(__slist_take_first(head), type, member)

#define slist_next_entry(pos, member) \
    slist_entry((pos)->member.next, __typeof__(*(pos)), member)

#define slist_foreach(head, pos) \
    for (pos = (head)->n.next; pos != NULL; pos = pos->next)

#define slist_foreach_with_prev(head, pos, prev) \
    for (prev = NULL, pos = (head)->n.next; pos != NULL; prev = pos, pos = pos->next)

#define slist_foreach_entry(head, pos, member) \
    for (pos = slist_first_entry(head, __typeof__(*pos), member); \
         &pos->member != NULL; \
         pos = slist_next_entry(pos, member))

#endif
