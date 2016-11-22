
#include "common.h"

#include <string.h>

#include "gb.h"
#include "gb/mmu.h"
#include "gb/bios.h"
#include "debug.h"

/*
 *
 * MBC5 ROM
 *
 */

static uint8_t mbc5_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!emu->mmu.bios_flag && addr < 0x0100) {
        return gb_bios[addr];
    } else {
        uint16_t bank_no = 0;
        int data_offset = addr;

        if (addr >= 0x4000) {
            bank_no = emu->mmu.mbc5.rom_bank;
            data_offset &= 0x3FFF;
            data_offset += bank_no * 0x4000;
        }

        return emu->rom.data[data_offset];
    }
}

static void mbc5_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    uint16_t a;

    a = (addr >> 12);

    switch (a) {
    case 0x0:
    case 0x1:
        emu->mmu.mbc5.ram_bank_enable = val & 0xF;
        break;

    case 0x2:
        emu->mmu.mbc5.rom_bank = (emu->mmu.mbc5.rom_bank & 0x100) | val;
        break;

    case 0x3:
        emu->mmu.mbc5.rom_bank = (emu->mmu.mbc5.rom_bank & 0x0FF) | ((val & 0x01) << 8);
        break;

    case 0x4:
    case 0x5:
        emu->mmu.mbc5.ram_bank = val & 0x0F;
        break;
    }
}

static uint8_t mbc5_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!(emu->mmu.mbc5.ram_bank_enable == 0x0A))
        return 0;

    return emu->mmu.eram[emu->mmu.mbc5.ram_bank][addr];
}

static void mbc5_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    if (emu->mmu.mbc5.ram_bank_enable == 0x0A)
        emu->mmu.eram[emu->mmu.mbc5.ram_bank][addr] = val;
}

struct gb_mmu_entry gb_mbc5_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = mbc5_read8,
    .write8 = mbc5_write8,
};

struct gb_mmu_entry gb_mbc5_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = mbc5_eram_read8,
    .write8 = mbc5_eram_write8,
};

