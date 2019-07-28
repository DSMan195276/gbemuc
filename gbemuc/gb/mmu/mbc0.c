
#include "common.h"

#include <string.h>

#include "gb.h"
#include "gb/mmu.h"
#include "gb/bios.h"

/*
 *
 * MBC0 ROM
 *
 */

static uint8_t mbc0_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!emu->mmu.bios_flag && addr < 0x0100)
        return gb_bios[addr];

    return emu->rom.data[addr];
}

static void mbc0_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    /* NOP */
}

static uint8_t mbc0_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return 0;
}

static void mbc0_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    /* NOP */
}

static int mbc0_get_bank(struct gb_emu *emu, uint16_t addr)
{
    return 0;
}

struct gb_mmu_entry gb_mbc0_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = mbc0_read8,
    .write8 = mbc0_write8,
    .get_bank = mbc0_get_bank,
};

struct gb_mmu_entry gb_mbc0_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = mbc0_eram_read8,
    .write8 = mbc0_eram_write8,
    .get_bank = mbc0_get_bank,
};

