#ifndef GBEMUC_SDL_SDL_INTERNAL_H
#define GBEMUC_SDL_SDL_INTERNAL_H

#include <SDL2/SDL.h>

#include "gb/gpu.h"
#include "gb/sound.h"

#include "sdl_driver.h"

struct gb_display_sdl {
    struct gb_gpu_display gb_disp;
    SDL_Texture *texture;

    SDL_Window *win;
    SDL_Renderer *dest_rend;

    unsigned int last_tick;
    unsigned int show_fps;
    int fps_count;
    unsigned int fps_start;

    unsigned int p_pressed;
    unsigned int f_pressed;

    unsigned int cur_palette;
};

struct gb_sound_sdl {
    struct gb_apu_sound sound;

    SDL_AudioSpec spec;
    SDL_AudioDeviceID dev;
};

struct gb_sdl_driver {
    struct gb_display_sdl display;
    struct gb_sound_sdl sound;
};

void gb_sound_sdl_init(struct gb_sound_sdl *);
void gb_sound_sdl_clear(struct gb_sound_sdl *);

void gb_display_sdl_init(struct gb_display_sdl *, SDL_Window *);
void gb_display_sdl_clear(struct gb_display_sdl *);

#endif
