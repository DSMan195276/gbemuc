
#include "common.h"

#include <stdint.h>
#include <SDL2/SDL.h>

#include "gb/gpu.h"

struct gb_display_sdl {
    struct gb_gpu_display gb_disp;
    SDL_Texture *texture;

    SDL_Rect dest_rect;
    SDL_Renderer *dest;
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
}

struct gb_gpu_display *gb_sdl_display_new(SDL_Rect *rect, SDL_Renderer *rend)
{
    struct gb_display_sdl *disp = malloc(sizeof(*disp));

    disp->gb_disp.disp_buf = gb_sdl_display;

    disp->dest_rect = *rect;
    disp->dest = rend;

    disp->texture = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT);

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

