
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "gb/gpu.h"

/* GB runs at 59.7 FPS. This number is in microseconds to give us a bit better
 * precision for our sleep */
static const int gb_us_per_frame = 17000;

struct gb_display_sdl {
    struct gb_gpu_display gb_disp;
    SDL_Texture *texture;

    SDL_Window *win;
    SDL_Renderer *dest_rend;

    unsigned int last_tick;

    unsigned int p_pressed;
};

static void gb_sdl_display(struct gb_gpu_display *disp, union gb_gpu_color_u *buff)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    void *pixels;
    int pitch = GB_SCREEN_WIDTH * sizeof(uint32_t);
    int height, width;

    SDL_GetWindowSize(sdl->win, &width, &height);

    SDL_LockTexture(sdl->texture, NULL, &pixels, &pitch);

    memcpy(pixels, buff, GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * sizeof(uint32_t));

    SDL_UnlockTexture(sdl->texture);

    SDL_RenderCopy(sdl->dest_rend, sdl->texture, NULL, &(struct SDL_Rect) { .w = width, .h = height});
    SDL_RenderPresent(sdl->dest_rend);

    if (sdl->last_tick == 0) {
        sdl->last_tick = SDL_GetTicks();
    } else {
        unsigned int now = SDL_GetTicks();

        if ((now - sdl->last_tick) * 1000 < gb_us_per_frame) {
            unsigned int diff = gb_us_per_frame - ((now - sdl->last_tick) * 1000);
            struct timespec sleep_time;

            sleep_time.tv_sec = 0;
            sleep_time.tv_nsec = diff * 1000;

            nanosleep(&sleep_time, NULL);
        }

        sdl->last_tick = SDL_GetTicks();
    }
}

static void gb_sdl_get_keystate(struct gb_gpu_display *disp, struct gb_keypad *keys)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
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

    if (keystate[SDL_SCANCODE_P] && !sdl->p_pressed) {
        disp->palette_selection = (disp->palette_selection + 1) % disp->max_palette;
        sdl->p_pressed = 1;
    } else if (!keystate[SDL_SCANCODE_P]) {
        sdl->p_pressed = 0;
    }

    return ;
}

struct gb_gpu_display *gb_sdl_display_new(SDL_Window *window)
{
    struct gb_display_sdl *disp = malloc(sizeof(*disp));

    memset(disp, 0, sizeof(*disp));

    disp->gb_disp.disp_buf = gb_sdl_display;
    disp->gb_disp.get_keystate = gb_sdl_get_keystate;

    disp->win = window;
    disp->dest_rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    disp->texture = SDL_CreateTexture(disp->dest_rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

    disp->last_tick = 0;

    return &disp->gb_disp;
}

void gb_sdl_display_destroy(struct gb_gpu_display *disp)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    SDL_DestroyTexture(sdl->texture);
    SDL_DestroyRenderer(sdl->dest_rend);
    free(disp);
}

SDL_Texture *gb_sdl_texture(struct gb_gpu_display *disp)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    return sdl->texture;
}

