
#include "common.h"

#include <string.h>
#include <time.h>

#include "gb.h"
#include "gb/mmu.h"
#include "gb/bios.h"
#include "debug.h"

/*
 *
 * MBC3 ROM
 *
 */

static inline int bank_number(struct gb_emu *emu)
{
    int bank_no = 0;

    bank_no = emu->mmu.mbc3.rom_bank;

    if (bank_no == 0)
        bank_no++;

    return bank_no;
}

static uint8_t mbc3_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!emu->mmu.bios_flag && addr < 0x0100) {
        return gb_bios[addr];
    } else {
        uint16_t bank_no = 0;
        int data_offset = addr;

        if (addr >= 0x4000) {
            bank_no = bank_number(emu);
            data_offset &= 0x3FFF;
            data_offset += bank_no * 0x4000;
        }

        return emu->rom.data[data_offset];
    }
}

static void mbc3_update_time(struct gb_mmu *mmu)
{
    time_t now = time(NULL);
    time_t elapsed = now - mmu->mbc3.last_time_update;

    if (elapsed > 0) {
        int seconds, minutes, hours, days;

        seconds = elapsed % 60;
        elapsed /= 60;

        minutes = elapsed % 60;
        elapsed /= 60;

        hours = elapsed % 24;
        elapsed /= 24;

        days = elapsed;

        mmu->mbc3.cur.seconds += seconds;
        if (mmu->mbc3.cur.seconds > 60) {
            mmu->mbc3.cur.seconds -= 60;
            mmu->mbc3.cur.minutes++;
        }

        mmu->mbc3.cur.minutes += minutes;
        if (mmu->mbc3.cur.minutes > 60) {
            mmu->mbc3.cur.minutes -= 60;
            mmu->mbc3.cur.hours++;
        }

        mmu->mbc3.cur.hours += hours;
        if (mmu->mbc3.cur.hours > 24) {
            mmu->mbc3.cur.hours -= 24;
            mmu->mbc3.cur.days++;
        }

        mmu->mbc3.cur.days += days;
        if (mmu->mbc3.cur.days > 255) {
            mmu->mbc3.cur.control = 0;
            if (mmu->mbc3.cur.days > 512) {
                mmu->mbc3.cur.days %= 512;
                mmu->mbc3.cur.control |= 0x80;
            }

            mmu->mbc3.cur.control |= (mmu->mbc3.cur.days > 255)? 0x01: 0x00;
        }

    }

    mmu->mbc3.last_time_update = now;
}

static void mbc3_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    uint16_t a;

    a = (addr >> 12);

    switch (a) {
    case 0x0:
    case 0x1:
        emu->mmu.mbc3.ram_timer_enable = val & 0xF;
        break;

    case 0x2:
    case 0x3:
        emu->mmu.mbc3.rom_bank = val & 0x7F;
        break;

    case 0x4:
    case 0x5:
        emu->mmu.mbc3.ram_bank = val;
        break;

    case 0x6:
    case 0x7:
        if (emu->mmu.mbc3.timer_latch == 0 && val == 1) {
            mbc3_update_time(&emu->mmu);
            emu->mmu.mbc3.latched = emu->mmu.mbc3.cur;
        }

        emu->mmu.mbc3.timer_latch = val;
        break;
    }
}

static uint8_t mbc3_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!(emu->mmu.mbc3.ram_timer_enable == 0x0A))
        return 0;

    if (emu->mmu.mbc3.ram_bank < 0x04)
        return emu->mmu.eram[emu->mmu.mbc3.ram_bank][addr];
    else if (emu->mmu.mbc3.ram_bank == 0x08)
        return emu->mmu.mbc3.latched.seconds;
    else if (emu->mmu.mbc3.ram_bank == 0x09)
        return emu->mmu.mbc3.latched.minutes;
    else if (emu->mmu.mbc3.ram_bank == 0x0A)
        return emu->mmu.mbc3.latched.hours;
    else if (emu->mmu.mbc3.ram_bank == 0x0B)
        return emu->mmu.mbc3.latched.days & 0xFF;
    else if (emu->mmu.mbc3.ram_bank == 0x0C)
        return emu->mmu.mbc3.latched.control;

    return 0;
}

static void mbc3_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    if (!(emu->mmu.mbc3.ram_timer_enable == 0x0A))
        return ;

    if (emu->mmu.mbc3.ram_bank < 0x04)
        emu->mmu.eram[emu->mmu.mbc3.ram_bank][addr] = val;
    else {
        time_t now = time(NULL);

        emu->mmu.mbc3.last_time_update = now;

        if (emu->mmu.mbc3.ram_bank == 0x08)
            emu->mmu.mbc3.cur.seconds = val;
        else if (emu->mmu.mbc3.ram_bank == 0x09)
            emu->mmu.mbc3.cur.minutes = val;
        else if (emu->mmu.mbc3.ram_bank == 0x0A)
            emu->mmu.mbc3.cur.hours = val;
        else if (emu->mmu.mbc3.ram_bank == 0x0B)
            emu->mmu.mbc3.cur.days = val;
        else if (emu->mmu.mbc3.ram_bank == 0x0C)
            emu->mmu.mbc3.cur.control = val;
    }
}

static int mbc3_get_bank(struct gb_emu *emu, uint16_t addr)
{
    if (addr >= 0x4000)
        return bank_number(emu);

    return 0;
}

static int mbc3_eram_get_bank(struct gb_emu *emu, uint16_t addr)
{
    if (emu->mmu.mbc3.ram_bank < 0x04)
        return emu->mmu.mbc3.ram_bank;
    else
        return 0;
}

struct gb_mmu_entry gb_mbc3_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = mbc3_read8,
    .write8 = mbc3_write8,
    .get_bank = mbc3_get_bank,
};

struct gb_mmu_entry gb_mbc3_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = mbc3_eram_read8,
    .write8 = mbc3_eram_write8,
    .get_bank = mbc3_eram_get_bank,
};

