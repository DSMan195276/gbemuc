#ifndef INCLUDE_SDL_DISPLAY_H
#define INCLUDE_SDL_DISPLAY_H

#include "gb/gpu.h"

#include <SDL2/SDL.h>

/* 'rect' will be copied.
 * 'dest' will be saved, and has to exist for the life of the display. */
struct gb_gpu_display *gb_sdl_display_new(SDL_Rect *rect, SDL_Window *win, SDL_Surface *dest);
void gb_sdl_display_destroy(struct gb_gpu_display *);

SDL_Surface *gb_sdl_surface(struct gb_gpu_display *);

#endif
