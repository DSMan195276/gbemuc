#ifndef GBEMUC_GB_CPU_DISPATCHER_H
#define GBEMUC_GB_CPU_DISPATCHER_H

#include "gb.h"
#include <jit/jit.h>

#include "hashtable.h"
#include "object_pool.h"

struct cpu_dispatcher {
    jit_context_t context;
    struct hashtable htable;
    struct object_pool jit_blocks;
};

struct gb_cpu_jit_context {
    struct cpu_dispatcher *dispatcher;
    jit_context_t context;
    jit_function_t func;
    jit_value_t emu;

    struct gb_emu *gb_emu;
    uint16_t addr;
};

typedef void gb_cpu_jit_func_t(struct gb_emu *);

void gb_emu_cpu_dispatcher_init(struct cpu_dispatcher *);
void gb_emu_cpu_dispatcher_clear(struct cpu_dispatcher *);

void gb_emu_jit_func_create(struct gb_cpu_jit_context *ctx, struct cpu_dispatcher *dispatcher, struct gb_emu *emu, uint16_t addr);

gb_cpu_jit_func_t *gb_emu_jit_func_complete(struct gb_cpu_jit_context *ctx);

void gb_emu_run_dispatcher(struct cpu_dispatcher *, struct gb_emu *);

#endif
