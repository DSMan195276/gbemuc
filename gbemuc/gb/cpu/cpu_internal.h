#ifndef GBEMUC_GB_CPU_INTERNAL_H
#define GBEMUC_GB_CPU_INTERNAL_H

#include "gb.h"

int gb_emu_cpu_run_next_inst(struct gb_emu *emu);
int gb_emu_check_interrupt(struct gb_emu *emu);
int gb_emu_hdma_check(struct gb_emu *emu);
void gb_emu_run_interpreter(struct gb_emu *emu);

#ifdef CONFIG_JIT
# include "cpu_dispatcher.h"
static inline void gb_emu_run_jit(struct gb_emu *emu) {
        struct cpu_dispatcher dispatcher;

        gb_emu_cpu_dispatcher_init(&dispatcher);
        gb_emu_run_dispatcher(&dispatcher, emu);
        gb_emu_cpu_dispatcher_clear(&dispatcher);
}
#else
# include <stdio.h>
static inline void gb_emu_run_jit(struct gb_emu *emu)
{
    fprintf(stderr, "Error: JIT supported not compiled\n");
}
#endif

extern const uint16_t gb_daa_table[];

/* Returns whether or not the provided value maps to (HL) */
#define IS_HL(x) ((x) == b8(110))

extern const uint8_t gb_reg_map_8bit[8];
extern const uint8_t gb_reg_map_16bit_sp[4];
extern const uint8_t gb_reg_map_16bit_af[4];
#endif
