
#include "common.h"

#include <stdint.h>
#include <time.h>

#include "debug.h"
#include "gb_internal.h"
#include "gb.h"
#include "gb/timer.h"

#define GB_TIMER_TAC_CLOCK_ON 0x04
#define GB_TIMER_TAC_CLOCK_SELECT 0x03

/* These numbers are defined by the GameBoy */
//int gb_clock_select_divisor[] = { 256, 4, 16, 64 };
int gb_clock_select_divisor[] = { 256, 4, 16, 64 };

void gb_timer_tima_ticks(struct gb_emu *emu, int cycles)
{
    int ticks = cycles / 4;
    uint16_t tima = emu->timer.tima;

    ticks += emu->timer.tima_count;

    emu->timer.tima_count = ticks % gb_clock_select_divisor[emu->timer.clock_select];
    ticks /= gb_clock_select_divisor[emu->timer.clock_select];

    tima += ticks;

    if (tima > 0xFF) {
        uint16_t overflow = tima - 0xFF;

        emu->timer.tima = emu->timer.tma + overflow;
        emu->cpu.int_flags |= (1 << GB_INT_TIMER);
    } else {
        emu->timer.tima = tima;
    }
}

void gb_timer_div_ticks(struct gb_emu *emu, int cycles)
{
    int ticks = cycles / 4;
    uint16_t div = emu->timer.div;

    ticks += emu->timer.div_count;

    emu->timer.div_count = ticks % 64; /* DIV timer always ticks at 64 */
    ticks /= 64;

    div += ticks;

    if (div > 0xFF)
        div -= 0xFF;

    emu->timer.div = div;
}

void gb_timer_ticks(struct gb_emu *emu, int cycles)
{
    gb_timer_div_ticks(emu, cycles);

    if (emu->timer.clock_active)
        gb_timer_tima_ticks(emu, cycles);
}

void gb_timer_update_tac(struct gb_emu *emu, uint8_t new_tac)
{
    emu->timer.tac = new_tac;
    emu->timer.clock_active = !!(new_tac & GB_TIMER_TAC_CLOCK_ON);
    emu->timer.clock_select = new_tac & GB_TIMER_TAC_CLOCK_SELECT;
}

