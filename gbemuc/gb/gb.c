
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
    size_t flen = 0;
    gb_rom_open(&emu->rom, filename);

    if (emu->rom.sav_file) {
        fseek(emu->rom.sav_file, 0, SEEK_END);
        flen = ftell(emu->rom.sav_file);
        fseek(emu->rom.sav_file, 0, SEEK_SET);

        printf("Sav file size: %zd\n", flen);
    }

    cart_bitmap = gb_cart_type_bitmap[emu->rom.cart_type];
    if (cart_bitmap & GB_CART_FLAG(ROM)) {
        emu->mmu.mbc_controller = &gb_mbc0_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc0_eram_mmu_entry;
    } else if (cart_bitmap & GB_CART_FLAG(MBC1)) {
        DEBUG_PRINTF("MBC1 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc1_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc1_eram_mmu_entry;

        if (flen != 0)
            fread(emu->mmu.eram, 1, flen, emu->rom.sav_file);

    } else if (cart_bitmap & GB_CART_FLAG(MBC3)) {
        DEBUG_PRINTF("MBC3 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc3_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc3_eram_mmu_entry;

        if (flen != 0) {
            fread(emu->mmu.eram, 1, 4 * 8192, emu->rom.sav_file);
            if (cart_bitmap & GB_CART_FLAG(TIMER) && flen > 4 * 8192) {
                /* Timer information */
                fread(&emu->mmu.mbc3.cur, 1, sizeof(emu->mmu.mbc3.cur), emu->rom.sav_file);
                fread(&emu->mmu.mbc3.latched, 1, sizeof(emu->mmu.mbc3.latched), emu->rom.sav_file);
                fread(&emu->mmu.mbc3.last_time_update, 1, sizeof(emu->mmu.mbc3.last_time_update), emu->rom.sav_file);
            }
        }
    } else if (cart_bitmap & GB_CART_FLAG(MBC5)) {
        DEBUG_PRINTF("MBC5 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc5_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc5_eram_mmu_entry;

        if (flen != 0)
            fread(emu->mmu.eram, 1, flen, emu->rom.sav_file);
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

    gb_sound_init(&emu->sound);    /* Set the audio format */

    emu->cpu.ime = 0;

    emu->cpu.r.w[GB_REG_PC] = 0x0000;
}

void gb_emu_set_display(struct gb_emu *emu, struct gb_gpu_display *display)
{
    emu->gpu.display = display;
    gb_gpu_display_screen(emu, &emu->gpu);
}

void gb_emu_set_sound(struct gb_emu *emu, struct gb_apu_sound *sound)
{
    emu->sound.driver = sound;
}

void gb_emu_clear(struct gb_emu *emu)
{
    if (emu->rom.sav_file) {
        printf("Writing .sav file: %zd bytes\n", gb_ram_size[emu->rom.ram_size] * 1024);

        fseek(emu->rom.sav_file, 0, SEEK_SET);
        fwrite(emu->mmu.eram, 1, gb_ram_size[emu->rom.ram_size] * 1024, emu->rom.sav_file);

        if (gb_cart_type_bitmap[emu->rom.cart_type] & GB_CART_FLAG(TIMER)) {
            int zero = 0;
            fwrite(&emu->mmu.mbc3.cur, 1, sizeof(emu->mmu.mbc3.cur), emu->rom.sav_file);
            fwrite(&emu->mmu.mbc3.latched, 1, sizeof(emu->mmu.mbc3.latched), emu->rom.sav_file);
            fwrite(&emu->mmu.mbc3.last_time_update, 1, sizeof(emu->mmu.mbc3.last_time_update), emu->rom.sav_file);

            /* Some emulators only work with an extra 4-bytes of zeros on the
             * end (presumably for a 64-bit timestamp). We work with either, but
             * always write the zeros for compatible with other emulators. */
            fwrite(&zero, 1, 4, emu->rom.sav_file);
        }
    }

    gb_rom_clear(&emu->rom);
    gb_sound_clear(&emu->sound);

    free(emu->breakpoints);
}

