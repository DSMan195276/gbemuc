#ifndef INCLUDE_SDL_DRIVER_H
#define INCLUDE_SDL_DRIVER_H

#include "gb/sound.h"
#include "gb/gpu.h"

#include <SDL2/SDL.h>

struct gb_sdl_driver;

struct gb_sdl_driver *gb_sdl_driver_new(SDL_Window *);
void gb_sdl_driver_destory(struct gb_sdl_driver *);

/*
 * These structures are associated with the gb_sdl_driver and will be destroyed
 * when gb_sdl_driver_destroy is called on the gbsdl_driver that these came
 * from.
 */
struct gb_gpu_display *gb_sdl_driver_get_gb_gpu_display(struct gb_sdl_driver *);
struct gb_apu_sound   *gb_sdl_driver_get_gb_apu_sound(struct gb_sdl_driver *);

#endif
