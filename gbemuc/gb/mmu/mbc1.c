
#include "common.h"

#include <string.h>

#include "gb.h"
#include "gb/mmu.h"
#include "gb/bios.h"
#include "debug.h"

/*
 *
 * MBC1 ROM
 *
 */

static inline int bank_number(struct gb_emu *emu)
{
    int bank_no = 0;

    bank_no = emu->mmu.mbc1.rom_bank;
    if (emu->mmu.mbc1.rom_ram_mode == 0)
        bank_no |= emu->mmu.mbc1.ram_rom_bank_upper << 5;

    if (bank_no == 0 || bank_no == 0x20 || bank_no == 0x40 || bank_no == 0x60)
        bank_no++;

    return bank_no;
}

static uint8_t mbc1_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
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

static void mbc1_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    uint16_t a;

    a = (addr >> 12);

    switch (a) {
    case 0x0:
    case 0x1:
        emu->mmu.mbc1.ram_enable = val;
        break;

    case 0x2:
    case 0x3:
        emu->mmu.mbc1.rom_bank = val & 0x1F;
        break;

    case 0x4:
    case 0x5:
        emu->mmu.mbc1.ram_rom_bank_upper = val & 0x03;
        break;

    case 0x6:
    case 0x7:
        emu->mmu.mbc1.rom_ram_mode = val;
        break;
    }
}

static uint8_t mbc1_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->mmu.mbc1.rom_ram_mode)
        return emu->mmu.eram[emu->mmu.mbc1.ram_rom_bank_upper][addr];
    else
        return emu->mmu.eram[0][addr];
}

static void mbc1_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    if (emu->mmu.mbc1.rom_ram_mode)
        emu->mmu.eram[emu->mmu.mbc1.ram_rom_bank_upper][addr] = val;
    else
        emu->mmu.eram[0][addr] = val;
}

static int mbc1_get_bank(struct gb_emu *emu, uint16_t addr)
{
    if (addr >= 0x4000)
        return bank_number(emu);

    return 0;
}

static int mbc1_eram_get_bank(struct gb_emu *emu, uint16_t addr)
{
    if (emu->mmu.mbc1.rom_ram_mode)
        return emu->mmu.mbc1.ram_rom_bank_upper;
    else
        return 0;
}

struct gb_mmu_entry gb_mbc1_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = mbc1_read8,
    .write8 = mbc1_write8,
    .get_bank = mbc1_get_bank,
};

struct gb_mmu_entry gb_mbc1_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = mbc1_eram_read8,
    .write8 = mbc1_eram_write8,
    .get_bank = mbc1_eram_get_bank,
};

