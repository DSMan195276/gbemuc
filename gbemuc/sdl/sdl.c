
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "gb/gpu.h"

static const int gb_ms_per_frame = 17;

struct gb_display_sdl {
    struct gb_gpu_display gb_disp;
    SDL_Texture *texture;

    SDL_Rect dest_rect;
    SDL_Renderer *dest;

    unsigned int last_tick;
};

static void gb_sdl_display(struct gb_gpu_display *disp, union gb_gpu_color_u *buff)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    void *pixels;
    int pitch = GB_SCREEN_WIDTH * sizeof(uint32_t);

    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);

    memcpy(pixels, buff, GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * sizeof(uint32_t));

    SDL_UnlockTexture(sdl->texture);

    SDL_RenderCopy(sdl->dest, sdl->texture, NULL, &sdl->dest_rect);
    SDL_RenderPresent(sdl->dest);

    if (sdl->last_tick == 0) {
        sdl->last_tick = SDL_GetTicks();
    } else {
        unsigned int now = SDL_GetTicks();

        if (now - sdl->last_tick < gb_ms_per_frame) {
            unsigned int diff = gb_ms_per_frame - (now - sdl->last_tick);
            struct timespec sleep_time;

            sleep_time.tv_sec = 0;
            sleep_time.tv_nsec = diff * 1000000;

            nanosleep(&sleep_time, NULL);
        }

        sdl->last_tick = SDL_GetTicks();
    }
}

static void gb_sdl_get_keystate(struct gb_gpu_display *disp, struct gb_keypad *keys)
{
    const uint8_t *keystate;
    SDL_PumpEvents();

    keystate = SDL_GetKeyboardState(NULL);

    keys->key_up     = !keystate[SDL_SCANCODE_UP];
    keys->key_down   = !keystate[SDL_SCANCODE_DOWN];
    keys->key_left   = !keystate[SDL_SCANCODE_LEFT];
    keys->key_right  = !keystate[SDL_SCANCODE_RIGHT];
    keys->key_a      = !keystate[SDL_SCANCODE_A];
    keys->key_b      = !keystate[SDL_SCANCODE_B];
    keys->key_start  = !keystate[SDL_SCANCODE_RETURN];
    keys->key_select = !keystate[SDL_SCANCODE_RCTRL];
}

struct gb_gpu_display *gb_sdl_display_new(SDL_Rect *rect, SDL_Renderer *rend)
{
    struct gb_display_sdl *disp = malloc(sizeof(*disp));

    disp->gb_disp.disp_buf = gb_sdl_display;
    disp->gb_disp.get_keystate = gb_sdl_get_keystate;

    disp->dest_rect = *rect;
    disp->dest = rend;

    disp->texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

    disp->last_tick = 0;

    return &disp->gb_disp;
}

void gb_sdl_display_destroy(struct gb_gpu_display *disp)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    SDL_DestroyTexture(sdl->texture);
    free(disp);
}

SDL_Texture *gb_sdl_texture(struct gb_gpu_display *disp)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    return sdl->texture;
}

