
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

static inline uint16_t bank_number(struct gb_emu *emu)
{
    uint16_t bank_no = 0;

    bank_no = emu->mmu.mbc1.rom_bank;
    if (emu->mmu.mbc1.rom_ram_mode == 0)
        bank_no += emu->mmu.mbc1.ram_rom_bank_upper << 5;

    if (bank_no == 0 || bank_no == 0x20 || bank_no == 0x40 || bank_no == 0x60)
        bank_no++;

    DEBUG_PRINTF("BANK NO: 0x%02x\n", bank_no);

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
            DEBUG_PRINTF("MBC1 READ8 BANK: 0x%08x, 0x%04x, 0x%02x\n", data_offset, low, bank_no);
        }

        return emu->rom.data[data_offset];
    }
}

static uint16_t mbc1_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;

    if (!emu->mmu.bios_flag && addr < 0x0100) {
        memcpy(&ret, gb_bios + addr, sizeof(ret));
    } else {
        uint16_t bank_no = 0;
        int data_offset = addr;

        if (addr >= 0x4000) {
            bank_no = bank_number(emu);
            data_offset &= 0x3FFF;
            data_offset += bank_no * 0x4000;
            DEBUG_PRINTF("MBC1 READ8 BANK: 0x%08x, 0x%04x, 0x%02x\n", data_offset, low, bank_no);
        }

        memcpy(&ret, emu->rom.data + data_offset, sizeof(ret));
    }

    return ret;
}

static void mbc1_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    uint16_t a;
    DEBUG_PRINTF("MBC1 WRITE8: 0x%04x, 0x%04x\n", addr, low);

    a = (addr >> 12);

    switch (a) {
    case 0x0:
    case 0x1:
        emu->mmu.mbc1.ram_enable = val;
        break;

    case 0x2:
    case 0x3:
        emu->mmu.mbc1.rom_bank = val & 0x1F;
        DEBUG_PRINTF("MBC1 BANK NUMBER: 0x%02x\n", emu->mmu.mbc1.rom_bank);
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

static void mbc1_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    DEBUG_PRINTF("MBC1 WRITE16: 0x%04x, 0x%04x\n", addr, low);
    /* NOP */
}

static uint8_t mbc1_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->mmu.eram[addr][0];
}

static uint16_t mbc1_eram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    memcpy(&ret, &emu->mmu.eram[addr][0], sizeof(ret));
    return ret;
}

static void mbc1_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    emu->mmu.eram[addr][0] = val;
}

static void mbc1_eram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    memcpy(&emu->mmu.eram[addr][0], &val, sizeof(val));
}

struct gb_mmu_entry gb_mbc1_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = mbc1_read8,
    .read16 = mbc1_read16,
    .write8 = mbc1_write8,
    .write16 = mbc1_write16,
};

struct gb_mmu_entry gb_mbc1_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = mbc1_eram_read8,
    .read16 = mbc1_eram_read16,
    .write8 = mbc1_eram_write8,
    .write16 = mbc1_eram_write16,
};

