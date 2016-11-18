/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Gb_Apu.h"
#include "Stereo_Buffer.h"

#include "gb.h"
#include "gb/sound.h"

static Gb_Apu gb_apu;
static Stereo_Buffer *gb_buf = NULL;

extern "C" void gb_sound_init(struct gb_sound *sound)
{

}

extern "C" void gb_sound_clear(struct gb_sound *sound)
{

}

extern "C" void gb_sound_reset(struct gb_sound *sound)
{
	Gb_Apu::mode_t gbmode = Gb_Apu::mode_dmg;

#if 0
	if(gbEmulatorType == 4)
	 gbmode = Gb_Apu::mode_agb;
	else if(gbEmulatorType == 3)
	 gbmode = Gb_Apu::mode_dmg;
	else if(gbEmulatorType == 1)
	 gbmode = Gb_Apu::mode_cgb;
	else if(gbEmulatorType == 0)
	{
	 if(gbCgbMode)
	  gbmode = Gb_Apu::mode_cgb;
	 else
	  gbmode = Gb_Apu::mode_dmg;
	}
#endif

	//printf("%d -- %d\n", (int)gbmode, (int)Gb_Apu::mode_cgb);
	gb_apu.reset(gbmode);
}

extern "C" uint32_t gb_sound_read(struct gb_sound *sound, int ts, uint32_t addr)
{
	return gb_apu.read_register(ts * GB_APU_OVERCLOCK, addr);
}

extern "C" void gb_sound_write(struct gb_sound *sound, int ts, uint32_t addr, uint8_t val)
{
	gb_apu.write_register(ts * GB_APU_OVERCLOCK, addr, val);
}

static bool RedoBuffer(struct gb_sound *sound, uint32_t rate)
{
	if (gb_buf) {
        delete gb_buf;
        gb_buf = NULL;
	}

	gb_apu.set_output(NULL, NULL, NULL);

	if (rate) {
        gb_buf = new Stereo_Buffer();

        gb_buf->set_sample_rate(rate, 40);
        gb_buf->clock_rate((long)(4194304 * GB_APU_OVERCLOCK * 1));

        gb_apu.set_output(gb_buf->center(), gb_buf->left(), gb_buf->right());
    }

	return true;
}

extern "C" bool gb_sound_set_sound_rate(struct gb_sound *sound, uint32_t rate)
{
	RedoBuffer(sound, rate);

	return true;
}

extern "C" void gb_sound_start(struct gb_sound *sound)
{
    gb_apu.volume(0.5);

	RedoBuffer(sound, 0);
}

extern "C" void gb_sound_finish(struct gb_sound *sound)
{
    if (gb_buf != NULL) {
        delete gb_buf;
        gb_buf = NULL;
    }
}

extern "C" int32_t gb_sound_flush(struct gb_sound *sound, int ts, int16_t *SoundBuf, const int32_t MaxSoundFrames)
{
	int32_t SoundFrames = 0;

	gb_apu.end_frame(ts * GB_APU_OVERCLOCK);

	if (SoundBuf && gb_buf) {
	    gb_buf->end_frame(ts * GB_APU_OVERCLOCK);
	    SoundFrames = gb_buf->read_samples(SoundBuf, MaxSoundFrames * 2) / 2;
	} else if (gb_buf) {
	    exit(1);
    }

	return(SoundFrames);
}

