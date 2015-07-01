
#include "common.h"

#include <stdint.h>
#include <SDL2/SDL.h>

#include "gb/gpu.h"

struct gb_display_sdl {
    struct gb_gpu_display gb_disp;
    SDL_Surface *surface;

    SDL_Rect dest_rect;
    SDL_Surface *dest;
    SDL_Window *win;
};

static void gb_sdl_display(struct gb_gpu_display *disp, union gb_gpu_color_u *buff)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    memcpy(sdl->surface->pixels, buff, GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH * sizeof(uint32_t));

    SDL_BlitSurface(sdl->surface, NULL, sdl->dest, &sdl->dest_rect);
    SDL_UpdateWindowSurface(sdl->win);
}

struct gb_gpu_display *gb_sdl_display_new(SDL_Rect *rect, SDL_Window *win, SDL_Surface *dest)
{
    struct gb_display_sdl *disp = malloc(sizeof(*disp));
    disp->surface = SDL_CreateRGBSurface(0, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, 32, 0, 0, 0, 0);

    disp->gb_disp.disp_buf = gb_sdl_display;

    disp->dest_rect = *rect;
    disp->dest = dest;
    disp->win = win;

    return &disp->gb_disp;
}

void gb_sdl_display_destroy(struct gb_gpu_display *disp)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    SDL_FreeSurface(sdl->surface);
    free(disp);
}

SDL_Surface *gb_sdl_surface(struct gb_gpu_display *disp)
{
    struct gb_display_sdl *sdl = container_of(disp, struct gb_display_sdl, gb_disp);
    return sdl->surface;
}

