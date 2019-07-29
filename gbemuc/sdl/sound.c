
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <SDL.h>

#include "char_buf.h"
#include "gb/sound.h"
#include "sdl_internal.h"

#define SDL_SAMPLE_BUF_SIZE (8192 * 8)

static void gb_sdl_sound_play(struct gb_apu_sound *sound)
{
    SDL_PauseAudio(false);
}

static void gb_sdl_sound_pause(struct gb_apu_sound *sound)
{
    SDL_PauseAudio(true);
}

#ifdef INTERPRETER_ASYNC
static void gb_sdl_sound_play_buf_async(struct gb_apu_sound *sound, int16_t *buf, size_t bytes)
{
    struct gb_sound_sdl *driver = container_of(sound, struct gb_sound_sdl, sound);

    unsigned int queued = SDL_GetQueuedAudioSize(driver->dev);

    if (driver->draining_audio) {
        if (queued > bytes)
            return;

        driver->draining_audio = 0;
        printf("Audio drained to %u\n", queued);
    }

    if (queued > driver->max_drift) {
        printf("Dropping samples, queued: %u, max: %u, bytes: %zd\n", queued, driver->max_drift, bytes);
        printf("Draining audio queue...\n");
        driver->draining_audio = 1;
        return;
    }

    char new_buf[bytes];

    memset(new_buf, 0, bytes);

    SDL_MixAudio((uint8_t *)new_buf, (uint8_t *)buf, bytes, driver->cur_volume);
    SDL_QueueAudio(driver->dev, new_buf, bytes);
    return ;
}
#else
static void gb_sdl_sound_play_buf(struct gb_apu_sound *sound, int16_t *buf, size_t bytes)
{
    struct gb_sound_sdl *driver = container_of(sound, struct gb_sound_sdl, sound);

    while (SDL_GetQueuedAudioSize(driver->dev) > bytes * 2)
        SDL_Delay(5);

    char new_buf[bytes];

    memset(new_buf, 0, bytes);

    SDL_MixAudio((uint8_t *)new_buf, (uint8_t *)buf, bytes, driver->cur_volume);
    SDL_QueueAudio(driver->dev, new_buf, bytes);
    return ;
}
#endif

void gb_sound_sdl_init(struct gb_sound_sdl *driver)
{
    int ret;

    memset(driver, 0, sizeof(*driver));

    driver->cur_volume = 64;

    driver->sound.play = gb_sdl_sound_play;
    driver->sound.pause = gb_sdl_sound_pause;

#ifdef INTERPRETER_ASYNC
    driver->sound.play_buf = gb_sdl_sound_play_buf_async;
#else
    driver->sound.play_buf = gb_sdl_sound_play_buf;
#endif

    driver->spec.freq = 44100;
    driver->spec.format = AUDIO_S16SYS;
    driver->spec.channels = 2;    /* 1 = mono, 2 = stereo */
    driver->spec.samples = 2048 / 2;
    driver->spec.callback = NULL;
    driver->spec.userdata = driver;

    // This is in bytes, making this half a second
    driver->max_drift = driver->spec.freq;

    driver->dev = 1;

    ret = SDL_OpenAudio(&driver->spec, NULL);
    printf("SDL_OpenAudio: %d\n", ret);
    SDL_PauseAudio(false);
}

void gb_sound_sdl_clear(struct gb_sound_sdl *driver)
{
    SDL_CloseAudio();
}

