
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "debug.h"
#include "sdl_display.h"
#include "gb/rom.h"
#include "gb/debugger.h"
#include "gb.h"

static struct gb_emu emu;

int main(int argc, char **argv)
{
    struct gb_gpu_display *disp;
    SDL_Rect rect;
    SDL_Window *window;
    SDL_Renderer *renderer;

    DEBUG_INIT();

    gb_emu_init(&emu, NULL);

    if (argc == 2) {
        printf("Loading rom: %s\n", argv[1]);
        gb_emu_rom_open(&emu, argv[1]);

        gb_rom_dump_header(&emu.rom, stdout);
    } else {
        return 0;
    }

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("GBEMUC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    rect.x = 0;
    rect.y = 0;
    rect.w = GB_SCREEN_WIDTH;
    rect.h = GB_SCREEN_HEIGHT;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    disp = gb_sdl_display_new(&rect, renderer);

    DEBUG_OFF();
    gb_emu_set_display(&emu, disp);
    gb_debugger_run(&emu);

    gb_emu_clear(&emu);
    gb_sdl_display_destroy(disp);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    DEBUG_CLOSE();

    return 0;
}

