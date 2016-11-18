#ifndef INCLUDE_SDL_SOUND_H
#define INCLUDE_SDL_SOUND_H

#include "gb/sound.h"

struct gb_apu_sound *gb_sdl_sound_new(void);
void gb_sdl_sound_destroy(struct gb_apu_sound *);

#endif
