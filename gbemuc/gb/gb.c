
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gb.h"
#include "gb/gpu.h"
#include "gb/cpu.h"
#include "gb/rom.h"
#include "gb/io.h"
#include "gb/cgb_themes.h"
#include "debug.h"
#include "cpu/cpu_internal.h"

void gb_emu_rom_open(struct gb_emu *emu, const char *filename)
{
    int cart_bitmap;
    size_t flen = 0;
    FILE *f = NULL;

    gb_rom_open(&emu->rom, filename);

    if (emu->rom.sav_filename) {
        struct stat s;

        int err = stat(emu->rom.sav_filename, &s);
        if (!err) {
            flen = s.st_size;

            if (flen)
                f = fopen(emu->rom.sav_filename, "r");

            printf("Sav file size: %zd\n", flen);
        }
    }

    if (emu->config.type != GB_EMU_CGB || !(emu->rom.cgb_flag & (1 << 7)))
        emu->gb_type = GB_EMU_DMG;
    else
        emu->gb_type = GB_EMU_CGB;

    cart_bitmap = gb_cart_type_bitmap[emu->rom.cart_type];
    if (cart_bitmap & GB_CART_FLAG(ROM)) {
        emu->mmu.mbc_controller = &gb_mbc0_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc0_eram_mmu_entry;
    } else if (cart_bitmap & GB_CART_FLAG(MBC1)) {
        DEBUG_PRINTF("MBC1 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc1_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc1_eram_mmu_entry;

        if (f && flen != 0)
            fread(emu->mmu.eram, 1, flen, f);

    } else if (cart_bitmap & GB_CART_FLAG(MBC3)) {
        DEBUG_PRINTF("MBC3 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc3_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc3_eram_mmu_entry;

        if (f && flen != 0) {
            fread(emu->mmu.eram, 1, 4 * 8192, f);
            if (cart_bitmap & GB_CART_FLAG(TIMER) && flen > 4 * 8192) {
                /* Timer information */
                fread(&emu->mmu.mbc3.cur, 1, sizeof(emu->mmu.mbc3.cur), f);
                fread(&emu->mmu.mbc3.latched, 1, sizeof(emu->mmu.mbc3.latched), f);
                fread(&emu->mmu.mbc3.last_time_update, 1, sizeof(emu->mmu.mbc3.last_time_update), f);
            }
        }
    } else if (cart_bitmap & GB_CART_FLAG(MBC5)) {
        DEBUG_PRINTF("MBC5 CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_mbc5_mmu_entry;
        emu->mmu.eram_controller = &gb_mbc5_eram_mmu_entry;

        if (f && flen != 0) {
            size_t sl = fread(emu->mmu.eram, 1, flen, f);
            printf("Read %zd bytes!\n", sl);
        }
    } else if (emu->rom.cart_type == 0x7F) {
        DEBUG_PRINTF("GB LOADER CONTROLLER\n");
        emu->mmu.mbc_controller = &gb_loader_mmu_entry;
        emu->mmu.eram_controller = &gb_loader_eram_mmu_entry;
    }

    if (f)
        fclose(f);
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

void gb_emu_init(struct gb_emu *emu)
{
    memset(emu, 0, sizeof(*emu));
    gb_rom_init(&emu->rom);
    gb_gpu_init(&emu->gpu);

    gb_sound_init(&emu->sound);    /* Set the audio format */
}

void gb_emu_reset(struct gb_emu *emu)
{
    gb_emu_io_reset(emu);

    emu->cpu.ime = 0;

    emu->cpu.r.w[GB_REG_PC] = 0x0100;
    emu->cpu.r.w[GB_REG_SP] = 0xFFFE;
    emu->mmu.bios_flag = 1;

    if (gb_emu_is_cgb(emu))
        emu->cpu.r.b[GB_REG_A] = 0x11;
    else
        emu->cpu.r.b[GB_REG_A] = 0x01;
}

void gb_emu_set_display(struct gb_emu *emu, struct gb_gpu_display *display)
{
    union gb_gpu_color_u default_theme[4] = {
        { .i_color = 0xFFFFFFFF },
        { .i_color = 0xFFAAAAAA },
        { .i_color = 0xFF555555 },
        { .i_color = 0xFF000000 },
    };

    emu->gpu.display = display;
    if (!display)
        return ;

    if (cgb_find_theme(&display->dmg_theme, emu->rom.title, emu->rom.title_chksum)) {
        printf("Using theme: 0x%02x\n", emu->rom.title_chksum);
    } else {
        printf("Using default\n");

        memcpy(display->dmg_theme.bg, default_theme, sizeof(display->dmg_theme.bg));
        memcpy(display->dmg_theme.sprites[0], default_theme, sizeof(display->dmg_theme.sprites[0]));
        memcpy(display->dmg_theme.sprites[1], default_theme, sizeof(display->dmg_theme.sprites[1]));
    }

    gb_gpu_display_screen(emu, &emu->gpu);
}

void gb_emu_set_sound(struct gb_emu *emu, struct gb_apu_sound *sound)
{
    emu->sound.driver = sound;
}

void gb_emu_write_save(struct gb_emu *emu)
{
    FILE *f = fopen(emu->rom.sav_filename, "w+");

    printf("Writing .sav file: %zd bytes\n", gb_ram_size[emu->rom.ram_size] * 1024);
    printf("sav_file: %s\n", emu->rom.sav_filename);

    fseek(f, 0, SEEK_SET);
    fwrite(emu->mmu.eram, 1, gb_ram_size[emu->rom.ram_size] * 1024, f);

    if (gb_cart_type_bitmap[emu->rom.cart_type] & GB_CART_FLAG(TIMER)) {
        int zero = 0;
        fwrite(&emu->mmu.mbc3.cur, 1, sizeof(emu->mmu.mbc3.cur), f);
        fwrite(&emu->mmu.mbc3.latched, 1, sizeof(emu->mmu.mbc3.latched), f);
        fwrite(&emu->mmu.mbc3.last_time_update, 1, sizeof(emu->mmu.mbc3.last_time_update), f);

        /* Some emulators only work with an extra 4-bytes of zeros on the
         * end (presumably for a 64-bit timestamp). We work with either, but
         * always write the zeros for compatible with other emulators. */
        fwrite(&zero, 1, 4, f);
    }

    fclose(f);
}

void gb_emu_clear(struct gb_emu *emu)
{
    if (emu->rom.sav_filename)
        gb_emu_write_save(emu);

    gb_rom_clear(&emu->rom);
    gb_sound_clear(&emu->sound);

    free(emu->breakpoints);
}

static struct gb_emu *signal_emu;

static void debugger_run_sigint(int signum)
{
    signal_emu->stop_emu = 1;
    signal_emu->reason = GB_EMU_STOP;
}

enum gb_emu_stop gb_run(struct gb_emu *emu, enum gb_cpu_type cpu_type)
{
    struct sigaction new_act, old_act;

    emu->stop_emu = 0;

    if (emu->sound.driver) {
        gb_sound_start(&emu->sound);
        gb_sound_set_sound_rate(&emu->sound, GB_APU_SAMPLE_RATE);

        (emu->sound.driver->play) (emu->sound.driver);
    }

    memset(&new_act, 0, sizeof(new_act));
    new_act.sa_handler = debugger_run_sigint;

    sigaction(SIGINT, &new_act, &old_act);

    signal_emu = emu;

    switch (cpu_type) {
    case GB_CPU_INTERPRETER:
        gb_emu_run_interpreter(emu);
        break;

    case GB_CPU_JIT:
        gb_emu_run_jit(emu);
        break;
    }

    sigaction(SIGINT, &old_act, NULL);

    if (emu->sound.driver) {
        (emu->sound.driver->pause) (emu->sound.driver);

        gb_sound_finish(&emu->sound);
    }

    return emu->reason;
}

