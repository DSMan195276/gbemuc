
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "char_buf.h"
#include "gb/sound.h"

#define SDL_SAMPLE_BUF_SIZE (8192 * 2)

struct gb_sound_driver_sdl {
    struct gb_apu_sound sound;

    SDL_AudioSpec spec;
    SDL_AudioDeviceID dev;

    struct char_buf sample_buffer;
    char sample_data[SDL_SAMPLE_BUF_SIZE];
};

static void sdl_callback(void *udata, Uint8 *samples, int bytes)
{
    struct gb_sound_driver_sdl *driver = udata;

    char_buf_read(&driver->sample_buffer, samples, bytes);
    return ;
}

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

    SDL_LockAudioDevice(driver->dev);

    char_buf_write(&driver->sample_buffer, buf, bytes);

    SDL_UnlockAudioDevice(driver->dev);

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
    driver->spec.samples = 4096 / 2;
    driver->spec.callback = sdl_callback;
    driver->spec.userdata = driver;

    driver->dev = 1;

    ret = SDL_OpenAudio(&driver->spec, NULL);
    printf("SDL_OpenAudio: %d\n", ret);

    driver->sample_buffer.buffer = driver->sample_data;
    driver->sample_buffer.len = sizeof(driver->sample_data);

    return &driver->sound;
}

void gb_sdl_sound_destroy(struct gb_apu_sound *sound)
{
    struct gb_sound_driver_sdl *driver = container_of(sound, struct gb_sound_driver_sdl, sound);

    SDL_CloseAudio();
    free(driver);
}

