
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL.h>

#include "sdl_driver.h"
#include "sdl_internal.h"

struct gb_sdl_driver *gb_sdl_driver_new(void)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window *window = SDL_CreateWindow("GBEMUC", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GB_SCREEN_WIDTH * 4, GB_SCREEN_HEIGHT * 4, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
        return NULL;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    struct gb_sdl_driver *driver = malloc(sizeof(*driver));
    memset(driver, 0, sizeof(*driver));

    gb_display_sdl_init(&driver->display, window);
    gb_sound_sdl_init(&driver->sound);

    return driver;
}

void gb_sdl_driver_destroy(struct gb_sdl_driver *driver)
{
    gb_sound_sdl_clear(&driver->sound);
    gb_display_sdl_clear(&driver->display);
    SDL_Quit();
    free(driver);
}

struct gb_gpu_display *gb_sdl_driver_get_gb_gpu_display(struct gb_sdl_driver *driver)
{
    return &driver->display.gb_disp;
}

struct gb_apu_sound *gb_sdl_driver_get_gb_apu_sound(struct gb_sdl_driver *driver)
{
    return &driver->sound.sound;
}

