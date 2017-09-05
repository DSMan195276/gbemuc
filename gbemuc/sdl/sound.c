
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "char_buf.h"
#include "gb/sound.h"

#define SDL_SAMPLE_BUF_SIZE (8192 * 8)

struct gb_sound_driver_sdl {
    struct gb_apu_sound sound;

    SDL_AudioSpec spec;
    SDL_AudioDeviceID dev;
};

static void gb_sdl_sound_play(struct gb_apu_sound *sound)
{
    SDL_PauseAudio(false);
}

static void gb_sdl_sound_pause(struct gb_apu_sound *sound)
{
    SDL_PauseAudio(true);
}

static void gb_sdl_sound_play_buf(struct gb_apu_sound *sound, int16_t *buf, size_t bytes)
{
    struct gb_sound_driver_sdl *driver = container_of(sound, struct gb_sound_driver_sdl, sound);

    while (SDL_GetQueuedAudioSize(driver->dev) > bytes)
        SDL_Delay(5);

    char new_buf[bytes];

    memset(new_buf, 0, bytes);

    SDL_MixAudio((uint8_t *)new_buf, (uint8_t *)buf, bytes, 128 / 32);
    SDL_QueueAudio(driver->dev, new_buf, bytes);
    return ;
}

struct gb_apu_sound *gb_sdl_sound_new(void)
{
    int ret;
    struct gb_sound_driver_sdl *driver = malloc(sizeof(*driver));

    memset(driver, 0, sizeof(*driver));

    driver->sound.play = gb_sdl_sound_play;
    driver->sound.pause = gb_sdl_sound_pause;
    driver->sound.play_buf = gb_sdl_sound_play_buf;

    driver->spec.freq = 44100;
    driver->spec.format = AUDIO_S16SYS;
    driver->spec.channels = 2;    /* 1 = mono, 2 = stereo */
    driver->spec.samples = 2048 / 2;
    driver->spec.callback = NULL;
    driver->spec.userdata = driver;

    driver->dev = 1;

    ret = SDL_OpenAudio(&driver->spec, NULL);
    printf("SDL_OpenAudio: %d\n", ret);

    SDL_PauseAudio(false);
    return &driver->sound;
}

void gb_sdl_sound_destroy(struct gb_apu_sound *sound)
{
    struct gb_sound_driver_sdl *driver = container_of(sound, struct gb_sound_driver_sdl, sound);

    SDL_CloseAudio();
    free(driver);
}

