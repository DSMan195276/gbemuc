
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <signal.h>

#include <jit/jit.h>

#include "debug.h"
#include "gb_internal.h"
#include "gb/cpu.h"
#include "cpu_internal.h"
#include "cpu_jit_helpers.h"
#include "cpu_dispatcher.h"
#include "hashtable.h"

struct jit_block {
    hlist_node_t entry;
    uint16_t addr;
    int bank;
    jit_function_t func;
    void (*run_block) (struct gb_emu *);
};

void jit_block_init(struct jit_block *block)
{
    memset(block, 0, sizeof(*block));
    hlist_node_init(&block->entry);
}

static int jit_block_hash(uint16_t addr, int bank)
{
    return (addr ^ (bank << 16)) % HASH_TABLE_SIZE;
}

void gb_emu_cpu_dispatcher_init(struct cpu_dispatcher *dispatcher)
{
    memset(dispatcher, 0, sizeof(*dispatcher));

    object_pool_init(&dispatcher->jit_blocks, sizeof(struct jit_block), 50);
    dispatcher->context = jit_context_create();
}

void gb_emu_cpu_dispatcher_clear(struct cpu_dispatcher *dispatcher)
{
    jit_context_destroy(dispatcher->context);
    object_pool_clear(&dispatcher->jit_blocks);
}

void gb_emu_jit_func_create(struct gb_cpu_jit_context *ctx, struct cpu_dispatcher *dispatcher, struct gb_emu *emu, uint16_t addr)
{
    jit_type_t jit_block_params[] = { jit_type_void_ptr };
    jit_type_t jit_block_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, jit_block_params, ARRAY_SIZE(jit_block_params), 1);

    memset(ctx, 0, sizeof(*ctx));

    ctx->dispatcher = dispatcher;
    ctx->context = dispatcher->context;
    ctx->gb_emu = emu;
    ctx->addr = addr;
    ctx->func_exit_label = jit_label_undefined;

    ctx->func = jit_function_create(dispatcher->context, jit_block_signature);;
    ctx->emu = jit_value_get_param(ctx->func, 0);

    int i;
    for (i = 0; i < ARRAY_SIZE(ctx->regs); i++)
        ctx->regs[i] = jit_insn_load_relative(ctx->func, ctx->emu, GB_REG8_OFFSET(i), jit_type_ubyte);
}

void gb_emu_jit_func_exit(struct gb_cpu_jit_context *ctx)
{
    jit_insn_branch(ctx->func, &ctx->func_exit_label);
}

void (*gb_emu_jit_func_complete(struct gb_cpu_jit_context *ctx)) (struct gb_emu *)
{
    jit_insn_label(ctx->func, &ctx->func_exit_label);

    int i;
    for (i = 0; i < ARRAY_SIZE(ctx->regs); i++)
        jit_insn_store_relative(ctx->func, ctx->emu, GB_REG8_OFFSET(i), jit_insn_convert(ctx->func, ctx->regs[i], jit_type_ubyte, 0));

    jit_insn_default_return(ctx->func);
    jit_function_compile(ctx->func);
    return jit_function_to_closure(ctx->func);
}

void gb_emu_run_dispatcher(struct cpu_dispatcher *dispatcher, struct gb_emu *emu)
{
    while (!emu->stop_emu) {
        if (gb_emu_addr_is_rom(emu, emu->cpu.r.w[GB_REG_PC])) {
            /* Check if it is already compiled */
            uint16_t addr = emu->cpu.r.w[GB_REG_PC];
            int bank = emu->mmu.mbc_controller->get_bank(emu, addr);
            struct jit_block *block, *found = NULL;
            int hash = jit_block_hash(addr, bank);

            hlist_foreach_entry(&dispatcher->htable.table[hash], block, entry) {
                if (block->addr == addr && block->bank == bank) {
                    found = block;
                    break;
                }
            }

            if (!found) {
                printf("Compiling [0x%04x]...\n", emu->cpu.r.w[GB_REG_PC]);
                found = object_pool_get(&dispatcher->jit_blocks);
                jit_block_init(found);

                found->addr = addr;
                found->bank = bank;

                hlist_add(&dispatcher->htable.table[hash], &found->entry);

                struct gb_cpu_jit_context jit_ctx;
                gb_emu_jit_func_create(&jit_ctx, dispatcher, emu, addr);

                while (!gb_emu_cpu_jit_run_next_inst(&jit_ctx))
                    ;

                found->run_block = gb_emu_jit_func_complete(&jit_ctx);
            }

            (found->run_block) (emu);

        } else {
            /* For addresses that aren't read-only ROM, we use the interpreter rather then the JIT */
            gb_emu_cpu_run_next_inst(emu);
        }
    }
}

