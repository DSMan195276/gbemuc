#ifndef INCLUDE_GB_SOUND_H
#define INCLUDE_GB_SOUND_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __EMSCRIPTEN__
# include <SDL.h>
#else
# include <SDL2/SDL.h>
#endif

#define GB_APU_SAMPLES (4096 * 4)
#define GB_APU_SAMPLE_RATE 44100

#define GB_APU_CLOCK_TICKS (167772 / 5)

#ifdef __cplusplus
extern "C" {
#endif

struct gb_apu_sound {
    void (*play) (struct gb_apu_sound *);
    void (*pause) (struct gb_apu_sound *);
    void (*play_buf) (struct gb_apu_sound *, int16_t *samples, size_t bytes);
};

struct gb_sound {
    struct gb_apu *apu;
    struct gb_stereo_buffer *stereo;

    struct gb_apu_sound *driver;

    int16_t apu_sample_buffer[GB_APU_SAMPLES];
    unsigned int apu_cycles;
};

void gb_sound_init(struct gb_sound *);
void gb_sound_clear(struct gb_sound *);

uint32_t gb_sound_read(struct gb_sound *, int ts, uint32_t addr);
void gb_sound_write(struct gb_sound *, int ts, uint32_t addr, uint8_t val);

int32_t gb_sound_flush(struct gb_sound *, int ts, int16_t *SoundBuf, const int32_t MaxSoundFrames);
void gb_sound_start(struct gb_sound *);
void gb_sound_finish(struct gb_sound *);
void gb_sound_reset(struct gb_sound *);

bool gb_sound_set_sound_rate(struct gb_sound *, uint32_t rate);

#ifdef __cplusplus
}
#endif

#endif

