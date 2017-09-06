
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "sdl_driver.h"
#include "sdl_internal.h"

struct gb_sdl_driver *gb_sdl_driver_new(SDL_Window *window)
{
    struct gb_sdl_driver *driver = malloc(sizeof(*driver));
    memset(driver, 0, sizeof(*driver));

    gb_display_sdl_init(&driver->display, window);
    gb_sound_sdl_init(&driver->sound);

    return driver;
}

void gb_sdl_driver_destory(struct gb_sdl_driver *driver)
{
    gb_sound_sdl_clear(&driver->sound);
    gb_display_sdl_clear(&driver->display);
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

