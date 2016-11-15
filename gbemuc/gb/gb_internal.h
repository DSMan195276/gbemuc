#ifndef GBEMUC_GB_GB_INTERNAL_H
#define GBEMUC_GB_GB_INTERNAL_H

#include "gb.h"

struct gb_emu;

static inline void gb_emu_clock_tick(struct gb_emu *emu)
{
    gb_emu_gpu_tick(emu);

    gb_timer_ticks(emu, 4);
}

#endif
