
#include "common.h"

#include <stdio.h>
#include <stdint.h>

#include "gb.h"
#include "gb/gpu.h"
#include "gb/cpu.h"
#include "gb/rom.h"

void gb_emu_clock_tick(struct gb_emu *emu)
{
    gb_emu_gpu_tick(emu);
}

void gb_emu_rom_open(struct gb_emu *emu, const char *filename)
{
    gb_rom_open(&emu->rom, filename);
}

void gb_emu_init(struct gb_emu *emu, struct gb_gpu_display *display)
{
    memset(emu, 0, sizeof(*emu));
    gb_rom_init(&emu->rom);
    gb_gpu_init(&emu->gpu, display);

    emu->cpu.r.w[GB_REG_PC] = 0x0100;
    emu->cpu.r.w[GB_REG_SP] = 0xFFFE;
}

void gb_emu_set_display(struct gb_emu *emu, struct gb_gpu_display *display)
{
    emu->gpu.display = display;
    gb_gpu_display_screen(&emu->gpu);
}

void gb_emu_clear(struct gb_emu *emu)
{
    gb_rom_clear(&emu->rom);
}

