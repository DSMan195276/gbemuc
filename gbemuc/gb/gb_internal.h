#ifndef GBEMUC_GB_GB_INTERNAL_H
#define GBEMUC_GB_GB_INTERNAL_H

#include "gb.h"

#include <SDL2/SDL.h>

struct gb_emu;

static inline void gb_emu_clock_tick(struct gb_emu *emu)
{
    int cycles = emu->cpu.double_speed? 2: 4;

    gb_emu_gpu_tick(emu, cycles);
    gb_timer_ticks(emu, cycles);

    emu->sound.apu_cycles += cycles;

    if (emu->sound.apu_cycles > 72000) {
        int sample_count;
        if (emu->sound.driver) {
            sample_count = gb_sound_flush(&emu->sound, emu->sound.apu_cycles, emu->sound.apu_sample_buffer, GB_APU_SAMPLES);

            (emu->sound.driver->play_buf) (emu->sound.driver, emu->sound.apu_sample_buffer, sample_count * 4);

            emu->sound.apu_cycles = 0;
        }
    }
}

#endif
