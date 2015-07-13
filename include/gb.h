#ifndef INCLUDE_GB_H
#define INCLUDE_GB_H

#include <stdint.h>

#include "gb/cpu.h"
#include "gb/mmu.h"
#include "gb/gpu.h"
#include "gb/rom.h"

#define GB_HZ 4194304
/* #define GB_HZ 256 */

struct gb_emu {
    struct gb_cpu cpu;
    struct gb_mmu mmu;
    struct gb_gpu gpu;

    struct gb_rom rom;

    char wram[8 * 1024]; /* Working RAM space */
    char zram[128]; /* Zero-page RAM */
};

/* Calling this causes a clock-tick to occur */
void gb_emu_clock_tick(struct gb_emu *);

int gb_emu_cpu_run_next_inst(struct gb_emu *);
int gb_emu_cpu_run_inst(struct gb_emu *emu, uint8_t opcode);

void gb_emu_rom_open(struct gb_emu *emu, const char *filename);
void gb_emu_set_display(struct gb_emu *emu, struct gb_gpu_display *display);
void gb_emu_dump_regs(struct gb_emu *emu, char *output_buf);

void gb_emu_init(struct gb_emu *emu, struct gb_gpu_display *display);
void gb_emu_clear(struct gb_emu *emu);

void gb_run(struct gb_emu *emu);

#endif
