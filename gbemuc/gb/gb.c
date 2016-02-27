
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "gb.h"
#include "gb/gpu.h"
#include "gb/cpu.h"
#include "gb/rom.h"
#include "debug.h"

void gb_emu_rom_open(struct gb_emu *emu, const char *filename)
{
    int cart_bitmap;
    gb_rom_open(&emu->rom, filename);

    cart_bitmap = gb_cart_type_bitmap[emu->rom.cart_type];
    if (cart_bitmap & GB_CART_FLAG(ROM)) {
        emu->mmu.mbc_controller = &gb_mbc0_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc0_eram_mmu_entry;
    } else if (cart_bitmap & GB_CART_FLAG(MBC1)) {
        DEBUG_PRINTF("MBC1 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc1_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc1_eram_mmu_entry;
    }
}

void gb_emu_add_breakpoint(struct gb_emu *emu, uint16_t addr)
{
    emu->breakpoint_count++;

    emu->breakpoints = realloc(emu->breakpoints, emu->breakpoint_count * sizeof(*emu->breakpoints));
    emu->breakpoints[emu->breakpoint_count - 1] = addr;
}

void gb_emu_del_breakpoint(struct gb_emu *emu, int id)
{

}

void gb_emu_init(struct gb_emu *emu, struct gb_gpu_display *display)
{
    memset(emu, 0, sizeof(*emu));
    gb_rom_init(&emu->rom);
    gb_gpu_init(&emu->gpu, display);

    emu->cpu.ime = 0;

    emu->cpu.r.w[GB_REG_PC] = 0x0000;
}

void gb_emu_set_display(struct gb_emu *emu, struct gb_gpu_display *display)
{
    emu->gpu.display = display;
    gb_gpu_display_screen(&emu->gpu);
}

void gb_emu_clear(struct gb_emu *emu)
{
    gb_rom_clear(&emu->rom);
    free(emu->breakpoints);
}

