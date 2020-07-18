
#include "common.h"

#include <stdio.h>
#include <string.h>

#include "arg_parser.h"
#include "backend_driver.h"
#include "debug.h"
#include "gb/rom.h"
#include "gb/debugger.h"
#include "gb.h"

static const char *gbemuc_version = "gbemuc-" Q(GBEMUC_VERSION);

static struct gb_emu emu;

static const char *arg_str = "[Flags] [Game]";

#define XARGS \
    X(gb_only, "gb", 0, 'g', "Emulate Gameboy") \
    X(cgb_only, "cgb", 0, 'c', "Emulate Color Gameboy (default)") \
    X(cgb_accurate_colors, "cgb-accurate-colors", 0, '\0', "Modifies the color palette to make colors accorate to the CGB display (default)") \
    X(cgb_wrong_colors, "cgb-wrong-colors", 0, '\0', "Treats CGB colors as direct RGB colors.") \
    X(cpu, "cpu", 1, '\0', "'jit' or 'interpreter' ('interpreter' default)") \
    X(help, "help", 0, 'h', "Display help") \
    X(version, "version", 0, 'v', "Display version information") \
    X(sav, "sav", 1, 's', "Specify a sav file to load") \
    X(last, NULL, 0, '\0', NULL)

enum arg_index {
    ARG_EXTRA = ARG_PARSER_EXTRA,
    ARG_ERR = ARG_PARSER_ERR,
    ARG_DONE = ARG_PARSER_DONE,
#define X(enu, id, arg, op, help_text) ARG_##enu,
    XARGS
#undef X
};

static const struct arg gbemuc_args[] = {
#define X(enu, id, arg, op, help_text) [ARG_##enu] = { .lng = id, .shrt = op, .help_txt = help_text, .has_arg = arg },
    XARGS
#undef X
};

int main(int argc, char **argv)
{
    struct gb_gpu_display *disp;
    struct gb_apu_sound *sound;
    const char *game = NULL;
    int cpu_type = GB_CPU_INTERPRETER;

    DEBUG_INIT();

    memset(&emu, 0, sizeof(emu));
    gb_emu_init(&emu);
    emu.config.type = GB_EMU_CGB;
    emu.config.cgb_real_colors = 1;

    enum arg_index ret;

    while ((ret = arg_parser(argc, argv, gbemuc_args)) != ARG_DONE) {
        switch (ret) {
        case ARG_help:
            display_help_text(argv[0], arg_str, "", "", gbemuc_args);
            return 0;
        case ARG_version:
            printf("%s\n", gbemuc_version);
            return 0;

        case ARG_cgb_only:
            emu.config.type = GB_EMU_CGB;
            break;

        case ARG_gb_only:
            emu.config.type = GB_EMU_DMG;
            break;

        case ARG_cgb_accurate_colors:
            emu.config.cgb_real_colors = 1;
            break;

        case ARG_cgb_wrong_colors:
            emu.config.cgb_real_colors = 0;
            break;

        case ARG_cpu:
            {
                char *s, *str = strdup(argarg);
                for (s = str; *s; s++)
                    *s = tolower(*s);

                if (strcmp(str, "jit") == 0) {
                    cpu_type = GB_CPU_JIT;
                } else if (strcmp(str, "interpreter") == 0) {
                    cpu_type = GB_CPU_INTERPRETER;
                } else {
                    printf("%s: Invalid CPU type '%s'\n", argv[0], str);
                    return 0;
                }

                free(str);
            }
            break;

        case ARG_sav:
            printf("Using save file: %s\n", argarg);
            emu.rom.sav_filename = argarg;
            break;

        case ARG_EXTRA:
            if (!game)
                game = argarg;
            else {
                printf("%s: Please supply only one game to run.\n", argv[0]);
                return 0;
            }
            break;

        default:
            return 0;
        }
    }

    printf("Loading rom: %s\n", game);
    gb_emu_rom_open(&emu, game);
    gb_rom_dump_header(&emu.rom, stdout);

    gb_backend_driver *driver = gb_backend_driver_new();

    if (!driver) {
        printf("Unable to initialize gbemuc backend driver!\n");
        return 1;
    }

    disp = gb_backend_get_gpu_display(driver);
    sound = gb_backend_get_apu_sound(driver);

    gb_emu_set_display(&emu, disp);

    if (sound)
        gb_emu_set_sound(&emu, sound);

    gb_emu_reset(&emu);

    gb_run(&emu, cpu_type);
    // gb_debugger_run(&emu);

    gb_emu_clear(&emu);
    gb_backend_driver_destroy(driver);

    DEBUG_CLOSE();

    return 0;
}

