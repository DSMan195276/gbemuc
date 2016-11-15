#ifndef INCLUDE_GB_TIMER_H
#define INCLUDE_GB_TIMER_H

#include <stdint.h>

#define GB_IO_TIMER_DIV  0xFF04
#define GB_IO_TIMER_TIMA 0xFF05
#define GB_IO_TIMER_TMA  0xFF06
#define GB_IO_TIMER_TAC  0xFF07

struct gb_emu;

struct gb_timer {
    int div_count;
    uint8_t div;

    int tima_count;
    uint8_t tima;
    uint8_t tma;

    int clock_active;
    int clock_select;

    uint8_t tac;
};

void gb_timer_ticks(struct gb_emu *emu, int cycles);
void gb_timer_update_tac(struct gb_emu *emu, uint8_t new_tac);

#endif
