
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL.h>

#include "gb/gpu.h"
#include "sdl_driver.h"
#include "sdl_internal.h"

/* GB runs at 59.7 FPS. This number is in microseconds to give us a bit better
 * precision for our sleep */
static const unsigned int gb_us_per_frame = 16750;

struct gb_dmg_theme gb_palettes[] = {
    GB_DEFINE_THEME(0xFFFFF77B, 0xFFB5AE4A, 0xFF6B6931, 0xFF212010,
                    0xFFFFF77B, 0xFFB5AE4A, 0xFF6B6931, 0xFF212010,
                    0xFFFFF77B, 0xFFB5AE4A, 0xFF6B6931, 0xFF212010),
    GB_DEFINE_THEME(0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
                    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000,
                    0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000),
    GB_DEFINE_THEME(0xFF9BBC0F, 0xFF8BAC0F, 0xFF305230, 0xFF0F380F,
                    0xFF9BBC0F, 0xFF8BAC0F, 0xFF305230, 0xFF0F380F,
                    0xFF9BBC0F, 0xFF8BAC0F, 0xFF305230, 0xFF0F380F),
};

#define GB_PALETTES (sizeof(gb_palettes)/sizeof(*gb_palettes))

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

            //nanosleep(&sleep_time, NULL);
        }

        if (sdl->show_fps) {
            sdl->fps_count++;

            //if (sdl->fps_count == 60) {
            if ((now - sdl->fps_start) / 1000 >= 1) {
                float fps = (float)(now - sdl->fps_start) / 1000;
                fps = sdl->fps_count / fps;

                printf("FPS: %f\n", fps);

                sdl->fps_count = 0;
                sdl->fps_start = now;
            }
        }

        sdl->last_tick = now;
    }
}

static void gb_sdl_get_keystate(struct gb_gpu_display *disp, struct gb_keypad *keys)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    struct gb_sdl_driver *driver = container_of(sdl, struct gb_sdl_driver, display);
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
        sdl->cur_palette = (sdl->cur_palette + 1) % GB_PALETTES;
        disp->dmg_theme = gb_palettes[sdl->cur_palette];

        sdl->p_pressed = 1;
    } else if (!keystate[SDL_SCANCODE_P]) {
        sdl->p_pressed = 0;
    }

    if (keystate[SDL_SCANCODE_F] && !sdl->f_pressed) {
        sdl->show_fps = !sdl->show_fps;
        sdl->f_pressed = 1;
    } else if (!keystate[SDL_SCANCODE_F]) {
        sdl->f_pressed = 0;
    }

    if (keystate[SDL_GetScancodeFromKey(SDLK_PLUS)] && !sdl->plus_pressed) {
        sdl->plus_pressed = 1;
    } else if (!keystate[SDL_GetScancodeFromKey(SDLK_PLUS)] && sdl->plus_pressed) {
        driver->sound.cur_volume++;
        if (driver->sound.cur_volume > 128)
            driver->sound.cur_volume = 128;
        sdl->plus_pressed = 0;
    }

    if (keystate[SDL_SCANCODE_MINUS] && !sdl->minus_pressed) {
        sdl->minus_pressed = 1;
    } else if (!keystate[SDL_SCANCODE_MINUS] && sdl->minus_pressed) {
        driver->sound.cur_volume--;
        if (driver->sound.cur_volume < 0)
            driver->sound.cur_volume = 0;
        sdl->minus_pressed = 0;
    }


    return ;
}

void gb_display_sdl_init(struct gb_display_sdl *disp, SDL_Window *window)
{
    memset(disp, 0, sizeof(*disp));

    disp->gb_disp.disp_buf = gb_sdl_display;
    disp->gb_disp.get_keystate = gb_sdl_get_keystate;

    disp->win = window;
    disp->dest_rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);

    disp->texture = SDL_CreateTexture(disp->dest_rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

    disp->last_tick = 0;
    disp->show_fps = 0;
}

void gb_display_sdl_clear(struct gb_display_sdl *disp)
{
    SDL_DestroyTexture(disp->texture);
    SDL_DestroyRenderer(disp->dest_rend);
}

