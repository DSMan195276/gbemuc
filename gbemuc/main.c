
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "debug.h"
#include "sdl_display.h"
#include "sdl_sound.h"
#include "gb/rom.h"
#include "gb/debugger.h"
#include "gb.h"

static struct gb_emu emu;

int main(int argc, char **argv)
{
    struct gb_gpu_display *disp;
    struct gb_apu_sound *sound;
    SDL_Window *window;

    DEBUG_INIT();

    gb_emu_init(&emu);

    emu.config.type = GB_EMU_CGB;
    //emu.config.cgb_real_colors = 1;

    if (argc == 2) {
        printf("Loading rom: %s\n", argv[1]);
        gb_emu_rom_open(&emu, argv[1]);

        gb_rom_dump_header(&emu.rom, stdout);
    } else {
        return 0;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    window = SDL_CreateWindow("GBEMUC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH * 4, GB_SCREEN_HEIGHT * 4, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    disp = gb_sdl_display_new(window);
    sound = gb_sdl_sound_new();

    DEBUG_OFF();
    gb_emu_set_display(&emu, disp);
    gb_emu_set_sound(&emu, sound);

    gb_emu_reset(&emu);

    gb_debugger_run(&emu);

    gb_emu_clear(&emu);
    gb_sdl_display_destroy(disp);
    SDL_DestroyWindow(window);

    SDL_Quit();

    DEBUG_CLOSE();

    return 0;
}

