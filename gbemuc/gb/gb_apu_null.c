
#include <stdbool.h>
#include "gb.h"
#include "gb/sound.h"

void gb_sound_init(struct gb_sound *sound)
{

}

void gb_sound_clear(struct gb_sound *sound)
{

}

void gb_sound_reset(struct gb_sound *sound)
{
}

uint32_t gb_sound_read(struct gb_sound *sound, int ts, uint32_t addr)
{
    return 0;
}

void gb_sound_write(struct gb_sound *sound, int ts, uint32_t addr, uint8_t val)
{
}

bool gb_sound_set_sound_rate(struct gb_sound *sound, uint32_t rate)
{
	return true;
}

void gb_sound_start(struct gb_sound *sound)
{
}

void gb_sound_finish(struct gb_sound *sound)
{
}

int32_t gb_sound_flush(struct gb_sound *sound, int ts, int16_t *SoundBuf, const int32_t MaxSoundFrames)
{
	return MaxSoundFrames;
}

